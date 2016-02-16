#ifndef __SPSC_QUEUE_H__
#define __SPSC_QUEUE_H__

#include <queue>
#include <boost/lockfree/spsc_queue.hpp>
#include "spsc_impl/ProducerConsumerQueue.h"
#include "spsc_impl/readerwriterqueue.h"

// Empty SPSC Queue Class Definition
template <typename T, typename Queue> class SpscQueue;

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
    _queue.push( t);
    return true;
  }

  bool pop( T& t) {
    if ( !_queue.empty()) {
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

template <typename T>
class SpscQueue<T, folly::ProducerConsumerQueue<T>>
{
public:
  SpscQueue()
    : _queue( 1024)
  {}

  std::string name() {
    return "folly::ProducerConsumerQueue";
  }

  bool push( T const& t) {
    return _queue.write( t);
  }

  bool pop( T& t) {
    return _queue.read( t);
  }

private:
  folly::ProducerConsumerQueue<T> _queue;
};

// =========================================
// Boost Lockfree SPSC Queue Specialization
// =========================================

template <typename T>
class SpscQueue<T, boost::lockfree::spsc_queue<T>>
{
public:
  SpscQueue()
    : _queue( 1024)
  {}

  std::string name() {
    return "boost::lockfree::spsc_queue";
  }

  bool push( T const& t) {
    return _queue.push( t);
  }

  bool pop( T& t) {
    return _queue.pop( t);
  }
private:
  boost::lockfree::spsc_queue<T> _queue;
};

// =========================================
// MoodyCamel ReaderWriter SPSC Queue Specialization
// =========================================

template <typename T>
class SpscQueue<T, moodycamel::ReaderWriterQueue<T>>
{
public:
  std::string name() {
    return "moodycamel::ReaderWriterQueue";
  }

  bool push( T const& t) {
    return _queue.try_enqueue( t);
  }

  bool pop( T& t) {
    return _queue.try_dequeue( t);
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
