#include <atomic>
#include <chrono>
#include <cstdint>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <mutex>
#include <sstream>
#include <string_view>
#include <unistd.h>

#include "external/chess.hpp"
#include "external/parallel_hashmap/phmap.h"
#include "external/threadpool.hpp"

#include "cdbdirect.h"

using namespace chess;

using PackedBoard = std::array<std::uint8_t, 24>;

namespace std {
template <> struct hash<PackedBoard> {
  size_t operator()(const PackedBoard pbfen) const {
    std::string_view sv(reinterpret_cast<const char *>(pbfen.data()),
                        pbfen.size());
    return std::hash<std::string_view>{}(sv);
  }
};
} // namespace std

using fen_map_t = phmap::parallel_flat_hash_map<
    PackedBoard, std::int16_t, std::hash<PackedBoard>,
    std::equal_to<PackedBoard>,
    std::allocator<std::pair<PackedBoard, std::int16_t>>, 8, std::mutex>;

using fens_progressIndex_t = std::array<fen_map_t *, 3007>;

using fen_set_t =
    phmap::parallel_flat_hash_set<PackedBoard, std::hash<PackedBoard>,
                                  std::equal_to<PackedBoard>,
                                  std::allocator<PackedBoard>, 8, std::mutex>;

using fens_depthIndex_t = std::vector<fen_set_t *>;

using poslist_t = std::map<PackedBoard, int>;

// get memory in MB
std::pair<size_t, size_t> get_memory() {
  size_t tSize = 0, resident = 0, share = 0;
  std::ifstream buffer("/proc/self/statm");
  buffer >> tSize >> resident;
  buffer.close();

  long page_size = sysconf(_SC_PAGE_SIZE);
  return std::make_pair(tSize * page_size / (1024 * 1024),
                        resident * page_size / (1024 * 1024));
};

// locate command line arguments
inline bool find_argument(const std::vector<std::string> &args,
                          std::vector<std::string>::const_iterator &pos,
                          std::string_view arg,
                          bool without_parameter = false) {
  pos = std::find(args.begin(), args.end(), arg);

  return pos != args.end() &&
         (without_parameter || std::next(pos) != args.end());
}

// local data and time
std::string getCurrentDateTime() {
  // Get current time
  std::time_t now = std::time(nullptr);

  // Convert it to local time structure
  std::tm *localTime = std::localtime(&now);

  // Create a stringstream to format the date and time
  std::ostringstream dateTimeStream;
  dateTimeStream << (1900 + localTime->tm_year) << "-"
                 << (localTime->tm_mon + 1) << "-" << localTime->tm_mday << " "
                 << localTime->tm_hour << ":" << localTime->tm_min << ":"
                 << localTime->tm_sec;

  // Convert to string and return
  return dateTimeStream.str();
};

struct Stats {
  std::atomic<size_t> gets;
  std::atomic<size_t> hits;
  std::atomic<size_t> nodes;

  void clear() { gets = hits = nodes = 0; };
};

// returns an index that signifies progress during a chess game,
// this index will never increase during a game.
// it ranges from 3006 to 0.
// index / 97 gives the number of pieces - 2 on the board.
// index % 97 gives pawnProgress, a measure of how far the promotion square
// pawns are. both parts individually also decrease at all times.
size_t progressIndex(const Board &board) {

  // Sum distances to promotion rank, this decreases on any pawn move and range
  // from 0 .. 96
  Bitboard pawns;
  size_t pawnProgress = 0;
  pawns = board.pieces(PieceType::PAWN, Color::WHITE);
  while (pawns) {
    Square ps(pawns.pop());
    int r = int(ps.rank());
    pawnProgress += 7 - r;
  };
  pawns = board.pieces(PieceType::PAWN, Color::BLACK);
  while (pawns) {
    Square ps(pawns.pop());
    int r = int(ps.rank());
    pawnProgress += r;
  };

  size_t nPieces = board.occ().count();

  return (nPieces - 2) * 97 + pawnProgress;
}

