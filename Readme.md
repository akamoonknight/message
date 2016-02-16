# Counters with thread local storage

Uses the idea of queues and RAII counters/samplers. Adds in thread local storage where each thread writes to its own thread local queue. That local queue is placed into a vector of thread local queues that are read from in a different thread, and are then placed into a separate queue from which they can be read at leisure (might be bounded queue though depending on the underlying type, so they should be read from at some point).

The thread local storage does require some initialization on the first destruction of a counter/sampler. The QueuePool was created to attempt to decrease this time by having an initial set of queues that can be pulled from. Currently there is not a way to give a queue back to the pool, need to set up a free list for indexes in the vector or some other type of implementation.

SpscQueue is meant to be specialized for any queue you want to put into it. There is an alias named spsc_queue_t that can be changed so that multiple classes can see the same implementation of an spsc_queue.
