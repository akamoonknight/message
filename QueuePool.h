#ifndef __QUEUE_POOL_H__
#define __QUEUE_POOL_H__

#include "SpscQueue.h"

template <typename T>
class QueuePool
{
public:
  static constexpr std::uint16_t NUM_QUEUES = 4;
  using rw_mutex_t  = std::mutex;
  using rw_wrlock_t = std::lock_guard<rw_mutex_t>;
  using rw_rdlock_t = std::unique_lock<rw_mutex_t>;
  using queue_t     = spsc_queue_t<T>;
  using queue_ptr_t = queue_t*;

  static void initialize() {
    for ( int i = 0; i < NUM_QUEUES; i++) {
      rw_wrlock_t lock( _mutex);
      _queues.push_back( new queue_t);
    }
  }

  static void get( queue_ptr_t* queue_ptr) {
    if ( _idx > NUM_QUEUES) {
      rw_wrlock_t lock( _mutex);
      _queues.push_back( new queue_t);
    } else {
      rw_rdlock_t lock( _mutex);
      *queue_ptr = _queues.at( _idx++);
    }
  }

  static void give( queue_ptr_t queue) {
  }

private:
  static rw_mutex_t               _mutex;
  static std::uint8_t             _idx;
  static std::vector<queue_ptr_t> _queues;
};

template <typename T>
typename QueuePool<T>::rw_mutex_t QueuePool<T>::_mutex;

template <typename T>
std::uint8_t QueuePool<T>::_idx = 0;

template <typename T>
std::vector<typename QueuePool<T>::queue_ptr_t> QueuePool<T>::_queues;

#endif // __QUEUE_POOL_H__