// progress a list of fens to the next depth
void explore(const fen_set_t::EmbeddedSet &fen_list, int depth,
             const std::uintptr_t handle, Stats &stats, fen_set_t &visited_keys,
             fens_depthIndex_t &fens_depthIndex,
             fens_progressIndex_t &fens_progressIndex, const int maxCPLoss) {

  for (const auto &key : fen_list) {

    stats.nodes++;

    Board board = Board::Compact::decode(key);

    // probe DB
    std::vector<std::pair<std::string, int>> result =
        cdbdirect_get(handle, board.getFen(false));
    stats.gets++;
    size_t n_elements = result.size();
    int ply = result[n_elements - 1].second;

    if (ply == -2)
      continue;

    stats.hits++;

    bool is_new_entry = visited_keys.lazy_emplace_l(
        std::move(key), [](fen_set_t::value_type &p) {},
        [&key](const fen_set_t::constructor &ctor) { ctor(std::move(key)); });

    if (!is_new_entry)
      continue;

    if (depth == 0)
      continue;

    // No moves to explore (can this happen?)
    if (n_elements <= 1)
      continue;

    // Now explore the remaining moves
    int bestScore = result[0].second;
    size_t pI_1 = progressIndex(board);
    for (auto &pair : result)
      if (pair.first != "a0a0") {

        if (bestScore - pair.second > maxCPLoss)
          break;

        Move m = uci::uciToMove(board, pair.first);
        board.makeMove<true>(m);

        PackedBoard pbfen = Board::Compact::encode(board);
        size_t pI_2 = progressIndex(board);

        if (pI_1 == pI_2) {
          fens_depthIndex[depth - 1]->lazy_emplace_l(
              std::move(pbfen), [](fen_set_t::value_type &p) {},
              [&pbfen](const fen_set_t::constructor &ctor) {
                ctor(std::move(pbfen));
              });
        } else {
          fens_progressIndex[pI_2]->lazy_emplace_l(
              std::move(pbfen),
              [&pbfen, &depth](fen_map_t::value_type &p) {
                p.second = std::max(p.second, std::int16_t(depth - 1));
              },
              [&pbfen, &depth](const fen_map_t::constructor &ctor) {
                ctor(std::move(pbfen), depth - 1);
              });
        }

        board.unmakeMove(m);
      }
  }
}

