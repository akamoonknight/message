# Counters with thread local storage

Uses the idea of queues and RAII counters/samplers. Adds in thread local storage where each thread writes to its own thread local queue. A pointer to that local queue is placed into a vector which can then be read from and the entries can be placed into a more central queue and can then be sent somewhere else.

The thread local storage does require some initialization on the first destruction of a counter/sampler. The QueuePool was created to attempt to decrease the time this initialization takes by having an initial set of queues that can be pulled from as opposed to requiring object creation. Currently there is not a way to give a queue back to the pool, need to set up a free list for indexes in the vector or some other type of implementation.

SpscQueue is meant to be specialized for any queue you want to put into it. There is an alias named spsc_queue_t that can be changed so that multiple classes can see the same implementation of an spsc_queue.

I compiled it with gcc 5, so there are some incompatibilities with gcc 4.7, but they are not unsolvable.

Some sample results I got on my little Ubuntu machine at home:

Counter Queue:     folly::ProducerConsumerQueue
PreSampler Queue:  folly::ProducerConsumerQueue
PostSampler Queue: folly::ProducerConsumerQueue
| Thread | Sum | Count | Avg | Min | Max | Percentiles | Outliers |
| :---: | :---: | :---: | :---: | :---: | :---: | :---: | :---: |
| 140446856238848 | 210158 | 1000 | 210 | 153 | 4560 | <ul><li>80%ile - 178</li><li>90%ile - 183</li><li>99%ile - 2591</li><li>99.9%ile - 2591</li><li>99.99%ile - 2591</li><li>99.999%ile - 2591</li><li>99.9999%ile - 2591</li></ul> | <ul><li>2506</li><li>2509</li><li>2547</li><li>2579</li><li>2586</li><li>2599</li><li>2646</li><li>2755</li><li>3082</li><li>4560</li></ul> |
| 140446847846144 | 209678 | 1000 | 209 | 179 | 3405 | <ul><li>80%ile - 199</li><li>90%ile - 206</li><li>99%ile - 1270</li><li>99.9%ile - 1270</li><li>99.99%ile - 1270</li><li>99.999%ile - 1270</li><li>99.9999%ile - 1270</li></ul> | <ul><li>243</li><li>269</li><li>1127</li><li>2470</li><li>2525</li><li>2532</li><li>2553</li><li>2684</li><li>2739</li><li>3405</li></ul> |
