/*
 * (c) 2005-2018  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */
#include "CustomType.h"

#ifdef RTI_WIN32
  #define snprintf sprintf_s
#endif

/*
 * This is the source code file that contains the implementation of the API
 * required to work with the Custom type.
 */

DDS_LongSeq long_seq;

bool initialize_custom_type_data(RTI_CUSTOM_TYPE &data)
{
    bool success = true;
    if (!data.test_seq.test_seq.ensure_length(SIZE_TEST_SEQ, SIZE_TEST_SEQ)) {
        success = false;
    }
    data.test_enum = ENUM1;
    return success;
}

void register_custom_type_data(RTI_CUSTOM_TYPE &data, unsigned long key)
{
    data.test_long = key;
}

bool set_custom_type_data(
        RTI_CUSTOM_TYPE &data,
        unsigned long key,
        int target_data_len)
{
    bool success = true;
    data.test_long = key;
    if (snprintf(data.test_string.test_string, SIZE_TEST_STRING, "Hello World! %lu", key) < 0) {
        success = false;
    }
    return success;
}

bool finalize_custom_type_data(RTI_CUSTOM_TYPE &data)
{
    bool success = true;
    if (!data.test_seq.test_seq.maximum(0)) {
        success = false;
    }
    return success;
}

bool initialize_custom_type_dynamic_data(DDS_DynamicData &data)
{
    bool success = long_seq.maximum(0);
    if (!success) {
        fprintf(stderr, "long_seq.maximum failed.\n");
    }
    return success;
}

void register_custom_type_dynamic_data(DDS_DynamicData &data, unsigned long key)
{
    DDS_ReturnCode_t retcode = data.set_long(
            "custom_type.test_long",
            DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED,
            key);
    if (retcode != DDS_RETCODE_OK) {
        fprintf(stderr, "set_long(test_long) failed: %d.\n", retcode);
    }
}

bool set_custom_type_dynamic_data(
        DDS_DynamicData &data,
        unsigned long key,
        int target_data_len)
{
    DDS_ReturnCode_t retcode;
    char test_string[SIZE_TEST_STRING]; //size of member_name
    bool success = true;
    DDS_DynamicData custom_type_data(NULL, DDS_DYNAMIC_DATA_PROPERTY_DEFAULT);
    DDS_DynamicData test_seq_data(NULL, DDS_DYNAMIC_DATA_PROPERTY_DEFAULT);

    retcode = data.set_long(
            "custom_type.test_long",
            DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED,
            key);
    if (retcode != DDS_RETCODE_OK) {
        fprintf(stderr, "set_long(test_long) failed: %d.\n", retcode);
        success = false;
    }

    if (snprintf(test_string, SIZE_TEST_STRING, "Hello World! %lu", key) < 0) {
        success = false;
    }
    retcode = data.set_string(
            "custom_type.test_string.test_string",
            DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED,
            test_string);
    if (retcode != DDS_RETCODE_OK) {
        fprintf(stderr, "set_string(test_string) failed: %d.\n", retcode);
        success = false;
    }

    retcode = data.set_long(
            "custom_type.test_enum",
            DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED,
            ENUM1);
    if (retcode != DDS_RETCODE_OK) {
        fprintf(stderr, "set_long(test_enum) failed: %d.\n", retcode);
        success = false;
    }

    retcode = data.bind_complex_member(custom_type_data, "custom_type",
            DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED);
    if (retcode != DDS_RETCODE_OK) {
        fprintf(
                stderr,
                "bind_complex_member(custom_type) failed: %d.\n",
                retcode);
        success = false;
    }
    retcode = custom_type_data.bind_complex_member(test_seq_data, "test_seq",
            DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED);
    if (retcode != DDS_RETCODE_OK) {
        fprintf(
                stderr,
                "bind_complex_member(test_seq_data) failed: %d.\n",
                retcode);
        success = false;
    }
    retcode = test_seq_data.set_long_seq(
                "test_seq",
                DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED,
                long_seq);
    if (retcode != DDS_RETCODE_OK) {
        fprintf(stderr, "set_long(test_seq) failed: %d.\n", retcode);
        success = false;
    }
    retcode = custom_type_data.unbind_complex_member(test_seq_data);
    if (retcode != DDS_RETCODE_OK) {
        fprintf(
                stderr,
                "bind_complex_member(test_seq_data) failed: %d.\n",
                retcode);
    }
    retcode = data.unbind_complex_member(custom_type_data);
    if (retcode != DDS_RETCODE_OK) {
        fprintf(
                stderr,
                "bind_complex_member(custom_type) failed: %d.\n",
                retcode);
        success = false;
    }
    return success;
}


bool finalize_custom_type_dynamic_data(DDS_DynamicData &data)
{
    bool success = long_seq.ensure_length(0, 0);
    if (!success) {
        fprintf(stderr, "long_seq.ensure_length failed.\n");
    }
    DDS_ReturnCode_t retcode = data.clear_all_members();
    if (retcode != DDS_RETCODE_OK) {
        fprintf(stderr, "clear_all_members failed: %d.\n", retcode);
        success = false;
    }
    return success;
}
