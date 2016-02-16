# Counters with thread local storage

Uses the idea of queues and RAII counters/samplers. Adds in thread local storage where each thread writes to its own thread local queue. A pointer to that local queue is placed into a vector which can then be read from and the entries can be placed into a more central queue and can then be sent somewhere else.

The thread local storage does require some initialization on the first destruction of a counter/sampler. The QueuePool was created to attempt to decrease the time this initialization takes by having an initial set of queues that can be pulled from as opposed to requiring object creation. Currently there is not a way to give a queue back to the pool, need to set up a free list for indexes in the vector or some other type of implementation.

SpscQueue is meant to be specialized for any queue you want to put into it. There is an alias named spsc_queue_t that can be changed so that multiple classes can see the same implementation of an spsc_queue.

I compiled it with gcc 5, so there are some incompatibilities with gcc 4.7, but they are not unsolvable.

Some sample results I got on my little Ubuntu machine at home:

`folly::ProducerConsumerQueue`
<table><tbody>
<tr><th>Thread</th><th>Sum</th><th>Count</th><th>Avg</th><th>Min</th><th>Max</th><th>Percentiles</th><th>Outliers</th></tr>
<tr><td>140566333323008</td><td>10088504576</td><td>100000000</td><td>100</td><td>90</td><td>36908</td><td><ul><li>80%ile - 104</li><li>90%ile - 111</li><li>99%ile - 162</li><li>99.9%ile - 245</li><li>99.99%ile - 1878</li><li>99.999%ile - 5549</li><li>99.9999%ile - 13870</li></ul></td><td><ul><li>22586</li><li>22780</li><li>23088</li><li>23382</li><li>24736</li><li>24760</li><li>24854</li><li>26338</li><li>31624</li><li>36908</li></ul></td></tr>
<tr><td>140566324930304</td><td>12171954368</td><td>100000000</td><td>121</td><td>100</td><td>60680</td><td><ul><li>80%ile - 120</li><li>90%ile - 126</li><li>99%ile - 228</li><li>99.9%ile - 296</li><li>99.99%ile - 3304</li><li>99.999%ile - 7154</li><li>99.9999%ile - 15792</li></ul></td><td><ul><li>31550</li><li>34082</li><li>34244</li><li>34380</li><li>37208</li><li>43252</li><li>44632</li><li>56020</li><li>56448</li><li>60680</li></ul></td></tr>
</tbody></table>

`boost::lockfree::spsc_queue`
<table><tbody>
<tr><th>Thread</th><th>Sum</th><th>Count</th><th>Avg</th><th>Min</th><th>Max</th><th>Percentiles</th><th>Outliers</th></tr>
<tr><td>140448964605696</td><td>9999685180</td><td>100000000</td><td>99</td><td>86</td><td>72630</td><td><ul><li>80%ile - 104</li><li>90%ile - 108</li><li>99%ile - 162</li><li>99.9%ile - 224</li><li>99.99%ile - 1823</li><li>99.999%ile - 5950</li><li>99.9999%ile - 13208</li></ul></td><td><ul><li>28092</li><li>28548</li><li>28812</li><li>29968</li><li>30770</li><li>50470</li><li>57084</li><li>57548</li><li>62300</li><li>72630</li></ul></td></tr>
<tr><td>140448956212992</td><td>11892717872</td><td>100000000</td><td>118</td><td>100</td><td>63100</td><td><ul><li>80%ile - 120</li><li>90%ile - 120</li><li>99%ile - 202</li><li>99.9%ile - 265</li><li>99.99%ile - 3203</li><li>99.999%ile - 7289</li><li>99.9999%ile - 15931</li></ul></td><td><ul><li>28288</li><li>30108</li><li>30332</li><li>35018</li><li>35384</li><li>36324</li><li>38060</li><li>38444</li><li>47112</li><li>63100</li></ul></td></tr>
</tbody></table>

`moodycamel::ReaderWriterQueue`
<table><tbody>
<tr><th>Thread</th><th>Sum</th><th>Count</th><th>Avg</th><th>Min</th><th>Max</th><th>Percentiles</th><th>Outliers</th></tr>
<tr><td>140431091955456</td><td>10259010687</td><td>100000000</td><td>102</td><td>88</td><td>35150</td><td><ul><li>80%ile - 108</li><li>90%ile - 108</li><li>99%ile - 125</li><li>99.9%ile - 209</li><li>99.99%ile - 362</li><li>99.999%ile - 4874</li><li>99.9999%ile - 14328</li></ul></td><td><ul><li>19646</li><li>20112</li><li>20148</li><li>20770</li><li>21924</li><li>25674</li><li>27034</li><li>29922</li><li>35142</li><li>35150</li></ul></td></tr>
<tr><td>140431083562752</td><td>11880821453</td><td>100000000</td><td>118</td><td>102</td><td>37172</td><td><ul><li>80%ile - 120</li><li>90%ile - 121</li><li>99%ile - 149</li><li>99.9%ile - 231</li><li>99.99%ile - 725</li><li>99.999%ile - 5794</li><li>99.9999%ile - 13923</li></ul></td><td><ul><li>19944</li><li>21898</li><li>22084</li><li>23460</li><li>23990</li><li>28504</li><li>30546</li><li>31556</li><li>31758</li><li>37172</li></ul></td></tr>
</tbody></table>
