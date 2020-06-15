.. _section-custom-types:

============
Custom Types
============

*RTI Perftest* allows the user to test the performance of their own datatypes
defined via 'idl', instead of the one provided by default. The purpose of this
feature is to get a closer approximation to the performance behavior of the
user's final system. The main reason why using one type or another will result
in different results for latency and throughput is that serialization and
deserialization performance depends on the complexity of the datatype.

This feature is designed to work for regular data, DynamicData and FlatData type
definitions (required to test Connext DDS FlatData and/or Shared Memory ZeroCopy
features). This feature is only implemented for *Traditional C++*.

**Note 1:** *DynamicData* testing uses the Connext DDS *DynamicData* API to
serialize/deserialize/copy and otherwise manipulate a data structure instead
of compiled type support-generated functions from the user's type definition
file.  However, for Perftest to do this with a user's datatype, the user must
still provide their datatype definition in an *idl* file with which *DynamicData*
will use a compiled/generated function to access the typecode for the datatype.

**Note 2:** To test *Flat Data* serialization/deserialization of user data types
requires the user to define both a regular datatype and a *FlatData* variant.
See specific notes in the section **Custom Types + `FlatData`** below.

The simplest way to begin testing your own types is to begin with the working
example found in 'resource/examples_customType'. You can copy the examples and
modify the example `ìdl` with your own datatype definition for the various
testing scenarios, regular, *DynamicData*, *FlatData* (needed for *ZeroCopy*).

How does it work
~~~~~~~~~~~~~~~~

*RTI Perftest* default datatype resembles the following (with some variations depending on
the use of *Keyed data*, *Flat Data*, *Unbounded Types* or *Zero Copy*):

.. code-block:: c

    struct TestData_t {
        octet key[KEY_SIZE];
        long entity_id;
        unsigned long seq_num;
        long timestamp_sec;
        unsigned long timestamp_usec;
        long latency_ping;
        sequence<octet, MAX_BOUNDED_SEQ_SIZE> bin_data;
    };

However, in order to use an user-specified datatype, the struct has been modified as follows:

.. code-block:: c

    struct TestData_t {
        octet key[KEY_SIZE];
        long entity_id;
        unsigned long seq_num;
        long timestamp_sec;
        unsigned long timestamp_usec;
        long latency_ping;
        sequence<octet, MAX_BOUNDED_SEQ_SIZE> bin_data;
    #ifdef RTI_CUSTOM_TYPE
        RTI_CUSTOM_TYPE custom_type;
        // custom_type_size is the serialize size of the field custom_type
        // in the the sample that we send on the wire
        long custom_type_size;
    #endif
    };

Notice that a `RTI_CUSTOM_TYPE` member has been added to the struct (conditionally).
This macro is replaced at generation time with the type specified by the
user as the custom type.

Therefore if the customer provides an `.idl` file with the following structure:

.. code-block:: c

    struct Test {
        sequence<octet, MAX_SIZE> test_seq;
        long test_long; //@key
    };

Users must supply the name 'Test' to the perftest build process as the name of
their customType so that 'RTI_CUSTOM_TYPE' will be replaced by 'Test'.

Steps to use Custom Types
~~~~~~~~~~~~~~~~~~~~~~~~~

The steps needed to use a custom type in *RTI Perftest* are:

Copy the `idl` file(s) into ``~/rtiperftest/srcIdl/customTypes/``
-----------------------------------------------------------------

Copy the `idl` file(s) used to define the user's datatype to be tested into
``~/rtiperftest/srcIdl/customTypes/``.

Even though a datatype may require multiple `idl` files to fully define the type
and all of its contained members, perftest can only be compiled to test a single
user datatype at a time. You may build different images of perftest to test
different user datatypes by supplying a different customType name at build
invocation.

There are some restrictions for this step:

- You can include multiple `.idl` files, but all of them should be at the same
  level (no folders are allowed inside the `customTypes` folder).
- The `idl` files cannot be named `custom.idl`. A file with this name will be generated
  automatically by the `build` script, so this file would be overwritten.
