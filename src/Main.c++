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

constexpr std::uint64_t LOOPS = 100000000;
constexpr std::uint64_t PRINT_MOD = LOOPS / 10;

class TrackingStats
{
public:
  static constexpr std::uint16_t NUM_OUTLIERS = 10;

  void setType( std::string type) {
    _type = type;
  }

  void setId( std::thread::id id) {
    _id = id;
  }

  void addEntry( std::uint64_t entry) {
    _entries.push_back( entry);
    if ( (_entries.size() % PRINT_MOD) == 0) {
      _resizeEntries();     
    }
  }

  void _resizeEntries() {
    std::sort( _entries.begin(), _entries.end());
    _sum   += std::accumulate( _entries.begin(), _entries.end(), 0);
    _count += _entries.size();
    if ( _entries.back() < _min) {
      _min = _entries.front();
    }
    if ( _entries.back() > _max) {
      _max = _entries.back();
    }

    for ( int i = _entries.size() - NUM_OUTLIERS; i < _entries.size(); i++) {
      _outliers.push_back( _entries[ i ]);
    }

    std::sort( _outliers.begin(), _outliers.end(), std::greater<std::uint64_t>());
    _outliers.resize( NUM_OUTLIERS);

    _ptile_arrays[ 0.80 ].push_back( _entries[ 0.80 * _entries.size() ]);
    _ptile_arrays[ 0.90 ].push_back( _entries[ 0.90 * _entries.size() ]);
    _ptile_arrays[ 0.99 ].push_back( _entries[ 0.99 * _entries.size() ]);
    _ptile_arrays[ 0.999 ].push_back( _entries[ 0.999 * _entries.size() ]);
    _ptile_arrays[ 0.9999 ].push_back( _entries[ 0.9999 * _entries.size() ]);
    _ptile_arrays[ 0.99999 ].push_back( _entries[ 0.99999 * _entries.size() ]);
    _ptile_arrays[ 0.999999 ].push_back( _entries[ 0.999999 * _entries.size() ]);

    _entries.clear();
  }

private:
  friend void printTrackingStats( std::initializer_list<TrackingStats>);
  std::uint64_t _sum   = 0;
  std::uint64_t _count = 0;
  std::uint64_t _min   = 50000000;
  std::uint64_t _max   = 0;

  std::string                                  _type;
  std::thread::id                              _id;
  std::vector<std::uint64_t>                   _entries;
  std::map<double, std::vector<std::uint64_t>> _ptile_arrays;
  std::vector<std::uint64_t>                   _outliers;
};

void printTrackingStats( std::initializer_list<TrackingStats> tss) {
  std::stringstream msg;
  msg << "<table><tbody>\n";
  msg << "<tr><th>Type</th><th>Thread</th><th>Sum</th><th>Count</th><th>Avg</th><th>Min</th><th>Max</th><th>Percentiles</th><th>Outliers</th></tr>\n";
  for ( TrackingStats ts : tss) {
    if ( ts._entries.size() != 0) {
      ts._resizeEntries();
    }

    msg << "<tr><td>" << ts._type << "</td><td>" << ts._id << "</td><td>" << ts._sum << "</td><td>" << ts._count << "</td><td>"
        << (ts._sum / ts._count) << "</td><td>" << ts._min << "</td><td>" << ts._max << "</td><td>";

    msg << "<ul>";
    for ( auto& ptiles : ts._ptile_arrays) {
      msg << "<li>" << (ptiles.first * 100.0) << "\%ile - " << (std::accumulate( ptiles.second.begin(), ptiles.second.end(), 0) / ptiles.second.size())  << "</li>";
    }
    msg << "</ul></td><td>";

    std::reverse( ts._outliers.begin(), ts._outliers.end());

    msg << "<ul>";
    for ( auto& outlier : ts._outliers) {
      msg << "<li>" << outlier << "</li>";
    }
    msg << "</ul></td></tr>\n";
  }

  msg << "</tbody></table>\n";

  std::cout << msg.str();
}

// Loop that uses pre samplers
void r1( TrackingStats* ts) {
  constexpr int startNum = 4000;
  std::uint32_t aux;

  ts->setId( std::this_thread::get_id());

  for ( std::uint64_t i = startNum; i < startNum + LOOPS; i++) {
    std::uint64_t start = __rdtscp( &aux);
    {
      Counter c( static_cast<std::uint16_t>(startNum + 2), Token{ static_cast<std::uint16_t>( i), static_cast<std::uint16_t>( i)});
    }
    std::uint64_t end = __rdtscp( &aux);
    ts->addEntry( end - start);
    //std::this_thread::sleep_for( std::chrono::microseconds( 10));
  }
}

// Loop that uses pre samplers
void r2( TrackingStats* ts) {
  constexpr int startNum = 5000;
  std::uint32_t aux;

  ts->setId( std::this_thread::get_id());

  for ( std::uint64_t i = startNum; i < startNum + LOOPS; i++) {
    std::uint64_t start = __rdtscp( &aux);
    {
      PreSampler c( static_cast<std::uint16_t>(startNum + 2), Token{ static_cast<std::uint16_t>( i), static_cast<std::uint16_t>( i)});
    }
    std::uint64_t end = __rdtscp( &aux);
    ts->addEntry( end - start);
    //std::this_thread::sleep_for( std::chrono::microseconds( 10));
  }
}

// Loop that uses post samplers
void r3( TrackingStats* ts) {
  constexpr int startNum = 6000;
  std::uint32_t aux;

  ts->setId( std::this_thread::get_id());

  for ( std::uint64_t i = startNum; i < startNum + LOOPS; i++) {
    std::uint64_t start = __rdtscp( &aux);
    {
      PostSampler c( static_cast<std::uint16_t>(startNum + 2), Token{ static_cast<std::uint16_t>( i), static_cast<std::uint16_t>( i)});
    }
    std::uint64_t end = __rdtscp( &aux);
    ts->addEntry( end - start);
    //std::this_thread::sleep_for( std::chrono::microseconds( 10));
  }
}

int main() {
  Counter::_queue = new Queue<Delta>;
  Counter::_queue->init();

  PreSampler::_queue = new Queue<Sample>;
  PreSampler::_queue->init();

  PostSampler::_queue = new Queue<Sample>;
  PostSampler::_queue->init();

  std::stringstream msg;
  msg << "`" << Counter::_queue->name() << "`\n";
  std::cout << msg.str();

  TrackingStats ts1, ts2, ts3;
  ts1.setType( "Counter");
  ts2.setType( "PreSampler");
  ts3.setType( "PostSampler");

  std::thread t1( r1, &ts1);
  std::thread t2( r2, &ts2);
  std::thread t3( r3, &ts3);

  t1.join();
  t2.join();
  t3.join();

  printTrackingStats( {ts1, ts2, ts3});

  std::this_thread::sleep_for( std::chrono::milliseconds( 500));

  delete Counter::_queue;
  delete PreSampler::_queue;
  delete PostSampler::_queue;

  return 0;
}
