#include <iostream>
#include <thread>
#include <chrono>
#include <sstream>
#include <x86intrin.h>
#include "SpscQueue.h"

std::uint32_t MAX_COUNT = 10000000;

void r( spsc_queue_t<std::uint32_t>* queue1, spsc_queue_t<std::uint32_t>* queue2) {
  std::uint32_t count = 0;

  auto t_start = std::chrono::high_resolution_clock::now();
  while ( count != MAX_COUNT) {
    std::uint32_t value;
    while ( !queue1->pop( value)) {}
    queue2->push( value);
    count++;
  }
  auto t_end = std::chrono::high_resolution_clock::now();

  std::stringstream msg;
  msg << "Count: " << std::chrono::duration<double, std::milli>(t_end - t_start).count() << "\n";
  std::cout << msg.str();

}

int main() {
  spsc_queue_t<std::uint32_t> queue1;
  spsc_queue_t<std::uint32_t> queue2;

  for ( int i = 0; i < 1; i++) {
    queue1.push( 5);
  }

  std::thread ping( r, &queue1, &queue2);
  std::thread pong( r, &queue2, &queue1);

  ping.join();
  pong.join();

  return 0;
}