- The customType that you want to test must be declared in the `.idl` file as
  ``struct <NameOfTheType> {``. In the declaration, there must be only a single
  space character between ``struct`` and the name of the datatype as well as
  between the name of the datatype and the ``{`` character. The build scripts
  will complain that they can't find your datatype definition in the `.idl`
  files if your datatype definition does not follow the required format.

Implement the API custom type functions of `customType.cxx`
-----------------------------------------------------------

You should find a file in ``${PERFTEST_HOME}/srcCpp/customType.cxx``. This file
contains several functions that may need to be implemented in order to correctly
initialize, finalize and populate the sample. Please see full discussion in the
section Full example using Custom Types.

**Note:** *RTI Perftest* will not initialize by default sequences, optional members
or non-primitive structures. This means that those fields will need to be
initialized by the user in the `set_custom_type_data`, `set_custom_type_dynamic_data`
and `set_custom_type_data_flatdata`.

**Note:** If using *FlatData* see the dedicated section below.

Run the `build` script adding ``--customType <NameOfTheType``
-------------------------------------------------------------

Run the ``build.sh`` or ``build.bat`` script using the ``--customType`` option.

**Note:** If using *FlatData* see the dedicated section below.

Run *RTI Perftest* as usual
---------------------------

A string at the beginning of the execution should indicate that you are using a
custom type and that this is not a regular execution.


Custom Types + `FlatData`
~~~~~~~~~~~~~~~~~~~~~~~~~

*FlatData* types are at this point in *RTI Perftest* associated to regular types.
If you want to use *FlatData* with for your Custom Types, follow these
additional steps:

- Create a copy of your original type (which should not be `FlatData`) in the same IDL file.
- Make this new type mutable (``@mutable``) and make it FlatData compatible (``@language_binding(FLAT_DATA)``).
- Implement the additional API custom type functions of customtype.cxx for *FlatData* types.
  This is a *must*, as the type is ``@mutable``, and every field is considered as optional
  and not sent into the wire if it is not set.
- Run the build script with the command-line parameter ``--customType <type>`` and ``--customTypeFlatData <flat_type>``.
- Run *RTI Perftest* as usual.

Full example using Custom Types
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The following types will be used for this example. The code can be found under
``resource/examples_customType/no_key_large``.

.. code-block:: c

    const long SIZE_TEST_SEQ = 100;
    const long SIZE_TEST_STRING = 128;

    enum TestEnum {
        ENUM1,
        ENUM2
    };//@Extensibility FINAL_EXTENSIBILITY

    struct StringTest {
        string<SIZE_TEST_STRING> test_string;
    };//@Extensibility FINAL_EXTENSIBILITY

    struct SeqTest {
        sequence<long, SIZE_TEST_SEQ> test_seq;
    };//@Extensibility FINAL_EXTENSIBILITY

    struct Test {
        long test_long;
        TestEnum test_enum;
        StringTest test_string;
        SeqTest test_seq;
    };//@Extensibility FINAL_EXTENSIBILITY

These functions in the example are spreaded across several `idl` files to show the
multiple files capability of the feature.

These are the steps needed to use the above type in *RTI Perftest* for the
C++ (Traditional) API:

1. Copy all the `idl` files into the `srcIdl/customType/` folder.

2. The following functions should be implemented (optionally) to properly
   initialize and set the custom type structures.

    - **initialize_custom_type_data**
    This function is used to initialize your data.
    Using this function, you will be able to allocate memory or set an immutable
    field of the data.
    The function takes one argument:

        - A reference to custom type data.

    .. code-block:: c

        bool initialize_custom_type_data(RTI_CUSTOM_TYPE &data)
        {
            bool success = true;
            if (!data.test_seq.test_seq.ensure_length(SIZE_TEST_SEQ, SIZE_TEST_SEQ)) {
                success = false;
            }
            data.test_enum = ENUM1;
            return success;
        }

    - **register_custom_type_data**
    This function is used to set your data before being registered. It is only
    required for key types. Set the key field of the data based on the key input.
    There is a one-to-one mapping between an input key
    and an instance.
    The function takes two arguments:

        - A reference to custom type data.
        - A specific number unique for every key.

    .. code-block:: c

        void register_custom_type_data(RTI_CUSTOM_TYPE & data, unsigned long key)
        {
            data.test_long = key;
        }

    - **set_custom_type_data**
    This function is used to set your data before it is sent.
    It is called every time the data is sent.
    You must set the custom type data before it is sent with the right
    "key" value and the "targetDataLen".
    The function takes three arguments:

        - A reference to custom type data.
        - A specific number unique for every key.
        - The target size set by the command-line parameter ``-dataLen <bytes>``
        minus the overhead of *RTI Perftest*. If applicable, you can use this
        value to set the content of the data.

    .. code-block:: c

        bool set_custom_type_data(
                RTI_CUSTOM_TYPE & data,
                unsigned long key,
                int targetDataLen)
        {
            bool success = true;
            data.test_long = key;
            if (sprintf(data.test_string.test_string, "Hello World! %lu", key) < 0) {
                success = false;
            }
            return success;
        }

    - **finalize_custom_type_data**:
    This function is used to remove your data. It is called in the destructor.
    The function takes one argument:

        - A reference to custom type data.

    .. code-block:: c

        bool finalize_custom_type_data(RTI_CUSTOM_TYPE & data)
        {
            bool success = true;
            if (!data.test_seq.test_seq.maximum(0)) {
                success = false;
            }
            return success;
        }

    - **initialize_custom_type_dynamic_data**:
    This function is used to initialize your DynamicData.
    Using this function, you will be able to allocate memory or set an immutable
    field of the data.
    The function takes one argument:

        - A reference to the full DDS_DynamicData object that includes custom_type.

    .. code-block:: c

        bool initialize_custom_type_dynamic_data(DDS_DynamicData & data)
        {
            bool success = true;
            if (!longSeq.ensure_length(SIZE_TEST_SEQ, SIZE_TEST_SEQ)) {
                success = false;
                fprintf(stderr, "longSeq.ensure_length failed.\n");
            }
            return success;
        }

    - **register_custom_type_dynamic_data**:
    This function is used to set your DynamicData before it has been registered.
    It is only required for key types.
    Set the key field of the data based on the key input.
    There is a one-to-one mapping between an input key and an instance.
    The function takes two arguments:

        - A reference to the full DDS_DynamicData object that includes custom_type.
        - A specific number unique for every key.

    .. code-block:: c

        void register_custom_type_dynamic_data(DDS_DynamicData & data, unsigned long key)
        {
            DDS_ReturnCode_t retcode = data.set_long(
                    "custom_type.test_long",
                    DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED,
                    key);
            if (retcode != DDS_RETCODE_OK) {
                fprintf(stderr, "set_long(test_long) failed: %d.\n", retcode);
            }
        }

    - **set_custom_type_dynamic_data**:
    This function is used to set your DynamicData before it is sent.
    It is called every time the data is sent.
    Set the custom type data before it is sent with the right "key"
    value and the "targetDataLen".
    The function takes three arguments:

        - A reference to the full DDS_DynamicData object that includes custom_type.
        - A specific number unique for every key.
        - The target size set by the command-line parameter ``-dataLen <bytes>``
        minus the overhead of *RTI Perftest*. If applicable, you can use this
        value to set the content of the data.

    .. code-block:: c

        bool set_custom_type_dynamic_data(
                DDS_DynamicData & data,
                unsigned long key,
                int targetDataLen)
        {
            DDS_ReturnCode_t retcode;
            char test_string[SIZE_TEST_STRING]; //size of member_name
            bool success = true;
            DDS_DynamicData customTypeData(NULL, DDS_DYNAMIC_DATA_PROPERTY_DEFAULT);
            DDS_DynamicData testSeqData(NULL, DDS_DYNAMIC_DATA_PROPERTY_DEFAULT);

            retcode = data.bind_complex_member(
                    customTypeData,
                    "custom_type",
                    DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED);
            if (retcode != DDS_RETCODE_OK) {
                fprintf(stderr,
                        "bind_complex_member(custom_type) failed: %d.\n",
                        retcode);
                success = false;
            }

            retcode = customTypeData.set_long(
                    "test_long",
                    DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED,
                    key);
            if (retcode != DDS_RETCODE_OK) {
                fprintf(stderr, "set_long(test_long) failed: %d.\n", retcode);
                success = false;
            }

            retcode = customTypeData.set_long(
                    "test_enum",
                    DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED,
                    ENUM1);
            if (retcode != DDS_RETCODE_OK) {
                fprintf(stderr, "set_long(test_enum) failed: %d.\n", retcode);
                success = false;
            }

            if (snprintf(test_string, SIZE_TEST_STRING, "Hello World! %lu", key) < 0) {
                success = false;
            }
            retcode = customTypeData.set_string(
                    "test_string.test_string",
                    DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED,
                    test_string);
            if (retcode != DDS_RETCODE_OK) {
                fprintf(stderr, "set_string(test_string) failed: %d.\n", retcode);
                success = false;
            }

            retcode = customTypeData.bind_complex_member(
                    testSeqData,
                    "test_seq",
                    DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED);
            if (retcode != DDS_RETCODE_OK) {
                fprintf(stderr,
                        "bind_complex_member(testSeqData) failed: %d.\n",
                        retcode);
                success = false;
            }
            retcode = testSeqData.set_long_seq(
                        "test_seq",
                        DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED,
                        longSeq);
            if (retcode != DDS_RETCODE_OK) {
                fprintf(stderr, "set_long(test_seq) failed: %d.\n", retcode);
                success = false;
            }
            retcode = customTypeData.unbind_complex_member(testSeqData);
            if (retcode != DDS_RETCODE_OK) {
                fprintf(stderr,
                        "unbind_complex_member(testSeqData) failed: %d.\n",
                        retcode);
                success = false;
            }
            retcode = data.unbind_complex_member(custom_type_data);
            if (retcode != DDS_RETCODE_OK) {
                fprintf(stderr,
                        "unbind_complex_member(custom_type) failed: %d.\n",
                        retcode);
                success = false;
            }
            return success;
        }

    - **finalize_custom_type_dynamic_data**:
    This function is used to remove your data. It is called in the destructor.
    The function takes one argument:

        - A reference to the full DDS_DynamicData object that includes custom_type.

    .. code-block:: c

        bool finalize_custom_type_dynamic_data(DDS_DynamicData & data)
        {
            bool success = true;
            if (!longSeq.ensure_length(0, 0)) {
                success = false;
                fprintf(stderr, "longSeq.ensure_length failed.\n");
            }
            DDS_ReturnCode_t retcode = data.clear_all_members();
            if (retcode != DDS_RETCODE_OK) {
                fprintf(stderr, "clear_all_members failed: %d.\n", retcode);
                success = false;
            }
            return success;
        }

3. Build *RTI Perftest* using ``--customType <type>``.

.. code-block:: bash

        ./build.sh --platform x64Linux3gcc5.4.0 --nddshome /home/rti_connext_dds-6.0.0 --cpp-build --customType Test

4. Launch *RTI Perftest*.

.. code-block:: bash

        ~/rtiperftest$ ./bin/x64Linux3gcc5.4.0/release/perftest_cpp -pub -executionTime 60 -noprint
        RTI Perftest 3.0.0 (RTI Connext DDS 6.0.0)

        Custom Type provided: 'Test'

        Mode: THROUGHPUT TEST
            (Use "-latencyTest" for Latency Mode)

        Perftest Configuration:
            Reliability: Reliable
            Keyed: No
            Publisher ID: 0
            Latency count: 1 latency sample every 10000 samples
            Data Size: 100
            Batching: 8192 Bytes (Use "-batchSize 0" to disable batching)
            Publication Rate: Unlimited (Not set)
            Execution time: 60 seconds
            Receive using: Listeners
            Domain: 1
            Dynamic Data: No
            Asynchronous Publishing: No
            XML File: perftest_qos_profiles.xml

        Transport Configuration:
            Kind: UDPv4 & SHMEM (taken from QoS XML file)
            Use Multicast: False

        Waiting to discover 1 subscribers ...
        Waiting for subscribers announcement ...
        Sending 4050 initialization pings ...
        Publishing data ...
        Setting timeout to 60 seconds
        Length:   464  Latency: Ave     39 us  Std   30.7 us  Min     21 us  Max    276 us  50%     30 us  90%     60 us  99%    276 us  99.99%    276 us  99.9999%    276 us
        Finishing test due to timer...
        Test ended.


.. code-block:: bash

        ~/rtiperftest$ ./bin/x64Linux3gcc5.4.0/release/perftest_cpp -sub -noprint
        RTI Perftest 3.0.0 (RTI Connext DDS 6.0.0)

        Custom Type provided: 'Test'

        Perftest Configuration:
            Reliability: Reliable
            Keyed: No
            Subscriber ID: 0
            Receive using: Listeners
            Domain: 1
            Dynamic Data: No
            XML File: perftest_qos_profiles.xml

        Transport Configuration:
            Kind: UDPv4 & SHMEM (taken from QoS XML file)
            Use Multicast: False

        Waiting to discover 1 publishers ...
        Waiting for data ...
        Length:   464  Packets: 68081040  Packets/s(ave): 1134692  Mbps(ave):  4212.0  Lost:     0 (0.00%)
        Finishing test...
        Test ended.



5. You can also launch *RTI Perftest* with your customType using DynamicData.

.. code-block:: bash

        ~/rtiperftest$ ./bin/x64Linux3gcc5.4.0/release/perftest_cpp -pub -executionTime 60 -noprint -dynamicData
        RTI Perftest 3.0.0 (RTI Connext DDS 6.0.0)

        Custom Type provided: 'Test'

        Mode: THROUGHPUT TEST
            (Use "-latencyTest" for Latency Mode)

        Perftest Configuration:
            Reliability: Reliable
            Keyed: No
            Publisher ID: 0
            Latency count: 1 latency sample every 10000 samples
            Data Size: 100
            Batching: 8192 Bytes (Use "-batchSize 0" to disable batching)
            Publication Rate: Unlimited (Not set)
            Execution time: 60 seconds
            Receive using: Listeners
            Domain: 1
            Dynamic Data: Yes
            Asynchronous Publishing: No
            XML File: perftest_qos_profiles.xml

        Transport Configuration:
            Kind: UDPv4 & SHMEM (taken from QoS XML file)
            Use Multicast: False

        Waiting to discover 1 subscribers ...
        Waiting for subscribers announcement ...
        Sending 4050 initialization pings ...
        Publishing data ...
        Setting timeout to 60 seconds
        Length:   464  Latency: Ave    158 us  Std  166.5 us  Min     71 us  Max    678 us  50%    105 us  90%    169 us  99%    678 us  99.99%    678 us  99.9999%    678 us
        Finishing test due to timer...
        Test ended.




.. code-block:: bash

        ~/rtiperftest$ ./bin/x64Linux3gcc5.4.0/release/perftest_cpp -sub -noprint -dynamicData
        RTI Perftest 3.0.0 (RTI Connext DDS 6.0.0)

        Custom Type provided: 'Test'

        Perftest Configuration:
            Reliability: Reliable
            Keyed: No
            Subscriber ID: 0
            Receive using: Listeners
            Domain: 1
            Dynamic Data: Yes
            XML File: perftest_qos_profiles.xml

        Transport Configuration:
            Kind: UDPv4 & SHMEM (taken from QoS XML file)
            Use Multicast: False

        Waiting to discover 1 publishers ...
        Waiting for data ...
        Length:   464  Packets:  8146078  Packets/s(ave):  135770  Mbps(ave):   504.0  Lost:     0 (0.00%)
        Finishing test...
        Test ended.
