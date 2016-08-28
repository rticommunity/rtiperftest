# Example Command-Lines for Running the Performance Test

The followings are examples of how to run the performance test for different use cases.

* The tests below print final results only; if you want to see intermediate values, remove the `-noprint` argument from the command line.

* If you are running on 2 unequal machines, i.e., one machine is faster (has better processors) than another, you will see better performance by running the Publisher on the slower machine.

* To measure CPU usage while running these tests, use "top" or a similar utility.

## 1-to-1, Multicast, Best Latency as a Function of Message Size

* Publisher:

```
bin/<arch>/Release/perftest_cpp -pub -noPrint -nic <ipaddr> -domain <ID> -latencyCount 1 -dataLen <length> -latencyTest -multicast -executionTime 100
```

* Subscriber:

```
bin/<arch>/Release/perftest_cpp -sub -noPrint -nic <ipaddr> -domain <ID> -multicast
```

Modify `-dataLen <bytes>` to see latencies for different data sizes.
Set `-executionTime <seconds>` to be >=100 for statistically better results.

## 1-to-1, Multicast, Maximum Throughput as a Function of Message Size (with Batching)

* Publisher:

```
bin/<arch>/Release/perftest_cpp -pub -noPrint -nic <ipaddr> -dataLen <length> -batchSize <bytes> -sendQueueSize <number> -multicast -executionTime 100
```

* Subscriber:

```
bin/<arch>/Release/perftest_cpp -sub -noprint -nic <ipaddr> -multicast
```

To achieve maximum throughput, start by setting See `-batchSize <bytes>` to `6400`, then increase the size to see if you get better throughput.

The largest valid batch size is `63000 bytes`.

For maximum throughput, start by setting `-sendQueueSize <number>` to `30`; the best value will usually be between `30-50`.

Note: For larger data sizes (`8000 bytes` and higher), batching often does not improve throughput, at least for 1-Gig networks.


## 1-to-1, Multicast, Latency vs. Throughput for 200-byte Messages (with Batching)

* Publisher:

```
bin/<arch>/Release/perftest_cpp -pub -noPrint -nic <ipaddr> -dataLen 200 -batchSize <bytes> -sendQueueSize <number> -spin <count> -multicast -executionTime 100
```

* Subscriber

```
bin/<arch>/Release/perftest_cpp -sub -noPrint -nic <ipaddr> -multicast
```

To adjust throughput, experiment with the value of `-spin <count>`. For example, to get a rate of 10,000 messages/sec, use `-spin 20000` to see the resulting rate, then adjust up or down as needed.


## 1-to-1, Multicast, Reliable UDPv4, All Sizes

* Publisher:

```
bin/<arch>/Release/perftest_cpp -pub -noPrint -sendQueueSize 32 -latencyCount 10000 -scan -multicast
```

* Subscriber:

```
bin/<arch>/Release/perftest_cpp -sub -noPrint -multicast
```

## 1-to-1, Unicast, Best-Effort, UDPv4, 1 Size

* Publisher:

```
bin/<arch>/Release/perftest_cpp -pub -noPrint -sendQueueSize 32  -latencyCount 1000 -dataLen 1024 -bestEffort -executionTime 100

```

* Subscriber

```
bin/<arch>/Release/perftest_cpp -sub -noPrint -dataLen 1024 -bestEffort
```

## 1-to-1, Multicast, Reliable, UDPv4, Batching Enabled

* Publisher:

```
bin/<arch>/Release/perftest_cpp -pub -noPrint -sendQueueSize 32 -latencyCount 1000 -dataLen 200 -batchSize 6400  -multicast -executionTime 100

```

* Subscriber:

```
bin/<arch>/Release/perftest_cpp -sub -noPrint -dataLen 200 -batchSize 6400 -multicast
```

## 1-to-2, Multicast, Reliable, UDPv4

* Publisher:

```
bin/<arch>/Release/perftest_cpp -pub -noPrint -pidMultiPubTest 0 -sendQueueSize 32 -numSubscribers 2 -latencyCount 1000 -dataLen 200 -multicast -executionTime 100

```

* Subscriber 1:

```
bin/<arch>/Release/perftest_cpp -sub -noPrint -dataLen 200 -batchSize 6400 -multicast
```

* Subscriber 2:

```
bin/<arch>/Release/perftest_cpp -sub -noPrint -dataLen 200 -batchSize 6400 -multicast
```

## 2-to-1, Multicast, Reliable, UDPv4

* Publisher 1:

```
bin/<arch>/Release/perftest_cpp -pub -noPrint -pidMultiPubTest 0 -sendQueueSize 32 -numSubscribers 1 -latencyCount 1000 -dataLen 200 -multicast -executionTime 100

```

* Publisher 2:

```
bin/<arch>/Release/perftest_cpp -pub -noPrint -pidMultiPubTest 1 -sendQueueSize 32 -numSubscribers 1 -latencyCount 1000 -dataLen 200 -multicast -executionTime 100
```

* Subscriber:

```
bin/<arch>/Release/perftest_cpp -sub -noPrint -dataLen 200 -numPublishers 2 -sidMultiSubTest 0 -multicast
```

## 1-to-1, Unicast, Reliable, UDPv4, Using Security: Signing Packages, Encrypting Data

* Publisher:

```
bin/<arch>/Release/perftest_cpp -pub -noPrint -dataLen 63000 -secureSign -secureEncryptData -executionTime 100
```

* Subscriber

```
bin/<arch>/Release/perftest_cpp -sub -noPrint -dataLen 63000 -secureSign -secureEncryptData
```