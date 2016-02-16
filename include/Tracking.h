#ifndef __COUNTER_H__
#define __COUNTER_H__

#include <ostream>
#include "Queue.h"
#include "DataTypes.h"

class Counter
{
  friend std::ostream& operator<<( std::ostream& out, Counter const& counter);
public:
  Counter( std::uint16_t scope, Token token);
  ~Counter();
  static Queue<Delta>* _queue;

private:
  // required for __rdtscp call
  static thread_local std::uint32_t _aux;

  Delta _delta;
};

class PreSampler
{
  friend std::ostream& operator<<( std::ostream& out, PreSampler const& preSampler);
public:
  PreSampler( std::uint16_t scope, Token token);
  ~PreSampler();
  static Queue<Sample>* _queue;

private:
  // required for __rdtscp call
  static thread_local std::uint32_t _aux;
  Sample _sample;
};

class PostSampler
{
  friend std::ostream& operator<<( std::ostream& out, PostSampler const& postSampler);
public:
  PostSampler( std::uint16_t scope, Token token);
  ~PostSampler();
  static Queue<Sample>* _queue;

private:
  // required for __rdtscp call
  static thread_local std::uint32_t _aux;
  Sample _sample;
};

std::ostream& operator<<( std::ostream& out, Counter const& counter);
std::ostream& operator<<( std::ostream& out, PreSampler const& preSampler);
std::ostream& operator<<( std::ostream& out, PostSampler const& postSampler);

#endif // __COUNTER_H__
