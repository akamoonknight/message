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
#include "TrackingStats.h"

constexpr std::uint64_t LOOPS = 1000000;
constexpr std::uint64_t PRINT_MOD = LOOPS / 1;

template <typename TrackingType>
void r1( TrackingStats* ts) {
  constexpr int startNum = 4000;
  std::uint32_t aux;

  ts->setId( std::this_thread::get_id());

  for ( std::uint64_t i = startNum; i < startNum + LOOPS; i++) {
    std::uint64_t start = __rdtscp( &aux);
    {
      TrackingType t( static_cast<std::uint16_t>(startNum + 2), Token{ static_cast<std::uint16_t>( i), static_cast<std::uint16_t>( i)});
    }
    std::uint64_t end = __rdtscp( &aux);
    ts->addEntry( end - start);
    std::this_thread::sleep_for( std::chrono::microseconds( 1));
  }
}

int main() {
  std::vector<std::thread>   threads;
  std::vector<TrackingStats*> tss;

  Counter::_queue = new Queue<Delta>;
  Counter::_queue->init();

  PreSampler::_queue = new Queue<Sample>;
  PreSampler::_queue->init();

  PostSampler::_queue = new Queue<Sample>;
  PostSampler::_queue->init();

  std::stringstream msg;
  msg << "`" << Counter::_queue->name() << "`\n";
  std::cout << msg.str();

  for ( int i = 0; i < 1; i++) {
    TrackingStats* ts1 = new TrackingStats;
    TrackingStats* ts2 = new TrackingStats;
    TrackingStats* ts3 = new TrackingStats;

    ts1->setPrintOccurrence( PRINT_MOD); ts1->setType( "Counter");
    ts2->setPrintOccurrence( PRINT_MOD); ts2->setType( "PreSampler");
    ts3->setPrintOccurrence( PRINT_MOD); ts3->setType( "PostSampler");

    tss.push_back( ts1);
    tss.push_back( ts2);
    tss.push_back( ts3);

    threads.push_back( std::thread( r1<Counter>, ts1));

    cpu_set_t cpuset;
    CPU_ZERO( &cpuset);
    CPU_SET( 0, &cpuset);
    int rc = pthread_setaffinity_np( threads[ 0 ].native_handle(), sizeof( cpu_set_t), &cpuset);

    threads.push_back( std::thread( r1<PreSampler>, ts2));
    CPU_ZERO( &cpuset);
    CPU_SET( 1, &cpuset);
    rc = pthread_setaffinity_np( threads[ 1 ].native_handle(), sizeof( cpu_set_t), &cpuset);

    threads.push_back( std::thread( r1<PostSampler>, ts3));
    CPU_ZERO( &cpuset);
    CPU_SET( 2, &cpuset);
    rc = pthread_setaffinity_np( threads[ 2 ].native_handle(), sizeof( cpu_set_t), &cpuset);
  }

  for ( std::thread& t : threads) {
    if ( t.joinable()) {
      t.join();
    }
  }

  //TrackingStats::printDefaultTrackingStats( tss);
  TrackingStats::printTrackingStats( tss);

  std::this_thread::sleep_for( std::chrono::milliseconds( 500));

  delete Counter::_queue;
  delete PreSampler::_queue;
  delete PostSampler::_queue;

  for ( TrackingStats* ts : tss) {
    delete ts;
  }

  return 0;
}
