#ifndef __LOCK_FREE_QUEUE_H__
#define __LOCK_FREE_QUEUE_H__

#include <list>

template <typename T>
struct LockFreeQueue
{
  LockFreeQueue();
  void produce( T const& t);
  bool consume( T& t);

private:
  typedef std::list<T> TList;
  typename TList::iterator iHead, iTail;

  TList list;
};

template <typename T>
LockFreeQueue<T>::LockFreeQueue() {
  list.push_back( T());
  iHead = list.begin();
  iTail = list.end();
}

template <typename T>
void
LockFreeQueue<T>::produce( T const& t) {
  list.push_back( t);
  iTail = list.end();
  list.erase( list.begin(), iHead);
}

template <typename T>
bool
LockFreeQueue<T>::consume( T& t) {
  typename TList::iterator iNext = iHead;
  ++iNext;
  if ( iNext != iTail) {
    iHead = iNext;
    t = *iHead;
    return true;
  }
  return false;
}

#endif // __LOCK_FREE_QUEUE_H__
