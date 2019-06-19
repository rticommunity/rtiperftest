#ifdef RTI_CUSTOM_TYPE
#ifndef __CUSTOMTYPE_H__
#define __CUSTOMTYPE_H__
/*
 * (c) 2005-2018  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */
#define Q(x) #x
#define QUOTE(x) Q(x) // Used to set quotation marks

#include QUOTE(RTI_CUSTOM_TYPE_FILE_NAME_SUPPORT)

#include "perftest.h"
#include "perftest_cpp.h"
#include <string>

#if defined(RTI_WIN32) || defined(RTI_INTIME)
  #define snprintf sprintf_s
#endif

/*
 * This file contains the definition of the API required to work with the Custom
 * Type.
 *
 * These functions will be used only if the parameter "--customType <type>" is
 * applied in the build.
 * You can find more information in the section "Using Custom Types" under
 * "Use-Cases And Examples" in the documentation.
 *
 * Live cycle of the data:
 * - In the constructor:
 *      - "initialize_custom_type_data" is called.
 *      - "register_custom_type_data" is called with a specific key per instance.
 * - "set_custom_type_data" is called, with "key" and "targe_data_len", to set
 *      the data before it is sent.
 * - In the destructor, finalize_custom_type_data is called.
 */

/*
*   @brief This function is used to initialize your data.
*       Using this function, you will be able to allocate memory or set an
*       immutable field of the data.
*   @param data \b InOut. A reference to the custom type data.
*   @return true if the operation was successful, otherwise false.
*/
bool initialize_custom_type_data(RTI_CUSTOM_TYPE &data);

/*
*   @brief This function is used to set your data before being registered.
*       It is only required for key types.
*       Set the key field of the data based on the key input.
*       There is a one-to-one mapping between an input key and an instance.
*   @param data \b InOut. A reference to the custom type data.
*   @param key \b In. A specific number unique for every key.
*/
void register_custom_type_data(RTI_CUSTOM_TYPE &data, unsigned long key);

/*
*   @brief This function is used to set your data before it is sent.
*       It is called every time the data is sent.
*       You must set the Custom type data before it is sent with the right
*       "key" value and the "targetDataLen".
*   @param data \b InOut. A reference to the custom type data.
*   @param key \b In. A specific number unique for every key.
*   @param targe_data_len \b In. Value of the command-line parameter
*       "-dataLen <bytes>" minus the overhead of RTI Perftest.
*       If applicable, you can use this value to set the content of the data.
*   @return true if the operation was successful, otherwise false.
*/
bool set_custom_type_data(
        RTI_CUSTOM_TYPE &data,
        unsigned long key,
        int targetDataLen);

/*
*   @brief This function is used to remove your data. It is called in the destructor.
*   @param data \b InOut. A reference to the custom type data.
*   @return true if the operation was successful, otherwise false.
*/
bool finalize_custom_type_data(RTI_CUSTOM_TYPE &data);

/*
*   @brief This function is used to initialize your DynamicData.
*       Using this function, you will be able to allocate memory or set an
*       immutable field of the data.
*   @param data \b InOut. A reference to the full DDS_DynamicData object
*       that includes custom_type.
*   @return true if the operation was successful, otherwise false.
*/
bool initialize_custom_type_dynamic_data(DDS_DynamicData &data);

/*
*   @brief This function is used to set your DynamicData before it has been registered.
*       It is only required for key types.
*       Set the key field of the data based on the key input.
*       There is a one-to-one mapping between an input key and an instance.
*   @param data \b InOut. A reference to the full DDS_DynamicData object
*       that includes custom_type.
*   @param key \b In. A specific number unique for every key.
*/
void register_custom_type_dynamic_data(DDS_DynamicData &data, unsigned long key);


/*
*   @brief This function is used to set your DynamicData before it is sent.
*       It is called every time the data is sent.
*       You must set the Custom type data before it is sent with the right
*       "key" value and the "targetDataLen".
*   @param data \b InOut. A reference to the full DDS_DynamicData object
*       that includes custom_type.
*   @param key \b In. A specific number unique for every key.
*   @param targe_data_len \b In. Value of the command-line parameter
*       "-dataLen <bytes>" minus the overhead of RTI Perftest.
*       If applicable, you can use this value to set the content of the data.
*   @return true if the operation was successful, otherwise false.
*/
bool set_custom_type_dynamic_data(
        DDS_DynamicData &data,
        unsigned long key,
        int targetDataLen);

/*
*   @brief This function is used to remove your data. It is called in the destructor.
*   @param data \b InOut. A reference to the full DDS_DynamicData object
*       that includes custom_type.
*   @return true if the operation was successful, otherwise false.
*/
bool finalize_custom_type_dynamic_data(DDS_DynamicData &data);

#endif // __CUSTOMTYPE_H__
#endif // RTI_CUSTOM_TYPE