#ifndef __CUSTOMTYPE_H__
#define __CUSTOMTYPE_H__
/*
 * (c) 2005-2017  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */
 // TODO add the XPlugin.h of your type if call initialize_custom_type
#define RTI_CUSTOM_FILE_NAME_PLUGIN "testPlugin.h"
#include RTI_CUSTOM_FILE_NAME_PLUGIN

#define RTI_CUSTOM_FILE_NAME_SUPPORT "testSupport.h"
#include RTI_CUSTOM_FILE_NAME_SUPPORT

#include "perftest.h"
#include <string>

void initialize_custom_type(RTI_CUSTOM_TYPE & data);
void register_custom_type(RTI_CUSTOM_TYPE & data, unsigned long key);
void set_custom_type(
        RTI_CUSTOM_TYPE & data,
        unsigned long key,
        int target_data_len);
void finalize_data_custom_type(RTI_CUSTOM_TYPE & data);

void initialize_custom_type_dynamic(DDS_DynamicData & data);
void register_custom_type_dynamic(DDS_DynamicData & data, unsigned long key);
void set_custom_type_dynamic(
        DDS_DynamicData & data,
        unsigned long key,
        int target_data_len);
void finalize_data_custom_type_dynamic(DDS_DynamicData & data);

#endif // __CUSTOMTYPE_H__
