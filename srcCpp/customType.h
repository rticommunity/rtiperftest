#ifndef __CUSTOMTYPE_H__
#define __CUSTOMTYPE_H__
/*
 * (c) 2005-2017  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */
 // TODO add the XPlugin.h of your type if call initialize_custom_type
#define RTI_CUSTOM_FILE_NAME "testPlugin.h"
#include RTI_CUSTOM_FILE_NAME

#include "perftest.h"
#include <string>
#define concatenate(A, B) A ## B
#define get_serialized_sample_size(RTI_CUSTOM_TYPE) concatenate(RTI_CUSTOM_TYPE,Plugin_get_serialized_sample_size)
#define Fooget_serialized_sample_size get_serialized_sample_size(RTI_CUSTOM_TYPE)


void initialize_custom_type(RTI_CUSTOM_TYPE & data);
void register_custom_type(
        RTI_CUSTOM_TYPE & data,
        unsigned long key,
        int target_data_len);
void set_custom_type(
        RTI_CUSTOM_TYPE & data,
        unsigned long key,
        int target_data_len);
void initialize_custom_type_dynamic(DDS_DynamicData & data);
void register_custom_type_dynamic(DDS_DynamicData & data,
        unsigned long key,
        int target_data_len);
void set_custom_type_dynamic(DDS_DynamicData & data,
        unsigned long key,
        int target_data_len);

#endif // __CUSTOMTYPE_H__
