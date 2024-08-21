#include <atomic>
#include <chrono>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <map>
#include <mutex>

#include "external/chess.hpp"
#include "external/parallel_hashmap/phmap.h"
#include "external/threadpool.hpp"

#include "cdbdirect.h"

using namespace chess;

using zobrist_map_t = phmap::parallel_flat_hash_map<
    std::uint64_t, std::int16_t, std::hash<std::uint64_t>,
    std::equal_to<std::uint64_t>,
    std::allocator<std::pair<std::uint64_t, std::int16_t>>, 8, std::mutex>;

using fen_map_t = phmap::parallel_flat_hash_map<
    std::string, std::int16_t, std::hash<std::string>,
    std::equal_to<std::string>,
    std::allocator<std::pair<std::string, std::int16_t>>, 8, std::mutex>;

using poslist_t = std::map<std::string, int>;

// collect brute force all fens reachable up to a given depth, useful for a
// quick precompute?
void collect(Board &board, int depth, bool allow_captures,
             std::uintptr_t handle, std::atomic<size_t> &total_gets,
             poslist_t &poslist) {

  std::string fen = board.getFen(false);
  std::vector<std::pair<std::string, int>> result = cdbdirect_get(handle, fen);
  total_gets++;
  size_t n_elements = result.size();
  int ply = result[n_elements - 1].second;

  // not in DB
  if (ply == -2)
    return;

  if (poslist.contains(fen)) {
    if (poslist[fen] < depth)
      poslist[fen] = depth;
  } else {
    poslist[fen] = depth;
  }

  // No remaining depth
  if (depth <= 0)
    return;

  // No moves to explore
  if (n_elements <= 1)
    return;

  for (auto &pair : result)
    if (pair.first != "a0a0") {
      Move m = uci::uciToMove(board, pair.first);
      if (!board.isCapture(m) || allow_captures) {
        board.makeMove<true>(m);
        collect(board, depth - 1, allow_captures, handle, total_gets, poslist);
        board.unmakeMove(m);
      }
    }
}

// main function, generates a map of all visited keys with their maximum depth
void explore(Board &board, int depth, bool allow_captures,
             std::uintptr_t handle, std::atomic<size_t> &total_gets,
             zobrist_map_t &visited_keys, fen_map_t &fens_todo) {

  std::uint64_t key = board.hash();

  // quick exist if this has already been explored at equal or higher depth
  int found_depth = -1;
  visited_keys.if_contains(key, [&found_depth](zobrist_map_t::value_type &p) {
    found_depth = p.second;
  });
  if (found_depth >= depth)
    return;

  // probe DB
  std::string fen = board.getFen(false);
  std::vector<std::pair<std::string, int>> result = cdbdirect_get(handle, fen);
  total_gets++;
  size_t n_elements = result.size();
  int ply = result[n_elements - 1].second;

  // not in DB
  if (ply == -2)
    return;

  // In DB, add to map
  std::int16_t value;
  bool is_new_entry = visited_keys.lazy_emplace_l(
      std::move(key),
      [&](zobrist_map_t::value_type &p) {
        value = p.second;
        p.second = std::max(p.second, std::int16_t(depth));
      },
      [&](const zobrist_map_t::constructor &ctor) {
        ctor(std::move(key), depth);
        value = std::int16_t(depth - 1);
      });

  // Meanwhile already visited at sufficient depth
  if (value >= depth)
    return;

  // No remaining depth
  if (depth <= 0)
    return;

  // No moves to explore
  if (n_elements <= 1)
    return;

  // Now explore the remaining moves
  for (auto &pair : result)
    if (pair.first != "a0a0") {
      Move m = uci::uciToMove(board, pair.first);
      bool isCapture = board.isCapture(m);
      board.makeMove<true>(m);
      if (!isCapture || allow_captures) {
        explore(board, depth - 1, allow_captures, handle, total_gets,
                visited_keys, fens_todo);
      } else {
        std::string fen = board.getFen(false);
        fens_todo.lazy_emplace_l(
            std::move(fen),
            [&fen, &depth](fen_map_t::value_type &p) {
              p.second = std::max(p.second, std::int16_t(depth - 1));
            },
            [&fen, &depth](const fen_map_t::constructor &ctor) {
              ctor(std::move(fen), depth - 1);
            });
      }
      board.unmakeMove(m);
    }

  return;
}

