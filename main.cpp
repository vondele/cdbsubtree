#include <atomic>
#include <chrono>
#include <cstdint>
#include <map>
#include <mutex>
#include <iostream>
#include <iomanip>

#include "external/chess.hpp"
#include "external/parallel_hashmap/phmap.h"
#include "external/threadpool.hpp"

#include "cdbdirect.h"

using namespace chess;

using zobrist_map_t = phmap::parallel_flat_hash_map<
    std::uint64_t, std::int16_t, std::hash<std::uint64_t>,
    std::equal_to<std::uint64_t>,
    std::allocator<std::pair<std::uint64_t, std::int16_t>>, 8, std::mutex>;
using poslist_t = std::map<std::string, int>;

// collect brute force all fens reachable up to a given depth, useful for a quick precompute?
void collect(Board &board, int depth, std::uintptr_t handle,
             std::atomic<size_t> &total_gets, poslist_t &poslist) {

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
      board.makeMove<true>(m);
      collect(board, depth - 1, handle, total_gets, poslist);
      board.unmakeMove(m);
    }
}

// main function, generates a map of all visited keys with their maximum depth
void explore(Board &board, int depth, std::uintptr_t handle,
             std::atomic<size_t> &total_gets, zobrist_map_t &visited_keys) {

  std::uint64_t key = board.hash();

  // quick exist if this has already been explored at equal or higher depth
  int found_depth = -1;
  visited_keys.if_contains(key,[&found_depth](zobrist_map_t::value_type &p) { found_depth = p.second; });
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
      board.makeMove<true>(m);
      explore(board, depth - 1, handle, total_gets, visited_keys);
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

  int depth = 8;

  std::cout << "Exploring fen: " << fen << std::endl;
  std::cout << "Max depth: " << depth << std::endl;

  std::cout << "Opening DB" << std::endl;
  std::uintptr_t handle = cdbdirect_initialize("/mnt/ssd/chess-20240814/data");

  // counters
  std::atomic<size_t> total_gets = 0;
  Board board(fen);

  std::cout << "sequential prepare" << std::endl;

  // sequential prepare, generate enough fens in the tree to parallelize
  poslist_t poslist;
  int edepth = -1;
  while (poslist.size() <= 10 * std::thread::hardware_concurrency() &&
         edepth <= depth) {
    edepth++;
    collect(board, edepth, handle, total_gets, poslist);
  }

  // sort with the low depths first
  std::vector<std::pair<std::string, int>> todos;
  for (auto itr = poslist.begin(); itr != poslist.end(); ++itr)
    todos.push_back(*itr);
  std::sort(todos.begin(), todos.end(),
            [](std::pair<std::string, int> &a, std::pair<std::string, int> &b) {
              return a.second < b.second;
            });

  // The big one...
  // The memory size allocated is actually roughly 2 * map_reserve_size, and can be 87.5% occupied
  size_t map_reserve_size = 3 * 1024 * size_t(1024 * 1024);
  std::cout << "Reserving map of size " << map_reserve_size << std::endl;

  zobrist_map_t visited_keys;
  visited_keys.reserve(map_reserve_size);
  std::cout << "Bucket count: " << visited_keys.bucket_count() << std::endl;
  std::cout << "Estimated memory use: " << visited_keys.bucket_count() * (sizeof(zobrist_map_t::value_type) + 1) << std::endl;

  // Start exploring.
  std::cout << "Exploring tree" << std::endl;

  auto t_start = std::chrono::high_resolution_clock::now();
  ThreadPool pool(std::thread::hardware_concurrency());

  for (const auto &[fen, fendepth] : todos) {
    int rdepth = depth - edepth + fendepth;
    pool.enqueue(
        [&handle, &total_gets, &visited_keys](std::string fen, int depth) {
          Board board(fen);
          explore(board, depth, handle, total_gets, visited_keys);
        },
        fen, rdepth);
  }

  pool.wait();

  auto t_end = std::chrono::high_resolution_clock::now();
  double elapsed_time_microsec =
      std::chrono::duration<double, std::micro>(t_end - t_start).count();

  // Debrief
  std::cout << "Done!" << std::endl;
  std::cout << "  Total number of DB gets: " << total_gets << std::endl;
  std::cout << "  Duration (sec) " << elapsed_time_microsec / (1000 * 1000)
            << std::endl;
  std::cout << "  DB gets per second: "
            << int(total_gets * (1000 * 1000) / elapsed_time_microsec)
            << std::endl;

  std::cout << "Number of cdb positions reachable from fen: "
            << visited_keys.size() << std::endl;

  // Detailed info
  std::cout << "Detailed stats:" << std::endl;
  std::cout << std::setw(4) << "ply    " << std::setw(12) << "count" << std::setw(12) << "cumulative" << std::endl;
  std::vector<size_t> counts(depth + 1, 0);
  for (const auto &[key, value] : visited_keys)
      counts[depth - value]++;
  size_t total = 0;
  for (int ply=0; ply<=depth; ply++)
  {
      total += counts[ply];
      std::cout <<  std::setw(4) << ply << " : " << std::setw(12) << counts[ply] << " " << std::setw(12) << total << std::endl;
  }

  // close DB
  handle = cdbdirect_finalize(handle);

  return 0;
}
