/*
 * (c) 2005-2018  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */
#include "CustomType.h"
/*
 * It is the source code files that contain the implementation of the requires
 * and optional functions used for the handling of the custom type. These
 * functions will be used only if the parameter "--customType <type>" is aplpied
 * in the build. You can find more information in the point "Using Custom Types"
 * of the section "Use-Cases And Examples" in the documentation.
 */

/*
*   This function is used to initialize your data.
*   @param data: A reference to the customer type.
*   @return true if the operation was success, otherwise false.
*/
bool initialize_custom_type(RTI_CUSTOM_TYPE &data)
{
    return true;
}

/*
*   This function is used to set your data before being register.
*   @param data: A reference to the customer type.
*   @param key: A specific number unique for every key.
*/
void register_custom_type(RTI_CUSTOM_TYPE &data, unsigned long key)
{
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
    return true;
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
    return true;
}

/*
*   This function is used to set your DynamicData before been register.
*   @param data: A reference to the full DDS_DynamicData object including custom_type.
*   @param key: A specific number unique for every key.
*/
void register_custom_type_dynamic(DDS_DynamicData &data, unsigned long key)
{
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
    return true;
}

/*
*   This function is used to remove your data. It is called in the destructor.
*   @param data: A reference to the full DDS_DynamicData object including custom_type.
*   @return true if the operation was success, otherwise false.
*/
bool finalize_data_custom_type_dynamic(DDS_DynamicData &data)
{
    return true;
}
