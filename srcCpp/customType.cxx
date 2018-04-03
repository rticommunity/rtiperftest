/*
 * (c) 2005-2017  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */
#include "customType.h"

long * test_seq;
DDS_LongSeq longSeq;

void initialize_custom_type(RTI_CUSTOM_TYPE & data)
{
    // TODO Initialize your data
    test_seq = new long[SIZE_TEST_SEQ];
    data.seqTest.test_seq.maximum(0);
}

void register_custom_type(
        RTI_CUSTOM_TYPE & data,
        unsigned long key)
{
    // TODO initialize your data to be registered
    data.test_long = key;
}

void set_custom_type(
        RTI_CUSTOM_TYPE & data,
        unsigned long key,
        int target_data_len)
{
    // TODO initialize your data to be sent
    data.test_long = key;
    data.stringTest.test_string = DDS_String_dup("Hello World!");
    data.test_enum = ENUM1;
    data.seqTest.test_seq.ensure_length(SIZE_TEST_SEQ, SIZE_TEST_SEQ);
    data.seqTest.test_seq.from_array(
            (DDS_Long*)test_seq,
            SIZE_TEST_SEQ);
}

void finalize_data_custom_type(RTI_CUSTOM_TYPE & data)
{
    // TODO delete the data. For example: unloan()...
}

void initialize_custom_type_dynamic(DDS_DynamicData & data)
{
    // TODO Initialize your data
    test_seq = new long[SIZE_TEST_SEQ];
    bool succeeded = longSeq.from_array(
            (DDS_Long *) test_seq,
            SIZE_TEST_SEQ);
    if (!succeeded) {
        fprintf(stderr, "from_array(test_seq) failed.\n");
    }
}

void register_custom_type_dynamic(
        DDS_DynamicData & data,
        unsigned long key)
{
    // TODO initialize DDS_DynamicData to be registered
    DDS_ReturnCode_t retcode;

    retcode = data.set_long(
            "custom_type.test_long",
            DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED,
            key);
    if (retcode != DDS_RETCODE_OK) {
        fprintf(stderr, "set_long(test_long) failed: %d.\n", retcode);
    }
}

void set_custom_type_dynamic(
        DDS_DynamicData & data,
        unsigned long key,
        int target_data_len)
{
    // TODO initialize DDS_DynamicData to be sent
    DDS_ReturnCode_t retcode;
    long *test_seq = new long[SIZE_TEST_SEQ];

    retcode = data.set_long(
            "custom_type.test_long",
            DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED,
            key);
    if (retcode != DDS_RETCODE_OK) {
        fprintf(stderr, "set_long(test_long) failed: %d.\n", retcode);
    }

    retcode = data.set_string(
            "custom_type.stringTest.test_string",
            DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED,
            "Hello World!");
    if (retcode != DDS_RETCODE_OK) {
        fprintf(
                stderr,
                "set_string(stringTest.test_string) failed: %d.\n",
                    retcode);
    }

    retcode = data.set_long(
            "custom_type.test_enum",
            DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED,
            ENUM1);
    if (retcode != DDS_RETCODE_OK) {
        fprintf(stderr, "set_long(test_enum) failed: %d.\n", retcode);
    }

    DDS_DynamicData custom_type_data(NULL, DDS_DYNAMIC_DATA_PROPERTY_DEFAULT);
    DDS_DynamicData test_seq_data(NULL, DDS_DYNAMIC_DATA_PROPERTY_DEFAULT);
    retcode = data.bind_complex_member(custom_type_data, "custom_type",
            DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED);
    if (retcode != DDS_RETCODE_OK) {
        fprintf(stderr, "bind_complex_member(custom_type) failed: %d.\n", retcode);
    }
    retcode = custom_type_data.bind_complex_member(test_seq_data, "seqTest",
            DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED);
    if (retcode != DDS_RETCODE_OK) {
        fprintf(stderr, "bind_complex_member(custom_type) failed: %d.\n", retcode);
    }
    retcode = test_seq_data.set_long_seq(
            "test_seq",
            DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED,
            longSeq);
    if (retcode != DDS_RETCODE_OK) {
        fprintf(stderr, "set_long(test_seq) failed: %d.\n", retcode);
    }
    retcode = custom_type_data.unbind_complex_member(test_seq_data);
    if (retcode != DDS_RETCODE_OK) {
        fprintf(stderr, "bind_complex_member(custom_type) failed: %d.\n", retcode);
    }
    retcode = data.unbind_complex_member(custom_type_data);
}

void finalize_data_custom_type_dynamic(DDS_DynamicData & data)
{
    // TODO delete the data.
}
