# Counters with thread local storage

Uses the idea of RAII counters/samplers and adds thread local storage. Every counter/sampler points to a static `Queue` object (just like the current implementation (sort of)). The difference is that that `Queue` internally contains a thread local queue, which gets initialized on the first produce call inside each thread. The thread local queues are placed onto a vector which can referenced by another thread for reading out and placing onto a unified queue which can then be published or whatever.

The thread local storage initialization does take some time during the first produce on a counter/sampler. So some initialization is done during the base `Queue` creation (which needs to be assigned externally anyways) in order to decrease the time this initialization takes. An initial set of queues are created and then handed out to a thread when needed.

Currently there is not a way to give a queue back to the pool, need to set up a free list for indexes in the vector or some other type of implementation, should probably be a map of queues or some such.

`SpscQueue` is meant to be specialized for any queue you want to put into it. There is an alias named spsc_queue_t that can be changed so that multiple classes can use the same implementation of a single producer single consumer queue.

I compiled it with gcc 5, so there are some incompatibilities with gcc 4.7, but they are not unsolvable.

Hopefully can do some testing on the work boxes, but in the meantime some sample results I got on my little Ubuntu machine at home:

*It should be noted that these times are from rdtscp calls wrapped around the creation/destruction of the defined type, not the times that result from the produce calls inside the objects*

`folly::ProducerConsumerQueue`
<table>
  <tbody>
    <tr>
      <th>Type</th>
      <th>Count</th>
      <th>Avg</th>
      <th>Min</th>
      <th>Max</th>
      <th>Percentiles</th>
      <th>Outliers</th>
    </tr>
    <tr>
      <td>Counter</td>
      <td>1000000</td>
      <td>186</td>
      <td>72</td>
      <td>143240</td>
      <td>
        <ul>
          <li>80%ile - 226</li><li>95%ile - 349</li><li>99.99%ile - 12732</li>
        </ul>
      </td>
      <td>
        <ul>
          <li>112781</li><li>135008</li><li>143240</li>
        </ul>
      </td>
    </tr>
    <tr>
      <td>PreSampler</td>
      <td>1000000</td>
      <td>149</td>
      <td>44</td>
      <td>23752</td>
      <td>
        <ul>
          <li>80%ile - 192</li><li>95%ile - 296</li><li>99.99%ile - 2776</li>
        </ul>
      </td>
      <td>
        <ul>
          <li>21056</li><li>21367</li><li>23752</li>
        </ul>
      </td>
    </tr>
    <tr>
      <td>PostSampler</td>
      <td>1000000</td>
      <td>156</td>
      <td>42</td>
      <td>30942</td>
      <td>
        <ul>
          <li>80%ile - 200</li><li>95%ile - 304</li><li>99.99%ile - 2900</li>
        </ul>
      </td>
      <td>
        <ul>
          <li>22164</li><li>23530</li><li>30942</li>
        </ul>
      </td>
    </tr>
  </tbody>
</table>

`boost::lockfree::spsc_queue`
<table>
  <tbody>
    <tr>
      <th>Type</th>
      <th>Count</th>
      <th>Avg</th>
      <th>Min</th>
      <th>Max</th>
      <th>Percentiles</th>
      <th>Outliers</th>
    </tr>
    <tr>
      <td>Counter</td>
      <td>1000000</td>
      <td>171</td>
      <td>66</td>
      <td>138316</td>
      <td>
        <ul>
          <li>80%ile - 196</li><li>95%ile - 290</li><li>99.99%ile - 2932</li>
        </ul>
      </td>
      <td>
        <ul>
          <li>121004</li><li>123996</li><li>138316</li>
        </ul>
      </td>
    </tr>
    <tr>
      <td>PreSampler</td>
      <td>1000000</td>
      <td>149</td>
      <td>44</td>
      <td>33859</td>
      <td>
        <ul>
          <li>80%ile - 196</li><li>95%ile - 280</li><li>99.99%ile - 3908</li>
        </ul>
      </td>
      <td>
        <ul>
          <li>26953</li><li>29852</li><li>33859</li>
        </ul>
      </td>
    </tr>
    <tr>
      <td>PostSampler</td>
      <td>1000000</td>
      <td>157</td>
      <td>42</td>
      <td>32406</td>
      <td>
        <ul>
          <li>80%ile - 192</li><li>95%ile - 280</li><li>99.99%ile - 1598</li>
        </ul>
      </td>
      <td>
        <ul>
          <li>19353</li><li>22096</li><li>32406</li>
        </ul>
      </td>
    </tr>
  </tbody>
</table>

`moodycamel::ReaderWriterQueue`
<table>
  <tbody>
    <tr>
      <th>Type</th>
      <th>Count</th>
      <th>Avg</th>
      <th>Min</th>
      <th>Max</th>
      <th>Percentiles</th>
      <th>Outliers</th>
    </tr>
    <tr>
      <td>Counter</td>
      <td>1000000</td>
      <td>178</td>
      <td>74</td>
      <td>22396</td>
      <td>
        <ul>
          <li>80%ile - 216</li><li>95%ile - 302</li><li>99.99%ile - 3904</li>
        </ul>
      </td>
      <td>
        <ul>
          <li>20424</li><li>20924</li><li>22396</li>
        </ul>
      </td>
    </tr>
    <tr>
      <td>PreSampler</td>
      <td>1000000</td>
      <td>157</td>
      <td>46</td>
      <td>20237</td>
      <td>
        <ul>
          <li>80%ile - 198</li><li>95%ile - 296</li><li>99.99%ile - 2508</li>
        </ul>
      </td>
      <td>
        <ul>
          <li>19648</li><li>19992</li><li>20237</li>
        </ul>
      </td>
    </tr>
    <tr>
      <td>PostSampler</td>
      <td>1000000</td>
      <td>171</td>
      <td>44</td>
      <td>210289</td>
      <td>
        <ul>
          <li>80%ile - 208</li><li>95%ile - 332</li><li>99.99%ile - 4682</li>
        </ul>
      </td>
      <td>
        <ul>
          <li>128684</li><li>136808</li><li>210289</li>
        </ul>
      </td>
    </tr>
  </tbody>
</table>
