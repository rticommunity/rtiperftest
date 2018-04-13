#ifndef __CUSTOMTYPE_H__
#define __CUSTOMTYPE_H__
/*
 * (c) 2005-2017  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */
#define Q(x) #x
#define QUOTE(x) Q(x) // Used to set quotation marks

#include QUOTE(RTI_CUSTOM_TYPE_FILE_NAME_SUPPORT)

#include "perftest.h"
#include <string>
#include <new>          // std::bad_alloc

bool initialize_custom_type(RTI_CUSTOM_TYPE & data);
void register_custom_type(RTI_CUSTOM_TYPE & data, unsigned long key);
bool set_custom_type(
        RTI_CUSTOM_TYPE & data,
        unsigned long key,
        int target_data_len);
bool finalize_data_custom_type(RTI_CUSTOM_TYPE & data);

bool initialize_custom_type_dynamic(DDS_DynamicData & data);
void register_custom_type_dynamic(DDS_DynamicData & data, unsigned long key);
bool set_custom_type_dynamic(
        DDS_DynamicData & data,
        unsigned long key,
        int target_data_len);
bool finalize_data_custom_type_dynamic(DDS_DynamicData & data);

#endif // __CUSTOMTYPE_H__