int main() {
  std::string fen;
  // 1. e4
  fen = "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq - 0 1";
  // 1. b3
  fen = "rnbqkbnr/pppppppp/8/8/8/1P6/P1PPPPPP/RNBQKBNR b KQkq - 0 1";
  // 1. g4
  fen = "rnbqkbnr/pppppppp/8/8/6P1/8/PPPPPP1P/RNBQKBNR b KQkq - 0 1";

  int depth = 14;

  bool allow_captures = false;

  std::cout << "Exploring fen: " << fen << std::endl;
  std::cout << "Max depth: " << depth << std::endl;
  std::cout << "Allowing captures: " << allow_captures << std::endl;

  std::cout << "Opening DB" << std::endl;
  std::uintptr_t handle = cdbdirect_initialize("/mnt/ssd/chess-20240814/data");

  // counters
  std::atomic<size_t> total_gets = 0;
  Board board(fen);

  std::cout << "sequential prepare" << std::endl;

  zobrist_map_t visited_keys;
  fen_map_t fens_todo_1;
  fen_map_t fens_todo_2;

  // sequential prepare, generate enough fens in the tree to parallelize
  poslist_t poslist;
  int edepth = -1;
  while (poslist.size() <= 10 * std::thread::hardware_concurrency() &&
         edepth <= depth) {
    edepth++;
    collect(board, edepth, allow_captures, handle, total_gets, poslist);
  }

  for (const auto &[fen, fendepth] : poslist)
    fens_todo_1[fen] = depth - edepth + fendepth;

  // Start exploring.
  std::cout << "Exploring tree" << std::endl;

  fen_map_t &fens_ongoing = fens_todo_1;
  fen_map_t &fens_new = fens_todo_2;

  std::vector<size_t> total_counts(depth + 1, 0);
  size_t total_visited = 0;

  auto total_t_start = std::chrono::high_resolution_clock::now();

  size_t iter = 0;

  while (fens_ongoing.size() > 0) {

    std::atomic<size_t> iter_gets = 0;
    std::vector<size_t> iter_counts(depth + 1, 0);

    iter++;
    std::cout << std::endl;
    size_t pieces_count = Board(fens_ongoing.begin()->first).occ().count();
    std::cout << "Iteration : " << iter << " starting from " << fens_ongoing.size() << " fens with " << pieces_count << " pieces" << std::endl;
    auto t_start = std::chrono::high_resolution_clock::now();

    // sort with the low depths first, more efficient, but also more memory
    std::vector<std::pair<std::string,std::int16_t>> todos;
    for (auto itr = fens_ongoing.begin(); itr != fens_ongoing.end(); ++itr)
      todos.push_back(*itr);
    fens_ongoing.clear();

    std::sort(todos.begin(), todos.end(),
              [](std::pair<std::string, std::int16_t> &a, std::pair<std::string, std::int16_t> &b) {
                return a.second < b.second;
              });

    ThreadPool pool(std::thread::hardware_concurrency());

    for (const auto &[fen, fendepth] : todos) {
      size_t size = pool.enqueue(
          [&allow_captures, &handle, &iter_gets, &visited_keys,
           &fens_new](std::string fen, int depth) {
            Board board(fen);
            explore(board, depth, allow_captures, handle, iter_gets,
                    visited_keys, fens_new);
          },
          fen, fendepth);
      // limit the size of the queue in the pool, no need to have it very long
      // if (size >= 5000 * std::thread::hardware_concurrency())
      // {
      //     using namespace std::chrono_literals;
      //     std::this_thread::sleep_for(20ms);
      // }
    }

    pool.wait();
    auto t_end = std::chrono::high_resolution_clock::now();
    double elapsed_time_sec =
        std::chrono::duration<float>(t_end - t_start).count();

    auto total_t_end = std::chrono::high_resolution_clock::now();
    double total_elapsed_time_sec =
        std::chrono::duration<float>(total_t_end - total_t_start).count();

    size_t iter_getss = size_t(iter_gets / elapsed_time_sec);

    total_gets += iter_gets;
    size_t total_getss = size_t(total_gets / total_elapsed_time_sec);

    size_t iter_visited = visited_keys.size();
    total_visited += iter_visited;

    // Debrief
    std::cout << std::setw(4) << "  " << std::setw(18) << "iter time" << std::setw(18) << "iter count"
                                      << std::setw(18) << "total time" << std::setw(18) << "total count" << std::endl;
    std::cout << std::setw(4) << "  " << std::setw(18) << elapsed_time_sec << std::setw(18) << iter_visited
                                      << std::setw(18) << total_elapsed_time_sec << std::setw(18) << total_visited << std::endl;

    std::cout << std::setw(4) << "  " << std::setw(18) <<   "iter DB gets"  << std::setw(18) << "iter DB gets/s"
                                      << std::setw(18) <<   "total DB gets"  << std::setw(18) << "total DB gets/s" << std::endl;
    std::cout << std::setw(4) << "  " << std::setw(18) <<   iter_gets       << std::setw(18) << iter_getss
                                      << std::setw(18) <<   total_gets      << std::setw(18) << total_getss << std::endl;

    // Detailed info
    std::cout << std::setw(4) << "ply" << std::setw(18) << "iter count" << std::setw(18) << "iter cumulative"
                                       << std::setw(18) << "total count" << std::setw(18) << "total cumulative" << std::endl;

    for (const auto &[key, value] : visited_keys)
      iter_counts[depth - value]++;

    size_t iter_cumu = 0;
    size_t total_cumu = 0;
    for (int ply = 0; ply <= depth; ply++) {
      total_counts[ply] += iter_counts[ply];
      iter_cumu  += iter_counts[ply];
      total_cumu += total_counts[ply];
      std::cout << std::setw(4) << ply << std::setw(18) << iter_counts[ply] << std::setw(18) << iter_cumu
                                       << std::setw(18) << total_counts[ply] << std::setw(18) << total_cumu << std::endl;
    }


    // Prepare for next iter
    std::swap(fens_ongoing, fens_new);
    fens_new.clear();
    visited_keys.clear();
  }

  std::cout << std::endl;
  std::cout << "Finished all iterations! " << std::endl;
  std::cout << "Closing DB" << std::endl;

  // close DB
  handle = cdbdirect_finalize(handle);

  return 0;
}
