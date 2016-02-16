#include <iostream>
#include <queue>
#include <vector>
#include <future>
#include <mutex>
#include <thread>
#include <chrono>
#include <sstream>
#include <algorithm>
#include <map>
#include <x86intrin.h>
#include "Queue.h"
#include "Tracking.h"

constexpr std::uint64_t LOOPS = 1000000;
constexpr std::uint64_t PRINT_MOD = LOOPS / 10;

class TrackingStats
{
public:
  void addEntry( std::uint64_t entry) {
    _entries.push_back( entry);
    if ( (_entries.size() % PRINT_MOD) == 0) {
      _resizeEntries();     
    }
  }

  void printStats() {
    if ( _entries.size() != 0) {
      _resizeEntries();
    }

    std::stringstream msg;
    msg << "Thread: " << std::this_thread::get_id() << "\n"
        << "  Sum: " << _sum << "\n"
        << "  Avg: " << (_sum / _count) << "\n"
        << "  Max: " << _max << "\n";

    for ( auto& ptiles : _ptile_arrays) {
      msg << "  " << (ptiles.first * 100.0) << "\%ile: " << (std::accumulate( ptiles.second.begin(), ptiles.second.end(), 0) / ptiles.second.size())  << "\n";
    }

    std::cout << msg.str();
  }

  void _resizeEntries() {
    std::sort( _entries.begin(), _entries.end());
    _sum   += std::accumulate( _entries.begin(), _entries.end(), 0);
    _count += _entries.size();
    if ( _entries.back() > _max) {
      _max = _entries.back();
    }

    _ptile_arrays[ 0.80 ].push_back( _entries[ 0.80 * _entries.size() ]);
    _ptile_arrays[ 0.90 ].push_back( _entries[ 0.90 * _entries.size() ]);
    _ptile_arrays[ 0.99 ].push_back( _entries[ 0.99 * _entries.size() ]);
    _ptile_arrays[ 0.999 ].push_back( _entries[ 0.999 * _entries.size() ]);
    _ptile_arrays[ 0.9999 ].push_back( _entries[ 0.9999 * _entries.size() ]);

    _entries.clear();
  }

private:
  std::uint64_t _sum   = 0;
  std::uint64_t _count = 0;
  std::uint64_t _max   = 0;

  std::vector<std::uint64_t>                   _entries;
  std::map<double, std::vector<std::uint64_t>> _ptile_arrays;
};

void r1() {
  constexpr int startNum = 5000;
  TrackingStats ts;
  std::uint32_t aux;

  for ( std::uint64_t i = startNum; i < startNum + LOOPS; i++) {
    std::uint64_t start = __rdtsc();
    {
      std::uint16_t i_cast = static_cast<std::uint16_t>( i);
      Counter c( static_cast<std::uint16_t>(startNum + 1), Token{ i_cast, i_cast});
      {
        PreSampler c( static_cast<std::uint16_t>(startNum + 2), Token{ i_cast, i_cast});
      }
    }
    std::uint64_t end = __rdtsc();
    ts.addEntry( end - start);

    //if ( (i - startNum) % PRINT_MOD == 0) {
    //  std::cout << i << ": " << (end - start) << std::endl;
    //}

    //std::this_thread::sleep_for( std::chrono::microseconds( 5));
  }

  ts.printStats();
}

void r2() {
  constexpr int startNum = 3000;
  TrackingStats ts;
  std::uint32_t aux;

  for ( std::uint64_t i = startNum; i < startNum + LOOPS; i++) {
    std::uint64_t start = __rdtsc();
    {
      std::uint16_t i_cast = static_cast<std::uint16_t>( i);
      Counter c( static_cast<std::uint16_t>(startNum + 1), Token{ i_cast, i_cast});
      {
        Counter c( static_cast<std::uint16_t>(startNum + 2), Token{ i_cast, i_cast});
      }
    }
    std::uint64_t end = __rdtsc();
    ts.addEntry( end - start);

    //if ( (i - startNum) % PRINT_MOD == 0) {
    //  std::cout << i << ": " << (end - start) << std::endl;
    //}

    //std::this_thread::sleep_for( std::chrono::microseconds( 10));
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
