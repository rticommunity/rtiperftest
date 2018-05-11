/*
 * (c) 2005-2018  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */
#include "CustomType.h"

#ifdef RTI_WIN32
  #define snprintf sprintf_s
#endif
/*
 * It is the source code files that contain the implementation of the requires
 * and optional functions used for the handling of the custom type. These
 * functions will be used only if the parameter "--customType <type>" is aplpied
 * in the build. You can find more information in the point "Using Custom Types"
 * of the section "Use-Cases And Examples" in the documentation.
 */

long * test_seq = NULL;
DDS_LongSeq long_seq;

/*
*   This function is used to initialize your data.
*   @param data: A reference to the customer type.
*   @return true if the operation was success, otherwise false.
*/
bool initialize_custom_type(RTI_CUSTOM_TYPE &data)
{
    bool success = true;
    if (!data.test_seq.test_seq.maximum(0)) {
        success = false;
    }
    if (!data.test_seq.test_seq.ensure_length(SIZE_TEST_SEQ, SIZE_TEST_SEQ)) {
        success = false;
    }
    data.test_enum = ENUM1;
    return success;
}

/*
*   This function is used to set your data before being register.
*   @param data: A reference to the customer type.
*   @param key: A specific number unique for every key.
*/
void register_custom_type(RTI_CUSTOM_TYPE &data, unsigned long key)
{
    data.test_long = key;
}

/*
*   This function is used to set your data before being sent.
*   @param data: A reference to the customer type.
*   @param key: A specific number unique for every key.
*   @param targe_data_len: The target size set by the command line parameter
*   ``-dataLen <bytes>``
*   @return true if the operation was success, otherwise false.
*/
bool set_custom_type(
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

/*
*   This function is used to remove your data. It is called in the destructor.
*   @param data: A reference to the customer type.
*   @return true if the operation was success, otherwise false.
*/
bool finalize_data_custom_type(RTI_CUSTOM_TYPE &data)
{
    return true;
}

/*
*   This function is used to initialize your DynamicData.
*   @param data: A reference to the full DDS_DynamicData object including custom_type.
*   @return true if the operation was success, otherwise false.
*/
bool initialize_custom_type_dynamic(DDS_DynamicData &data)
{
    try {
        test_seq = new long[SIZE_TEST_SEQ];
    } catch (std::bad_alloc& ba) {
        fprintf(stderr, "bad_alloc test_seq  %s failed.\n",  ba.what());
        return false;
    }
    bool success = long_seq.from_array(
            (DDS_Long *) test_seq,
            SIZE_TEST_SEQ);
    if (! success) {
        fprintf(stderr, "from_array(test_seq) failed.\n");
        return false;
    }
    return true;
}

/*
*   This function is used to set your DynamicData before been register.
*   @param data: A reference to the full DDS_DynamicData object including custom_type.
*   @param key: A specific number unique for every key.
*/
void register_custom_type_dynamic(DDS_DynamicData &data, unsigned long key)
{
    DDS_ReturnCode_t retcode = data.set_long(
            "custom_type.test_long",
            DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED,
            key);
    if (retcode != DDS_RETCODE_OK) {
        fprintf(stderr, "set_long(test_long) failed: %d.\n", retcode);
    }
}

/*
*   This function is used to set your DynamicData before been sent.
*   @param data: A reference to the full DDS_DynamicData object including custom_type.
*   @param key: A specific number unique for every key.
*   @param targe_data_len: The target size set by the command line parameter
*   ``-dataLen <bytes>``
*   @return true if the operation was success, otherwise false.
*/
bool set_custom_type_dynamic(
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

/*
*   This function is used to remove your data. It is called in the destructor.
*   @param data: A reference to the full DDS_DynamicData object including custom_type.
*   @return true if the operation was success, otherwise false.
*/
bool finalize_data_custom_type_dynamic(DDS_DynamicData &data)
{
    if (test_seq != NULL) {
        delete[] test_seq;
        test_seq = NULL;
    }
    return true;
}
