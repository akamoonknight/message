#ifndef __QUEUE_H__
#define __QUEUE_H__

#include <vector>
#include <mutex>
#include <future>
#include <iostream>
#include "SpscQueue.h"

template <typename T>
class Queue
{
public:
  using rw_mutex_t  = std::mutex; // TODO: Change these to use boost rw mutex impl
  using rw_wrlock_t = std::lock_guard<rw_mutex_t>;
  using rw_rdlock_t = std::unique_lock<rw_mutex_t>;
  using queue_t     = spsc_queue_t<T>;
  using queue_ptr_t = queue_t*;

  static constexpr std::uint16_t NUM_QUEUES = 4;

  // Create the internal queue
  Queue()
    : _keep_running( false)
    , _queue( new queue_t)
  {}

  // Stop the consuming thread
  ~Queue() {
    _keep_running = false;
    if ( _future.valid()) {
      _future.get();
    }
  }

  std::string name() {
    return _queue->name();
  }

  // Used by the Counters/Samplers to push their
  // objects onto the thread local queue
  bool produce( T const& t) {
    if ( _thread_queue == nullptr) {
      _initialize_thread_queue();
    }

    _thread_queue->push( t);
  }

  bool consume( T& t) {
    return _queue->pop( t);
  }

  // Either grabs a queue from the existing 'pool'
  // or creates a new queue and adds it to the
  // 'pool' if none are available.
  void _initialize_thread_queue() {
    if ( _idx == NUM_QUEUES) {
      _thread_queue = new queue_t;
      rw_wrlock_t lock( _mutex);
      _thread_queues.push_back( _thread_queue);
    } else {
      rw_rdlock_t lock( _mutex);
      _thread_queue = _thread_queues.at( _idx++);
    }
  }

  // Initialize should be called before any Counter/Samplers are created/destroyed
  // so that thread queues can be initially put into the 'pool'
  // starts up a thread that reads from the vector of thread queues and places
  // the results onto an internal queue
  void init() {
    for ( int i = 0; i < NUM_QUEUES; i++) {
      rw_wrlock_t lock( _mutex);
      _thread_queues.push_back( new queue_t);
    }

    _keep_running = true;
    _future = std::async( std::launch::async, [ this ]() {
      std::uint32_t count = 0;

      while ( _keep_running.load( std::memory_order_relaxed)) {
        std::this_thread::sleep_for( std::chrono::microseconds( 100));
        T t;

        rw_rdlock_t lock( _mutex);
        for ( queue_ptr_t thread_queue : _thread_queues) {
          lock.unlock(); // don't need to lock access to the individual queue object

          while ( thread_queue->pop( t)) {
            _queue->push( t);
            count++;
          }

          lock.lock();
        }
      }

      std::cout << "Count: " << count << std::endl;
    });
  }

private:
  static thread_local queue_ptr_t _thread_queue;

  std::atomic<bool>        _keep_running; // threading
  std::future<void>        _future;

  rw_mutex_t               _mutex; // This should be reader writer mutex used
                                   // to protect access to the vector of thread queues
                                   // so that threads can dynamically add/remove
                                   // themselves if needed

  std::uint16_t            _idx = 0;
  std::vector<queue_ptr_t> _thread_queues;
  queue_ptr_t              _queue;
};

template <typename T>
thread_local typename Queue<T>::queue_ptr_t Queue<T>::_thread_queue = nullptr;

#endif // __QUEUE_H__
