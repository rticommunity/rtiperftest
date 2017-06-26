# Using RTI Perftest with RTI Routing-Service

This wrapper has been created to test the effects in latency and Throughput introduced by *RTI Routing Service* when using *RTI Perftest*. It consists on a set of 2 files:

- A compatible configuration xml file for *RTI Routing Service* parameterized to use different environment variables depending on the scenario to test.
- A wrapper script to launch *RTI Routing Service* which will set the environment variables needed by the configuration xml file previously mentioned. It contains several Command-Line Parameters to control the scenario to be tested.

## Command-Line Parameters

-   `-domain <ID>`

    Domain ID.

    *RTI Routing Service* will route between the provided domain (ID) and (ID + 1).

    **Default:** `0`  
    **Range:** `0 - 200`

-   `-sendQueueSize <number>`

    Size of the send queue for the Writers used in *RTI Routing Service*

    **Default:** `50`  
    **Range:** `[1-100 million]`

-   `-bestEffort`

    Use best-effort reliability settings.

    **Default:** `false` (use reliable communication).

-   `-asynchronous`

    Enable asynchronous publishing in the DataWriter QoS.

    **Default:** `Not set`

-   `-unbounded`

    Use *Unbounded Sequences* and Large samples.

    **Default:** `Not set`

-   `-verbosity`

    Specify the verbosity level for *RTI Routing Service*

    `0` - `SILENT`  
    `1` - `ERROR` (default)
    `2` - `WARNING`  
    `3` - `ALL`

-   `-keyed`

    Specify the use of a keyed type.

    **Default:** `Unkeyed` type.

-   `-batchSize <bytes>`

    Enable batching and set the maximum batched message size.

    **Default:** `0` (batching disabled)  
    **Range:** `1 to 63000`

-   `-executionTime <sec>`

    Limit the test duration by specifying the number of seconds to keep *RTI Routing Service* running.

    **Default:** Not set, infinite.

-   `-nddshome`

    Path to the *RTI Connext DDS* installation. If this parameter is not present, the `$NDDSHOME` variable will be used.

## Example Command-Lines for Running the Performance Test

The followings are examples of how to run the performance test for different use cases.

### Minimum Latency -- 1 *Routing Service*

* *RTI Perftest* Publisher:

```
bin/<arch>/release/perftest_cpp -pub -noPrint -nic <ipaddr> -domain <ID> -latencyCount 1 -dataLen <length> -latencyTest -executionTime 100
```

* *RTI Routing Service* wrapper script:

```
resource/routing_service/routingservice_wrapper.sh -domain <ID> -executionTime 120
```

* *RTI Perftest* Subscriber:

```
bin/<arch>/release/perftest_cpp -sub -noPrint -nic <ipaddr> -domain <ID+1> -dataLen <length>
```

### Maximum Throughput -- 1 *Routing Service*

* *RTI Perftest* Publisher:

```
bin/<arch>/release/perftest_cpp -pub -noPrint -nic <ipaddr> -domain <ID> -batchSize <bytes> -sendQueueSize <number> -executionTime 100 -dataLen <length>
```

* *RTI Routing Service* wrapper script:

```
resource/routing_service/routingservice_wrapper.sh -domain <ID> -executionTime 120 -batchSize <bytes> -sendQueueSize <number>
```

* *RTI Perftest* Subscriber:

```
bin/<arch>/release/perftest_cpp -sub -noPrint -nic <ipaddr> -domain <ID+1> -dataLen <length>
```

### Maximum Throughput -- 2 *Routing Service*

* *RTI Perftest* Publisher:

```
bin/<arch>/release/perftest_cpp -pub -noPrint -nic <ipaddr> -domain <ID> -batchSize <bytes> -sendQueueSize <number> -executionTime 100 -dataLen <length>
```

* *RTI Routing Service 1* wrapper script:

```
resource/routing_service/routingservice_wrapper.sh -domain <ID> -executionTime 120 -batchSize <bytes> -sendQueueSize <number>
```

* *RTI Routing Service 2* wrapper script:

```
resource/routing_service/routingservice_wrapper.sh -domain <ID+1> -executionTime 120 -batchSize <bytes> -sendQueueSize <number>
```

* *RTI Perftest* Subscriber:

```
bin/<arch>/release/perftest_cpp -sub -noPrint -nic <ipaddr> -domain <ID+2> -dataLen <length>
```