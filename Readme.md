# Counters with thread local storage

Uses the idea of RAII counters/samplers and adds thread local storage. Every counter/sampler points to a static `Queue` object (just like the current implementation (sort of)). The difference is that that `Queue` internally contains a thread local queue, which gets initialized on the first produce call inside each thread. The thread local queues are placed onto a vector which can referenced by another thread for reading out and placing onto a unified queue which can then be published or whatever.

The thread local storage initialization does take some time during the first produce on a counter/sampler. So some initialization is done during the base `Queue` creation (which needs to be assigned externally anyways) in order to decrease the time this initialization takes. An initial set of queues are created and then handed out to a thread when needed.

Currently there is not a way to give a queue back to the pool, need to set up a free list for indexes in the vector or some other type of implementation, should probably be a map of queues or some such.

`SpscQueue` is meant to be specialized for any queue you want to put into it. There is an alias named spsc_queue_t that can be changed so that multiple classes can use the same implementation of a single producer single consumer queue.

I compiled it with gcc 5, so there are some incompatibilities with gcc 4.7, but they are not unsolvable.

Hopefully can do some testing on the work boxes, but in the meantime some sample results I got on my little Ubuntu machine at home:

`folly::ProducerConsumerQueue`
<table><tbody>
<tr><th>Type</th><th>Thread</th><th>Sum</th><th>Count</th><th>Avg</th><th>Min</th><th>Max</th><th>Percentiles</th><th>Outliers</th></tr>
<tr><td>Counter</td><td>139812437526272</td><td>8123761454</td><td>100000000</td><td>81</td><td>64</td><td>1454444</td><td><ul><li>80%ile - 86</li><li>90%ile - 91</li><li>99%ile - 129</li><li>99.9%ile - 239</li><li>99.99%ile - 764</li><li>99.999%ile - 11917</li><li>99.9999%ile - 33314</li></ul></td><td><ul><li>183560</li><li>183980</li><li>188106</li><li>193380</li><li>248062</li><li>316680</li><li>353506</li><li>367812</li><li>630768</li><li>1454444</li></ul></td></tr>
<tr><td>PreSampler</td><td>139812429133568</td><td>6195509016</td><td>100000000</td><td>61</td><td>36</td><td>2689954</td><td><ul><li>80%ile - 64</li><li>90%ile - 67</li><li>99%ile - 111</li><li>99.9%ile - 222</li><li>99.99%ile - 365</li><li>99.999%ile - 14772</li><li>99.9999%ile - 62906</li></ul></td><td><ul><li>337442</li><li>345530</li><li>361248</li><li>418200</li><li>435726</li><li>444980</li><li>459782</li><li>1293082</li><li>2059992</li><li>2689954</li></ul></td></tr>
<tr><td>PostSampler</td><td>139812420740864</td><td>5380916317</td><td>100000000</td><td>53</td><td>36</td><td>3287586</td><td><ul><li>80%ile - 58</li><li>90%ile - 59</li><li>99%ile - 86</li><li>99.9%ile - 191</li><li>99.99%ile - 339</li><li>99.999%ile - 5503</li><li>99.9999%ile - 43588</li></ul></td><td><ul><li>203174</li><li>205648</li><li>212208</li><li>250484</li><li>267354</li><li>271834</li><li>322052</li><li>374044</li><li>407112</li><li>3287586</li></ul></td></tr>
</tbody></table>

