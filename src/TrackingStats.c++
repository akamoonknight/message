#include <algorithm>
#include <sstream>
#include <iostream>
#include "TrackingStats.h"

void
TrackingStats::setPrintOccurrence( std::uint32_t print_occurrence) {
  _print_occurrence = print_occurrence;
}

void
TrackingStats::setType( std::string type) {
  _type = type;
}

void
TrackingStats::setId( std::thread::id id) {
  _id = id;
}

void
TrackingStats::addEntry( std::uint64_t entry) {
  _entries.push_back( entry);
  if ( (_entries.size() % _print_occurrence) == 0) {
    _resizeEntries();     
  }
}

void
TrackingStats::_resizeEntries() {
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

  //for ( double ptile : {0.80, 0.90, 0.95, 0.99, 0.999, 0.9999, 0.99999, 0.999999}) {
  for ( double ptile : {0.80, 0.95, 0.9999}) {
    _ptile_arrays[ ptile ].push_back( _entries[ ptile * _entries.size() ]);
  }

  _entries.clear();
}

void
TrackingStats::printDefaultTrackingStats( std::vector<TrackingStats*> tss) {
  for ( TrackingStats* tsp : tss) {
    TrackingStats& ts = *tsp;
    if ( ts._entries.size() != 0) {
      ts._resizeEntries();
    }

    std::stringstream msg;
    msg << "Type: " << ts._type << " Id: " << ts._id << " Sum: " << ts._sum << "\n"
        << "Count: " << ts._count << " Avg: " << (ts._sum / ts._count) << "\n"
        << "Min: " << ts._min << " Max: " << ts._max << "\n";

    msg << "Percentils:\n";
    for ( auto& ptiles : ts._ptile_arrays) {
      msg << " -> " << (ptiles.first * 100.0) << "\%ile - " << (std::accumulate( ptiles.second.begin(), ptiles.second.end(), 0) / ptiles.second.size()) << "\n";
    }

    std::reverse( ts._outliers.begin(), ts._outliers.end());

    msg << "Outliers:\n";
    for ( auto& outlier : ts._outliers) {
      msg << " -> " << outlier << "\n";
    }

    std::cout << msg.str() << std::endl;
  }
}

void
TrackingStats::printTrackingStats( std::vector<TrackingStats*> tss) {
  std::stringstream msg;
  msg << "<table>\n"
      << "  <tbody>\n"
      << "    <tr>\n"
      << "      <th>Type</th>\n"
      << "      <th>Count</th>\n"
      << "      <th>Avg</th>\n"
      << "      <th>Min</th>\n"
      << "      <th>Max</th>\n"
      << "      <th>Percentiles</th>\n"
      << "      <th>Outliers</th>\n"
      << "    </tr>\n";
  for ( TrackingStats* tsp : tss) {
    TrackingStats& ts = *tsp;
    if ( ts._entries.size() != 0) {
      ts._resizeEntries();
    }

  msg << "    <tr>\n"
      << "      <td>" << ts._type << "</td>\n"
      << "      <td>" << ts._count << "</td>\n"
      << "      <td>" << (ts._sum / ts._count) << "</td>\n"
      << "      <td>" << ts._min << "</td>\n"
      << "      <td>" << ts._max << "</td>\n"
      << "      <td>\n"
      << "        <ul>\n"
      << "          ";
    for ( auto& ptiles : ts._ptile_arrays) {
  msg << "<li>" << (ptiles.first * 100.0) << "\%ile - " << (std::accumulate( ptiles.second.begin(), ptiles.second.end(), 0) / ptiles.second.size())  << "</li>";
    }
  msg << "\n"
      << "        </ul>\n"
      << "      </td>\n"
      << "      <td>\n"
      << "        <ul>\n"
      << "          ";
    std::reverse( ts._outliers.begin(), ts._outliers.end());
    for ( auto& outlier : ts._outliers) {
  msg << "<li>" << outlier << "</li>";
    }
  msg << "\n"
      << "        </ul>\n"
      << "      </td>\n"
      << "    </tr>\n";
  }

  msg << "  </tbody>\n"
      << "</table>\n";

  std::cout << msg.str();
}

