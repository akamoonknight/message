# Counters with thread local storage

Uses the idea of queues and RAII counters/samplers. Adds in thread local storage where each thread writes to its own thread local queue. A pointer to that local queue is placed into a vector which can then be read from and the entries can be placed into a more central queue and can then be sent somewhere else.

The thread local storage does require some initialization on the first destruction of a counter/sampler. The QueuePool was created to attempt to decrease the time this initialization takes by having an initial set of queues that can be pulled from as opposed to requiring object creation. Currently there is not a way to give a queue back to the pool, need to set up a free list for indexes in the vector or some other type of implementation.

SpscQueue is meant to be specialized for any queue you want to put into it. There is an alias named spsc_queue_t that can be changed so that multiple classes can see the same implementation of an spsc_queue.

I compiled it with gcc 5, so there are some incompatibilities with gcc 4.7, but they are not unsolvable.

Some sample results I got on my little Ubuntu machine at home:

`folly::ProducerConsumerQueue`
<table><tbody>
<tr><th>Thread</th><th>Sum</th><th>Count</th><th>Avg</th><th>Min</th><th>Max</th><th>Percentiles</th><th>Outliers</th></tr>
<tr><td>139784881891072</td><td>165782</td><td>1000</td><td>165</td><td>100</td><td>26516</td><td><ul><li>80%ile - 116</li><li>90%ile - 118</li><li>99%ile - 4360</li><li>99.9%ile - 4360</li><li>99.99%ile - 4360</li><li>99.999%ile - 4360</li><li>99.9999%ile - 4360</li></ul></td><td><ul><li>1900</li><li>1906</li><li>1926</li><li>1932</li><li>1946</li><li>1978</li><li>1986</li><li>2076</li><li>3368</li><li>26516</li></ul></td></tr>
<tr><td>139784873498368</td><td>137032</td><td>1000</td><td>137</td><td>102</td><td>2064</td><td><ul><li>80%ile - 125</li><li>90%ile - 129</li><li>99%ile - 854</li><li>99.9%ile - 854</li><li>99.99%ile - 854</li><li>99.999%ile - 854</li><li>99.9999%ile - 854</li></ul></td><td><ul><li>154</li><li>218</li><li>622</li><li>1802</li><li>1848</li><li>1852</li><li>1858</li><li>1916</li><li>1940</li><li>2064</li></ul></td></tr>
</tbody></table>
