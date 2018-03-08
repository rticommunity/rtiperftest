/*
 * (c) 2005-2017  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */

#include "customType.h"
#include <string>

void initialize_custom_type(CustomType & data) {
    // CustomType::TypeSupport::initialize_data(&data);
}

void set_custom_type(CustomType & data) {
    // TODO initialize your data
    data.test_long = 5;
    data.stringTest.test_string = DDS_String_dup("Hello World!");
    data.test_enum = ENUM1;
    long * test_seq = new long[SIZE_TEST_SEQ];
    data.seqTest.test_seq.loan_contiguous((DDS_Long*)test_seq, SIZE_TEST_SEQ,SIZE_TEST_SEQ);
}

void unloan_custom_type(CustomType & data) {
    data.seqTest.test_seq.unloan();
}

void set_custom_type_dynamic(DDS_DynamicData & data) {
    // TODO initialize DDS_DynamicData
    DDS_ReturnCode_t retcode;
    long *test_seq = new long[SIZE_TEST_SEQ];

    retcode = data.set_long(
            "customType.test_long",
            DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED,
            5);
    if (retcode != DDS_RETCODE_OK) {
        fprintf(stderr, "set_long(test_long) failed: %d.\n", retcode);
    }

    retcode = data.set_string(
            "customType.stringTest.test_string",
            DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED,
            "Hello World!");
    if (retcode != DDS_RETCODE_OK) {
        fprintf(stderr, "set_string(stringTest.test_string) failed: %d.\n", retcode);
    }

    retcode = data.set_long(
            "customType.test_enum",
            DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED,
            ENUM1);
    if (retcode != DDS_RETCODE_OK) {
        fprintf(stderr, "set_long(test_enum) failed: %d.\n", retcode);
    }

    for (int i = 0; i < SIZE_TEST_SEQ; i++) {
        char member_name[29+21]; //size of member_name
        sprintf(member_name, "customType.seqTest.test_seq[%d]", i);
        retcode = data.set_long(
                member_name,
                DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED,
                test_seq[i]);
        if (retcode != DDS_RETCODE_OK) {
            fprintf(stderr, "set_long(seqTest.test_seq[%d]) failed: %d.\n", i, retcode);
        }
    }
}
