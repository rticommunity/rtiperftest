/*
 * (c) 2005-2018  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */
#include "CustomType.h"

/*
 * It is the source code file that contain the implementation of API required
 * to work with Custom Type.
 */

DDS_OctetSeq octect_seq;
int old_dynamic_target_data_len = 0;

bool initialize_custom_type_data(RTI_CUSTOM_TYPE &data)
{
    return true;
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
    if (!data.test_seq.ensure_length(target_data_len, target_data_len)) {
        success = false;
    }
    data.test_long = key;
    return success;
}

bool finalize_custom_type_data(RTI_CUSTOM_TYPE &data)
{
    bool success = data.test_seq.ensure_length(0, 0);
    return success;
}

bool initialize_custom_type_dynamic_data(DDS_DynamicData &data)
{
    bool success = octect_seq.maximum(0);
    if (!success) {
        fprintf(stderr, "octect_seq.maximum failed.\n");
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
    DDS_DynamicData custom_type_data(NULL, DDS_DYNAMIC_DATA_PROPERTY_DEFAULT);
    bool success = true;
    if (old_dynamic_target_data_len != target_data_len) {
        success = octect_seq.ensure_length(target_data_len, target_data_len);
        if (!success) {
            fprintf(stderr, "octect_seq.ensure_length failed.\n");
        }
        old_dynamic_target_data_len = target_data_len;
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
    retcode = custom_type_data.set_octet_seq(
            "test_seq",
            DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED,
            octect_seq);
    if (retcode != DDS_RETCODE_OK) {
        fprintf(stderr, "set_octet_seq(test_seq) failed: %d.\n", retcode);
        success = false;
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
    bool success = octect_seq.ensure_length(0, 0);
    if (!success) {
        fprintf(stderr, "octect_seq.ensure_length failed.\n");
    }
    DDS_ReturnCode_t retcode = data.clear_all_members();
    if (retcode != DDS_RETCODE_OK) {
        fprintf(stderr, "clear_all_members failed: %d.\n", retcode);
        success = false;
    }
    return success;
}
