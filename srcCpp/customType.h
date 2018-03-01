#ifndef __CUSTOMTYPE_H__
#define __CUSTOMTYPE_H__
/*
 * (c) 2005-2017  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */
// TODO add the XSupport.h of your type
#include "testSupport.h"

#include "perftest.h"
#include "perftestSupport.h"


void initialize_custom_type(CustomType & data);
void set_custom_type(CustomType & data);
void unloan_custom_type(CustomType & data);
void set_custom_type_dynamic(DDS_DynamicData & data);

#endif // __CUSTOMTYPE_H__