int main(int argc, char const *argv[]) {

  const std::vector<std::string> args(argv + 1, argv + argc);
  std::vector<std::string>::const_iterator pos;

  // 1. g4
  std::string fen =
      "rnbqkbnr/pppppppp/8/8/6P1/8/PPPPPP1P/RNBQKBNR b KQkq - 0 1";
  int depth = 8;
  int maxCPLoss = std::numeric_limits<int>::max();

  if (find_argument(args, pos, "--depth"))
    depth = std::stoi(*std::next(pos));

  if (find_argument(args, pos, "--maxCPLoss"))
    maxCPLoss = std::stoi(*std::next(pos));

  if (find_argument(args, pos, "--fen"))
    fen = {*std::next(pos)};

  std::cout << "Exploring fen: " << fen << std::endl;
  std::cout << "Max depth: " << depth << std::endl;
  std::cout << "Max cp loss: " << maxCPLoss << std::endl;
  std::cout << "Patience... " << std::endl;

  std::cout << "Opening DB" << std::endl;
  std::uintptr_t handle = cdbdirect_initialize("/mnt/ssd/chess-20240814/data");

  Board board(fen);

  if (cdbdirect_get(handle, board.getFen(false)).back().second == -2) {
    std::cout << "Initial fen not in DB!" << std::endl;
    std::cout << "Closing DB" << std::endl;
    handle = cdbdirect_finalize(handle);
    return 1;
  }

  // counters
  Stats stats;

  fen_set_t visited_keys;

  fens_progressIndex_t fens_progressIndex;
  for (auto &fp : fens_progressIndex)
    fp = new fen_map_t;

  size_t pI_orig = progressIndex(board);
  auto key = Board::Compact::encode(board);
  (*fens_progressIndex[pI_orig])[key] = depth;

  // Start exploring.
  std::cout << "Exploring tree" << std::endl;
  std::cout << "    starting on: " << getCurrentDateTime() << std::endl;
  auto [mem_virt, mem_res] = get_memory();
  std::cout << "    starting memory virt : " << std::setw(18) << mem_virt
            << " res :" << std::setw(18) << mem_res << std::endl;

  size_t total_assigned = 0;
  size_t total_gets = 0;
  size_t total_hits = 0;
  size_t total_nodes = 0;
  std::vector<size_t> total_counts(depth + 1, 0);

  auto total_t_start = std::chrono::high_resolution_clock::now();

  size_t iter = 0;

  for (int pieceProgress = 30; pieceProgress >= 0; pieceProgress--) {
    for (int pawnProgress = 96; pawnProgress >= 0; pawnProgress--) {
      size_t pI_now = pieceProgress * 97 + pawnProgress;
      auto &fens_ongoing = *fens_progressIndex[pI_now];

      if (fens_ongoing.size() > 0) {

        auto t_start = std::chrono::high_resolution_clock::now();

        stats.clear();

        std::vector<size_t> iter_counts(depth + 1, 0);

        iter++;

        size_t total_pending = 0;
        for (int pI_scan = pI_now; pI_scan >= 0; pI_scan--)
          total_pending += (*fens_progressIndex[pI_scan]).size();

        size_t pieces_count = pieceProgress + 2;

        std::cout << std::endl;
        std::cout << "Iteration : " << std::setw(4) << iter << std::endl;

        std::cout << std::endl;
        std::cout << std::setw(22) << "starting fens:" << std::setw(22)
                  << fens_ongoing.size() << std::endl;
        std::cout << std::setw(22) << "pieces:" << std::setw(22) << pieces_count
                  << std::endl;
        std::cout << std::setw(22) << "pawn progress:" << std::setw(22)
                  << pawnProgress << std::endl;
        std::cout << std::setw(22) << "progress index:" << std::setw(22)
                  << pI_now << std::endl;
        std::cout << std::setw(22) << "pending fens:" << std::setw(22)
                  << total_pending << std::endl;
        std::tie(mem_virt, mem_res) = get_memory();
        std::cout << std::setw(22) << "start timestamp:" << std::setw(22)
                  << getCurrentDateTime() << std::endl;
        std::cout << std::setw(22) << "virtual memory:" << std::setw(22)
                  << mem_virt << std::endl;
        std::cout << std::setw(22) << "resident memory:" << std::setw(22)
                  << mem_res << std::endl;

        // put the ongoing fens for this progressIndex in different sets
        // according to their needed depth;
        fens_depthIndex_t fens_depthIndex(depth + 1);
        for (auto &fp : fens_depthIndex)
          fp = new fen_set_t;

        for (auto itr = fens_ongoing.begin(); itr != fens_ongoing.end(); ++itr)
          fens_depthIndex[itr->second]->insert(itr->first);

        // Detailed info
        std::cout << std::endl;
        std::cout << std::setw(4) << "ply" << std::setw(18) << "iter count"
                  << std::setw(18) << "iter cumulative" << std::setw(18)
                  << "total count" << std::setw(18) << "total cumulative"
                  << std::endl;

        size_t iter_cumu = 0;
        size_t total_cumu = 0;
        for (int idepth = depth; idepth >= 0; idepth--) {

          size_t n_visited_start = visited_keys.size();

          ThreadPool pool(std::thread::hardware_concurrency());

          auto &fens_currentDepth = *fens_depthIndex[idepth];

          for (size_t i = 0; i < fens_currentDepth.subcnt(); ++i) {
            pool.enqueue(
                [&fens_currentDepth, &idepth, &handle, &stats, &visited_keys,
                 &fens_depthIndex, &fens_progressIndex, &maxCPLoss](size_t i) {
                  fens_currentDepth.with_submap(
                      i, [&](const fen_set_t::EmbeddedSet &set) {
                        explore(set, idepth, handle, stats, visited_keys,
                                fens_depthIndex, fens_progressIndex, maxCPLoss);
                      });
                },
                i);
          }

          pool.wait();

          size_t n_visited_stop = visited_keys.size();

          int ply = depth - idepth;
          iter_counts[ply] = n_visited_stop - n_visited_start;
          total_counts[ply] += iter_counts[ply];
          iter_cumu += iter_counts[ply];
          total_cumu += total_counts[ply];
          std::cout << std::setw(4) << ply << std::setw(18) << iter_counts[ply]
                    << std::setw(18) << iter_cumu << std::setw(18)
                    << total_counts[ply] << std::setw(18) << total_cumu
                    << std::endl;

          delete fens_depthIndex[idepth];
        }

        auto t_end = std::chrono::high_resolution_clock::now();
        double elapsed_time_sec =
            std::chrono::duration<float>(t_end - t_start).count();

        auto total_t_end = std::chrono::high_resolution_clock::now();
        double total_elapsed_time_sec =
            std::chrono::duration<float>(total_t_end - total_t_start).count();

        size_t iter_getss = size_t(stats.gets / elapsed_time_sec);
        total_gets += stats.gets;
        size_t total_getss = size_t(total_gets / total_elapsed_time_sec);

        size_t iter_hitss = size_t(stats.hits / elapsed_time_sec);
        total_hits += stats.hits;
        size_t total_hitss = size_t(total_hits / total_elapsed_time_sec);

        size_t iter_nodess = size_t(stats.nodes / elapsed_time_sec);
        total_nodes += stats.nodes;
        size_t total_nodess = size_t(total_nodes / total_elapsed_time_sec);

        size_t iter_assigned = visited_keys.size();
        size_t iter_assigneds = size_t(iter_assigned / elapsed_time_sec);
        total_assigned += iter_assigned;
        size_t total_assigneds =
            size_t(total_assigned / total_elapsed_time_sec);

        // Debrief
        std::cout << std::endl;
        std::tie(mem_virt, mem_res) = get_memory();
        std::cout << std::setw(22) << "end timestamp:" << std::setw(22)
                  << getCurrentDateTime() << std::endl;
        std::cout << std::setw(22) << "virtual memory:" << std::setw(22)
                  << mem_virt << std::endl;
        std::cout << std::setw(22) << "resident memory:" << std::setw(22)
                  << mem_res << std::endl;
        std::cout << std::setw(22) << "iteration time:" << std::fixed
                  << std::setw(22) << std::setprecision(3) << elapsed_time_sec
                  << std::endl;
        std::cout << std::setw(22) << "total time:" << std::fixed
                  << std::setw(22) << std::setprecision(3)
                  << total_elapsed_time_sec << std::endl;

        std::cout << std::endl;
        std::cout << std::setw(4) << "  " << std::setw(18) << "iter assigned"
                  << std::setw(18) << "iter assigned/s" << std::setw(18)
                  << "total assigned" << std::setw(18) << "total assigned/s"
                  << std::endl;
        std::cout << std::setw(4) << "  " << std::setw(18) << iter_assigned
                  << std::setw(18) << iter_assigneds << std::setw(18)
                  << total_assigned << std::setw(18) << total_assigneds
                  << std::endl;

        std::cout << std::setw(4) << "  " << std::setw(18) << "iter DB gets"
                  << std::setw(18) << "iter DB gets/s" << std::setw(18)
                  << "total DB gets" << std::setw(18) << "total DB gets/s"
                  << std::endl;
        std::cout << std::setw(4) << "  " << std::setw(18) << stats.gets
                  << std::setw(18) << iter_getss << std::setw(18) << total_gets
                  << std::setw(18) << total_getss << std::endl;

        std::cout << std::setw(4) << "  " << std::setw(18) << "iter DB hits"
                  << std::setw(18) << "iter DB hits/s" << std::setw(18)
                  << "total DB hits" << std::setw(18) << "total DB hits/s"
                  << std::endl;
        std::cout << std::setw(4) << "  " << std::setw(18) << stats.hits
                  << std::setw(18) << iter_hitss << std::setw(18) << total_hits
                  << std::setw(18) << total_hitss << std::endl;

        std::cout << std::setw(4) << "  " << std::setw(18) << "iter nodes"
                  << std::setw(18) << "iter nodes/s" << std::setw(18)
                  << "total nodes" << std::setw(18) << "total nodes/s"
                  << std::endl;
        std::cout << std::setw(4) << "  " << std::setw(18) << stats.nodes
                  << std::setw(18) << iter_nodess << std::setw(18)
                  << total_nodes << std::setw(18) << total_nodess << std::endl;

        // Prepare for next iter
        visited_keys.clear();
      }
      // Done with this map
      delete fens_progressIndex[pI_now];
    }
  }

  std::cout << std::endl;
  std::cout << "Finished all iterations! " << std::endl;

  std::cout << "Closing DB" << std::endl;
  handle = cdbdirect_finalize(handle);
  return 0;
}
