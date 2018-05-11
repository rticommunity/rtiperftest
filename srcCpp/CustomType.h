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

/*
 * It is the header files that contain the definition of the requires and
 * optional functions used for the handling of the custom type. These functions
 * will be used only if the parameter "--customType <type>" is applied in the
 * build. You can find more information in the point "Using Custom Types" of the
 * section "Use-Cases And Examples" in the documentation.
 */

bool initialize_custom_type(RTI_CUSTOM_TYPE &data);
void register_custom_type(RTI_CUSTOM_TYPE &data, unsigned long key);
bool set_custom_type(
        RTI_CUSTOM_TYPE &data,
        unsigned long key,
        int target_data_len);
bool finalize_data_custom_type(RTI_CUSTOM_TYPE &data);

bool initialize_custom_type_dynamic(DDS_DynamicData &data);
void register_custom_type_dynamic(DDS_DynamicData &data, unsigned long key);
bool set_custom_type_dynamic(
        DDS_DynamicData &data,
        unsigned long key,
        int target_data_len);
bool finalize_data_custom_type_dynamic(DDS_DynamicData &data);

#endif // __CUSTOMTYPE_H__
