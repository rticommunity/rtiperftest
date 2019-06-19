/*
 * (c) 2005-2018  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */
#ifdef RTI_CUSTOM_TYPE
#include "CustomType.h"

/*
 * This is the source code file that contains the implementation of the API
 * required to work with the Custom type.
 */

DDS_LongSeq longSeq;

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
        int targetDataLen)
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
    bool success = true;
    if (!longSeq.ensure_length(SIZE_TEST_SEQ, SIZE_TEST_SEQ)) {
        success = false;
        fprintf(stderr, "longSeq.ensure_length failed.\n");
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
    retcode = data.unbind_complex_member(customTypeData);
    if (retcode != DDS_RETCODE_OK) {
        fprintf(stderr,
                "unbind_complex_member(custom_type) failed: %d.\n",
                retcode);
        success = false;
    }
    return success;
}

bool finalize_custom_type_dynamic_data(DDS_DynamicData &data)
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

#endif // RTI_CUSTOM_TYPE