`boost::lockfree::spsc_queue`
<table><tbody>
<tr><th>Type</th><th>Thread</th><th>Sum</th><th>Count</th><th>Avg</th><th>Min</th><th>Max</th><th>Percentiles</th><th>Outliers</th></tr>
<tr><td>Counter</td><td>139984924759808</td><td>8570366266</td><td>100000000</td><td>85</td><td>64</td><td>9011482</td><td><ul><li>80%ile - 90</li><li>90%ile - 92</li><li>99%ile - 124</li><li>99.9%ile - 259</li><li>99.99%ile - 3027</li><li>99.999%ile - 56502</li><li>99.9999%ile - 99872</li></ul></td><td><ul><li>784120</li><li>931966</li><li>1089418</li><li>1213738</li><li>2091080</li><li>2767980</li><li>2830566</li><li>4023876</li><li>5430712</li><li>9011482</li></ul></td></tr>
<tr><td>PreSampler</td><td>139984916367104</td><td>6065890089</td><td>100000000</td><td>60</td><td>36</td><td>158874</td><td><ul><li>80%ile - 65</li><li>90%ile - 68</li><li>99%ile - 106</li><li>99.9%ile - 224</li><li>99.99%ile - 1642</li><li>99.999%ile - 6279</li><li>99.9999%ile - 17858</li></ul></td><td><ul><li>48356</li><li>50054</li><li>100336</li><li>108990</li><li>112678</li><li>112758</li><li>116672</li><li>122362</li><li>144848</li><li>158874</li></ul></td></tr>
<tr><td>PostSampler</td><td>139984907974400</td><td>5504780194</td><td>100000000</td><td>55</td><td>36</td><td>323906</td><td><ul><li>80%ile - 60</li><li>90%ile - 62</li><li>99%ile - 107</li><li>99.9%ile - 229</li><li>99.99%ile - 350</li><li>99.999%ile - 3764</li><li>99.9999%ile - 23987</li></ul></td><td><ul><li>127270</li><li>127494</li><li>128402</li><li>137016</li><li>137110</li><li>139132</li><li>178356</li><li>238952</li><li>312898</li><li>323906</li></ul></td></tr>
</tbody></table>

`moodycamel::ReaderWriterQueue`
<table><tbody>
<tr><th>Type</th><th>Thread</th><th>Sum</th><th>Count</th><th>Avg</th><th>Min</th><th>Max</th><th>Percentiles</th><th>Outliers</th></tr>
<tr><td>Counter</td><td>140718820267776</td><td>8293238154</td><td>100000000</td><td>82</td><td>66</td><td>2670786</td><td><ul><li>80%ile - 93</li><li>90%ile - 98</li><li>99%ile - 107</li><li>99.9%ile - 246</li><li>99.99%ile - 1736</li><li>99.999%ile - 12850</li><li>99.9999%ile - 53818</li></ul></td><td><ul><li>305424</li><li>306414</li><li>321090</li><li>328388</li><li>328462</li><li>329298</li><li>365316</li><li>389220</li><li>1660890</li><li>2670786</li></ul></td></tr>
<tr><td>PreSampler</td><td>140718811875072</td><td>5892842516</td><td>100000000</td><td>58</td><td>36</td><td>3131998</td><td><ul><li>80%ile - 62</li><li>90%ile - 65</li><li>99%ile - 81</li><li>99.9%ile - 193</li><li>99.99%ile - 336</li><li>99.999%ile - 6016</li><li>99.9999%ile - 41956</li></ul></td><td><ul><li>294246</li><li>308584</li><li>317990</li><li>327830</li><li>815866</li><li>829358</li><li>836752</li><li>2463266</li><li>2812996</li><li>3131998</li></ul></td></tr>
<tr><td>PostSampler</td><td>140718803482368</td><td>6147223110</td><td>100000000</td><td>61</td><td>36</td><td>1455760</td><td><ul><li>80%ile - 66</li><li>90%ile - 69</li><li>99%ile - 84</li><li>99.9%ile - 210</li><li>99.99%ile - 434</li><li>99.999%ile - 10626</li><li>99.9999%ile - 44887</li></ul></td><td><ul><li>211192</li><li>213110</li><li>261226</li><li>264142</li><li>303868</li><li>308364</li><li>327856</li><li>329654</li><li>349246</li><li>1455760</li></ul></td></tr>
</tbody></table>
