# Output Example

The following is an example of the expected output from the performance test.

* Publisher:

```
> perftest_cpp -pub -noPrint -domain 27 -sendQueueSize 50 -latencyCount 10000 -scan Waiting to discover 1 subscribers... 
Waiting for subscribers announcement ...
Publishing data...
Length: 32 Latency: Ave 396 us Std 48.9 us Min 83 us Max 538 us 50% 401 us 90% 459 us 99% 510 us 99.99% 538 us 99.9999% 538 us
Length: 64 Latency: Ave 399 us Std 53.1 us Min 88 us Max 1062 us 50% 403 us 90% 461 us 99% 537 us 99.99% 1062 us 99.9999% 1062 us
...
```

* Subscriber

```
> bin/i86Linux2.6gcc3.4.3/release/perftest_cpp -sub -noPrint -domain 27
Waiting to discover 1 publishers ...
Waiting for data...
Length: 32 Packets: 10000000 Packets/s(ave): 47913 Mbps(ave): 12.3 Lost: 0
Length: 64 Packets: 10000000 Packets/s(ave): 47580 Mbps(ave): 24.4 Lost: 0
...
```