# Counters with thread local storage

Uses the idea of RAII counters/samplers and adds thread local storage. Every counter/sampler points to a static `Queue` object (just like the current implementation (sort of)). The difference is that that `Queue` internally contains a thread local queue, which gets initialized on the first produce call inside each thread. The thread local queues are placed onto a vector which can referenced by another thread for reading out and placing onto a unified queue which can then be published or whatever.

The thread local storage initialization does take some time during the first produce on a counter/sampler. So some initialization is done during the base `Queue` creation (which needs to be assigned externally anyways) in order to decrease the time this initialization takes. An initial set of queues are created and then handed out to a thread when needed.

Currently there is not a way to give a queue back to the pool, need to set up a free list for indexes in the vector or some other type of implementation, should probably be a map of queues or some such.

`SpscQueue` is meant to be specialized for any queue you want to put into it. There is an alias named spsc_queue_t that can be changed so that multiple classes can use the same implementation of a single producer single consumer queue.

I compiled it with gcc 5, so there are some incompatibilities with gcc 4.7, but they are not unsolvable.

Hopefully can do some testing on the work boxes, but in the meantime some sample results I got on my little Ubuntu machine at home:

*it should be noted that these times are from rdtsc calls wrapped around the creation/destruction of the defined type, not the times that result from the produce calls inside the objects*

`folly::ProducerConsumerQueue`
<table><tbody>
<tr><th>Type</th><th>Thread</th><th>Sum</th><th>Count</th><th>Avg</th><th>Min</th><th>Max</th><th>Percentiles</th><th>Outliers</th></tr>
<tr><td>Counter</td><td>140454114981632</td><td>162773521</td><td>1000000</td><td>162</td><td>72</td><td>23744</td><td><ul><li>80%ile - 204</li><li>95%ile - 264</li><li>99.99%ile - 3616</li></ul></td><td><ul><li>22504</li><li>22876</li><li>23744</li></ul></td></tr>
<tr><td>PreSampler</td><td>140454106588928</td><td>130753455</td><td>1000000</td><td>130</td><td>44</td><td>44432</td><td><ul><li>80%ile - 176</li><li>95%ile - 248</li><li>99.99%ile - 1268</li></ul></td><td><ul><li>20648</li><li>22268</li><li>44432</li></ul></td></tr>
<tr><td>PostSampler</td><td>140454098196224</td><td>153415621</td><td>1000000</td><td>153</td><td>42</td><td>155617</td><td><ul><li>80%ile - 192</li><li>95%ile - 264</li><li>99.99%ile - 4928</li></ul></td><td><ul><li>138604</li><li>145396</li><li>155617</li></ul></td></tr>
</tbody></table>

`boost::lockfree::spsc_queue`
<table><tbody>
<tr><th>Type</th><th>Thread</th><th>Sum</th><th>Count</th><th>Avg</th><th>Min</th><th>Max</th><th>Percentiles</th><th>Outliers</th></tr>
<tr><td>Counter</td><td>140384091735808</td><td>177924179</td><td>1000000</td><td>177</td><td>68</td><td>223516</td><td><ul><li>80%ile - 208</li><li>95%ile - 308</li><li>99.99%ile - 3380</li></ul></td><td><ul><li>21204</li><li>23392</li><li>223516</li></ul></td></tr>
<tr><td>PreSampler</td><td>140384083343104</td><td>139148414</td><td>1000000</td><td>139</td><td>44</td><td>20864</td><td><ul><li>80%ile - 184</li><li>95%ile - 280</li><li>99.99%ile - 1908</li></ul></td><td><ul><li>20073</li><li>20732</li><li>20864</li></ul></td></tr>
<tr><td>PostSampler</td><td>140384074950400</td><td>157932558</td><td>1000000</td><td>157</td><td>44</td><td>219976</td><td><ul><li>80%ile - 192</li><li>95%ile - 284</li><li>99.99%ile - 2924</li></ul></td><td><ul><li>165176</li><li>176360</li><li>219976</li></ul></td></tr>
</tbody></table>

`moodycamel::ReaderWriterQueue`
<table><tbody>
<tr><th>Type</th><th>Thread</th><th>Sum</th><th>Count</th><th>Avg</th><th>Min</th><th>Max</th><th>Percentiles</th><th>Outliers</th></tr>
<tr><td>Counter</td><td>140006612326144</td><td>185428944</td><td>1000000</td><td>185</td><td>72</td><td>25756</td><td><ul><li>80%ile - 221</li><li>95%ile - 305</li><li>99.99%ile - 2732</li></ul></td><td><ul><li>24381</li><li>25612</li><li>25756</li></ul></td></tr>
<tr><td>PreSampler</td><td>140006603933440</td><td>147656822</td><td>1000000</td><td>147</td><td>44</td><td>108324</td><td><ul><li>80%ile - 192</li><li>95%ile - 285</li><li>99.99%ile - 2376</li></ul></td><td><ul><li>98916</li><li>105700</li><li>108324</li></ul></td></tr>
<tr><td>PostSampler</td><td>140006471497472</td><td>167983885</td><td>1000000</td><td>167</td><td>40</td><td>31476</td><td><ul><li>80%ile - 212</li><li>95%ile - 304</li><li>99.99%ile - 3087</li></ul></td><td><ul><li>19716</li><li>21840</li><li>31476</li></ul></td></tr>
