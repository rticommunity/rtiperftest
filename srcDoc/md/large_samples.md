# Modify *RTI Perftest* for Larger Samples

By default *RTI Perftest* can send samples up to 128 KB, in order to test for larger samples the following changes are needed:

## Changes in the idl:

Change the content of the `srcIdl/perftest.idl` source file so the maximum size of the sequence containing the data fits your desired size:

``` c
    const long MAX_BINDATA_SIZE = <NEW_SIZE>;
```

## Changes in the resource limits in the QoS profile

*RTI Perftest* is designed to allocate a big amount of samples in the reader queues, this helps increasing the performance when working with small sample sizes.

However, since you are increasing the size of the samples, now you could get into a position where the *RTI Connext DDS* libraries try and fail to allocate so much memory in the reader side.

This scenario is resolved by modifying the QoS profiles used by the *RTI Perftest* application. In order to do so, you will need to modify the following QoS profile in the `perftest_qos_profiles.xml` file:

``` xml
<qos_profile name="ThroughputQos" base_name="BaseProfileQos">
  <datareader_qos>
    <resource_limits>
      <max_samples>10000</max_samples>
      <initial_samples>5000</initial_samples>
      <max_samples_per_instance>10000</max_samples_per_instance>
    </resource_limits>
    
    <reader_resource_limits>
      <max_samples_per_remote_writer>10000</max_samples_per_remote_writer>
    </reader_resource_limits>
  </datareader_qos>
</qos_profile>
```

The exact values needed will depend on your specific sample size and the machine available resources. A good idea would be trying decreasing the existing sizes by a x100 factor.

## Changes in code

*RTI Perftest* performs some internal checkings to ensure that the sample size specified by command-line parameter is lower than the maximum allowed. Therefore some changes are required:

### Changes in C++

Modify the following line in the `srcCpp/MessagingIF.h` source file:

``` c++
    static const int MAX_DATA_SIZE = <NEW_SIZE>;
``` 

### Changes in C++03

Modify the following line in the `srcCpp03/MessagingIF.h` source file:

``` c++
    static const int MAX_DATA_SIZE = <NEW_SIZE>;
``` 

### Changes in Java

Modify the following line in the `srcJava/com/rti/perftest/TestMessage.java` source file:

``` java
    public static final int MAX_DATA_SIZE = <NEW_SIZE>;
``` 

### Changes in C# #


Modify the following line in the `srcCs/MessagingIF.cs` source file:

``` csharp
    public const int MAX_DATA_SIZE = <NEW_SIZE>;
``` 

## Changes in the Flow Controller

*RTI Perftest* will detect that the sample size is bigger than 63000 bytes, so it will set the DataWriters to do Asynchronous writing. This will imply the use of the Flow Controller defined in the `perftest_qos_profiles.xml` configuration file.

``` xml
<qos_profile name="BaseProfileQos">
  <participant_qos>
    <property>
      <value>
        <element>
          <name>dds.flow_controller.token_bucket.fast_flow.token_bucket.max_tokens</name>
          <value>30</value>
        </element>
        <element>
          <name>dds.flow_controller.token_bucket.fast_flow.token_bucket.tokens_added_per_period</name>
          <value>20</value>
        </element>
        <element>
          <name>dds.flow_controller.token_bucket.fast_flow.token_bucket.bytes_per_token</name>
          <value>65536</value>
        </element>
        <element>
          <name>dds.flow_controller.token_bucket.fast_flow.token_bucket.period.sec</name>
          <value>0</value>
        </element>
        <element>
          <name>dds.flow_controller.token_bucket.fast_flow.token_bucket.period.nanosec</name>
          <value>10000000</value>
        </element>
      </value>
    </property>
  </participant_qos>
</qos_profile>
```

Depending on your sample size, you may need to do further tuning to the Flow Controller values.