# Overview

The publishing side of the test writes data as fast as it can. Every few samples (configured through the command line), it sends a special sample requesting an echo from the subscribing side. It uses this `request -> echo` exchange to measure round-trip latency.

![PerfTest Overview Diagram](img/PerfTest_Overview_Diagram.png)

As you will see in Section 8, there are several command-line options, including ones to designate whether the application will act as the publisher or subscriber.

You will start multiple copies of the application (typically 1 publisher and 1 or more subscribers):

- The publishing application publishes throughput data; it also subscribes to the latency echoes.
- The subscribing applications subscribe to the throughput data, in which the echo requests are embedded; they also publish the latency echoes.

The publisher prints the latency test results meanwhile the subscriber prints the throughput results.
