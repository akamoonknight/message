#ifndef __QUEUE_H__
#define __QUEUE_H__

#include <vector>
#include <mutex>
#include <future>
#include <iostream>
#include "QueuePool.h"
#include "SpscQueue.h"

template <typename T>
class Queue
{
public:
  using rw_mutex_t  = std::mutex;
  using rw_wrlock_t = std::lock_guard<rw_mutex_t>;
  using rw_rdlock_t = std::unique_lock<rw_mutex_t>;
  using queue_t     = spsc_queue_t<T>;
  using queue_ptr_t = queue_t*;

  static constexpr std::uint16_t NUM_QUEUES = 4;

  Queue()
    : _keep_running( false)
    , _queue( new queue_t)
  {}

  ~Queue() {
    _keep_running = false;
    if ( _future.valid()) {
      _future.get();
    }
    QueuePool<T>::give( _thread_queue);
  }

  std::string name() {
    return _queue->name();
  }

  bool produce( T const& t) {
    if ( _thread_queue == nullptr) {
      _initialize_thread_queue();
    }

    _thread_queue->push( t);
  }

  void _initialize_thread_queue() {
    if ( _idx == NUM_QUEUES) {
      QueuePool<T>::get( &_thread_queue);

      rw_wrlock_t lock( _mutex);
      _thread_queues.push_back( _thread_queue);
    } else {
      rw_rdlock_t lock( _mutex);
      _thread_queue = _thread_queues.at( _idx++);
    }
  }

  void init() {
    for ( int i = 0; i < NUM_QUEUES; i++) {
      rw_wrlock_t lock( _mutex);
      _thread_queues.push_back( new queue_t);
    }

    _keep_running = true;
    _future = std::async( std::launch::async, [ this ]() {
      while ( _keep_running.load( std::memory_order_relaxed)) {
        std::this_thread::sleep_for( std::chrono::milliseconds( 1));
        T t;

        rw_rdlock_t lock( _mutex);
        for ( queue_ptr_t thread_queue : _thread_queues) {
          lock.unlock();
          while ( thread_queue->pop( t)) {
            if ( t.scope % 1000 == 1) {
              //std::cout << t << std::endl;
              _queue->push( t);
            }
          }
          lock.lock();
        }
      }
    });
  }

private:
  static thread_local queue_ptr_t _thread_queue;

  std::atomic<bool>        _keep_running;
  std::future<void>        _future;
  rw_mutex_t               _mutex;
  std::uint16_t            _idx = 0;
  std::vector<queue_ptr_t> _thread_queues;
  queue_ptr_t              _queue;
};

template <typename T>
thread_local typename Queue<T>::queue_ptr_t Queue<T>::_thread_queue = nullptr;

#endif // __QUEUE_H__
