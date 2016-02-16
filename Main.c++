#include <iostream>
#include <queue>
#include <vector>
#include <future>
#include <mutex>
#include <thread>
#include <chrono>
#include <sstream>
#include <x86intrin.h>
#include "Queue.h"
#include "Tracking.h"

class TrackingStats
{
public:
  void addEntry( std::uint64_t entry) {
    _entries.push_back( entry);
  }

  void printStats() {
    std::vector<std::uint64_t> sorted_entries = getSortedEntries( _entries);
    std::uint64_t sum = std::accumulate( sorted_entries.begin(), sorted_entries.end(), 0);
    std::uint64_t avg = sum / sorted_entries.size();

    std::uint64_t ptile_99 = sorted_entries[ 0.99 * sorted_entries.size() ];
    std::uint64_t ptile_90 = sorted_entries[ 0.90 * sorted_entries.size() ];

    std::stringstream msg;
    msg << "Sum: " << sum << " Avg: " << avg << " 90th \%ile: " << ptile_90
        << " 99th \%ile: " << ptile_99 << " Max: " << sorted_entries.back() << "\n";

    std::cout << msg.str();
  }

  std::vector<std::uint64_t> getSortedEntries( std::vector<std::uint64_t> entries) {
    std::sort( std::begin( entries), std::end( entries));
    return entries;
  }

private:
  std::vector<std::uint64_t> _entries;
};

void r1() {
  constexpr int startNum = 5000;
  TrackingStats ts;

  std::uint32_t aux;
  for ( std::uint16_t i = startNum; i < startNum + 1000; i++) {
    std::uint64_t start = __rdtscp( &aux);
    {
      Counter c( startNum + 1, Token{ i, i});
      {
        PreSampler c( startNum + 2, Token{ i, i});
      }
    }
    std::uint64_t end = __rdtscp( &aux);
    ts.addEntry( end - start);

    std::this_thread::sleep_for( std::chrono::microseconds( 50));
  }
  ts.printStats();
}

void r2() {
  constexpr int startNum = 3000;
  TrackingStats ts;
  std::uint32_t aux;

  for ( std::uint16_t i = startNum; i < startNum + 1000; i++) {
    std::uint64_t start = __rdtscp( &aux);
    {
      Counter c( startNum + 1, Token{ i, i});
      {
        Counter c( startNum + 2, Token{ i, i});
      }
    }
    std::uint64_t end = __rdtscp( &aux);
    ts.addEntry( end - start);

    std::this_thread::sleep_for( std::chrono::microseconds( 100));
  }

  ts.printStats();
}

int main() {
  QueuePool<Delta>::initialize();
  QueuePool<Sample>::initialize();

  Counter::_queue = new Queue<Delta>;
  Counter::_queue->init();

  PreSampler::_queue = new Queue<Sample>;
  PreSampler::_queue->init();

  PostSampler::_queue = new Queue<Sample>;
  PostSampler::_queue->init();

  std::stringstream msg;
  msg << "Counter Queue:     " << Counter::_queue->name() << "\n"
      << "PreSampler Queue:  " << PreSampler::_queue->name() << "\n"
      << "PostSampler Queue: " << PostSampler::_queue->name() << "\n";
  std::cout << msg.str();

  std::thread t1( r1);
  std::thread t2( r2);

  t1.join();
  t2.join();

  std::this_thread::sleep_for( std::chrono::milliseconds( 500));

  delete Counter::_queue;

  return 0;
}
