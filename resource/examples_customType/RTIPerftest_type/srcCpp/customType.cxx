/*
 * (c) 2005-2017  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */
#include "customType.h"

char * test_seq;
DDS_OctetSeq octect_seq;
int old_dynamic_target_data_len;

/**
*   This function is used to initialize your data.
*   @param data: A reference to the customer type.
*   @return true if the operation was success, otherwise false.
*/
bool initialize_custom_type(RTI_CUSTOM_TYPE & data)
{
    bool success = true;
    if (! data.test_seq.maximum(0)) {
        success = false;
    }
    return success;
}

/**
*   This function is used to set your data before been register.
*   @param data: A reference to the customer type.
*   @param key: A specific number unique for every key.
*/
void register_custom_type(RTI_CUSTOM_TYPE & data, unsigned long key)
{
}

/**
*   This function is used to set your data before being sent.
*   @param data: A reference to the customer type.
*   @param key: A specific number unique for every key.
*   @param targe_data_len: The target size set by the command line parameter
*   ``-dataLen <bytes>``
*   @return true if the operation was success, otherwise false.
*/
bool set_custom_type(
        RTI_CUSTOM_TYPE & data,
        unsigned long key,
        int target_data_len)
{
    bool success = true;
    if (! data.test_seq.ensure_length(target_data_len, target_data_len)) {
        success = false;
    }
    return success;
}

/**
*   This function is used to remove your data. It is called in the destructor.
*   @param data: A reference to the customer type.
*   @return true if the operation was success, otherwise false.
*/
bool finalize_data_custom_type(RTI_CUSTOM_TYPE & data)
{
    return true;
}

/**
*   This function is used to initialize your DynamicData.
*   @param data: A reference to the full DDS_DynamicData including custom_type.
*   @return true if the operation was success, otherwise false.
*/
bool initialize_custom_type_dynamic(DDS_DynamicData & data)
{
    old_dynamic_target_data_len = 0;
    try {
        test_seq = new char[MAX_SIZE];
    } catch (std::bad_alloc& ba) {
        fprintf(stderr, "bad_alloc test_seq  %s failed.\n",  ba.what());
        return false;
    }
    return true;
}

/**
*   This function is used to set your DynamicData before been register.
*   @param data: A reference to the full DDS_DynamicData including custom_type.
*   @param key: A specific number unique for every key.
*/
void register_custom_type_dynamic(DDS_DynamicData & data, unsigned long key)
{
}

/**
*   This function is used to set your DynamicData before been sent.
*   @param data: A reference to the full DDS_DynamicData including custom_type.
*   @param key: A specific number unique for every key.
*   @param targe_data_len: The target size set by the command line parameter
*   ``-dataLen <bytes>``
*   @return true if the operation was success, otherwise false.
*/
bool set_custom_type_dynamic(
        DDS_DynamicData & data,
        unsigned long key,
        int target_data_len)
{
    DDS_ReturnCode_t retcode;
    DDS_DynamicData custom_type_data(NULL, DDS_DYNAMIC_DATA_PROPERTY_DEFAULT);
    bool success = true;
    if (old_dynamic_target_data_len != target_data_len) {
        bool success = octect_seq.from_array(
                (DDS_Octet *) test_seq,
                target_data_len);
        if (! success) {
            fprintf(stderr, "from_array(test_seq) failed.\n");
            return false;
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

/**
*   This function is used to remove your data. It is called in the destructor.
*   @param data: A reference to the full DDS_DynamicData including custom_type.
*   @return true if the operation was success, otherwise false.
*/
bool finalize_data_custom_type_dynamic(DDS_DynamicData & data)
{
    if (test_seq != NULL) {
        delete[] test_seq;
        test_seq = NULL;
    }
    return true;
}
