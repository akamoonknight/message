#ifndef __SPSC_QUEUE_H__
#define __SPSC_QUEUE_H__

#include <queue>
#include <atomic>
#include <boost/lockfree/spsc_queue.hpp>
#include "spsc_impl/ProducerConsumerQueue.h"
#include "spsc_impl/readerwriterqueue.h"

struct DefaultQueueQualities
{
  static constexpr std::size_t Size = 1024;
};

class ToggleableObject
{
public:
  bool isToggled() { return _toggled.load( std::memory_order_relaxed); }
  bool toggle() { _toggled = !_toggled; }

private:
  std::atomic<bool> _toggled{ true};
};

// Empty SPSC Queue Class Definition
template <typename T, typename Queue, typename QueueQualities = DefaultQueueQualities> class SpscQueue;

// SpscQueue specializations should contain the calls:
//        std::string name() -> name of the type of underlying queue
// ,      bool push( T const& t) -> returns true if t was placed onto the queue
// and    bool pop( T& t) -> returns true if an object was taken
//                           off the queue and put into t
//   

// ==============================
// Standard Queue Specialization
// ==============================

template <typename T>
class SpscQueue<T, std::queue<T>>
{
public:
  std::string name() {
    return "std::queue";
  }

  bool push( T const& t) {
    if ( this->isToggled()) {
      _queue.push( t);
      return true;
    }
    return false;
  }

  bool pop( T& t) {
    if ( !_queue.empty() && this->isToggled()) {
      t = _queue.front();
      _queue.pop();
      return true;
    }

    return false;
  }

private:
  std::queue<T> _queue;
};

// =============================================
// Folly Producer Consumer Queue Specialization
// =============================================

template <typename T, typename QueueQualities>
class SpscQueue<T, folly::ProducerConsumerQueue<T>, QueueQualities> : public ToggleableObject
{
public:
  SpscQueue()
    : _queue( QueueQualities::Size)
  {}

  std::string name() {
    return "folly::ProducerConsumerQueue";
  }

  bool push( T const& t) {
    if ( this->isToggled()) {
      return _queue.write( t);
    }
    return false;
  }

  bool pop( T& t) {
    if ( this->isToggled()) {
      return _queue.read( t);
    }
    return false;
  }

private:
  folly::ProducerConsumerQueue<T> _queue;
};

// =========================================
// Boost Lockfree SPSC Queue Specialization
// =========================================

template <typename T, typename QueueQualities>
class SpscQueue<T, boost::lockfree::spsc_queue<T>, QueueQualities> : public ToggleableObject
{
public:
  SpscQueue()
    : _queue( QueueQualities::Size)
  {}

  std::string name() {
    return "boost::lockfree::spsc_queue";
  }

  bool push( T const& t) {
    if ( this->isToggled()) {
      return _queue.push( t);
    }
    return false;
  }

  bool pop( T& t) {
    if ( this->isToggled()) {
      return _queue.pop( t);
    }
    return false;
  }
private:
  boost::lockfree::spsc_queue<T> _queue;
};

// =========================================
// MoodyCamel ReaderWriter SPSC Queue Specialization
// =========================================

template <typename T, typename QueueQualities>
class SpscQueue<T, moodycamel::ReaderWriterQueue<T>, QueueQualities> : public ToggleableObject
{
public:
  SpscQueue()
    : _queue( QueueQualities::Size)
  {}

  std::string name() {
    return "moodycamel::ReaderWriterQueue";
  }

  bool push( T const& t) {
    if ( this->isToggled()) {
      return _queue.try_enqueue( t);
    }
    return false;
  }

  bool pop( T& t) {
    if ( this->isToggled()) {
      return _queue.try_dequeue( t);
    }
    return false;
  }
private:
  moodycamel::ReaderWriterQueue<T> _queue;
};

// ==================================
// System Wide SPSC Queue Type Alias
// ==================================

template <typename T>
using spsc_queue_t = SpscQueue<T, folly::ProducerConsumerQueue<T>>;

#endif // __SPSC_QUEUE_H__
