#include <x86intrin.h>
#include "Tracking.h"

// Counter Declarations
// =====================

thread_local std::uint32_t Counter::_aux = 0;
Queue<Delta>* Counter::_queue = nullptr;

Counter::Counter( std::uint16_t scope, Token token)
  : _delta( scope, token, __rdtscp( &_aux))
{}

Counter::~Counter() {
  _delta.count = __rdtscp( &_aux) - _delta.count;
  _queue->produce( _delta);
}

std::ostream& operator<<( std::ostream& out, Counter const& counter) {
  return out << counter._delta;
}

// PreSampler Declarations
// ========================

thread_local std::uint32_t PreSampler::_aux = 0;
Queue<Sample>* PreSampler::_queue = nullptr;

PreSampler::PreSampler( std::uint16_t scope, Token token)
  : _sample( scope, token, __rdtscp( &_aux))
{
  _queue->produce( _sample);
}

PreSampler::~PreSampler() {}

std::ostream& operator<<( std::ostream& out, PreSampler const& preSampler) {
  return out << preSampler._sample;
}

// PostSampler Declarations
// ========================

thread_local std::uint32_t PostSampler::_aux = 0;
Queue<Sample>* PostSampler::_queue = nullptr;

PostSampler::PostSampler( std::uint16_t scope, Token token)
  : _sample( scope, token, 0)
{}

PostSampler::~PostSampler() {
  _sample.sample = __rdtscp( &_aux);
  _queue->produce( _sample);
}

std::ostream& operator<<( std::ostream& out, PostSampler const& postSampler) {
  return out << postSampler._sample;
}

