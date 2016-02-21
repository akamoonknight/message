#ifndef __TRACKING_STATS_H__
#define __TRACKING_STATS_H__

#include <string>
#include <thread>
#include <vector>
#include <cstdint>
#include <map>

class TrackingStats
{
public:
  static constexpr std::uint16_t NUM_OUTLIERS = 3;

  void setPrintOccurrence( std::uint32_t print_occurrence);
  void setType( std::string type);
  void setId( std::thread::id id);
  void addEntry( std::uint64_t entry);
  void _resizeEntries();

  static void printDefaultTrackingStats( std::vector<TrackingStats*>);
  static void printTrackingStats( std::vector<TrackingStats*>);

private:
  std::uint64_t _sum   = 0;
  std::uint64_t _count = 0;
  std::uint64_t _min   = 50000000;
  std::uint64_t _max   = 0;

  std::uint32_t                                _print_occurrence;
  std::string                                  _type;
  std::thread::id                              _id;
  std::vector<std::uint64_t>                   _entries;
  std::map<double, std::vector<std::uint64_t>> _ptile_arrays;
  std::vector<std::uint64_t>                   _outliers;
};

#endif // __TRACKING_STATS_H__

