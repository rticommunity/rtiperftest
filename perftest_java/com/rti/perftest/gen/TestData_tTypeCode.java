
/*
WARNING: THIS FILE IS AUTO-GENERATED. DO NOT MODIFY.

This file was generated from .idl using "rtiddsgen".
The rtiddsgen tool is part of the RTI Connext distribution.
For more information, type 'rtiddsgen -help' at a command shell
or consult the RTI Connext manual.
*/

package com.rti.perftest.gen;

import com.rti.dds.typecode.*;

public class  TestData_tTypeCode {
    public static final TypeCode VALUE = getTypeCode();

    private static TypeCode getTypeCode() {
        TypeCode tc = null;
        int __i=0;
        StructMember sm[]=new StructMember[7];

        sm[__i]=new  StructMember("key", false, (short)-1,  false,(TypeCode) new TypeCode(new int[] {(com.rti.perftest.gen.KEY_SIZE.VALUE)}, TypeCode.TC_OCTET),0 , false);__i++;
        sm[__i]=new  StructMember("entity_id", false, (short)-1,  false,(TypeCode) TypeCode.TC_LONG,1 , false);__i++;
        sm[__i]=new  StructMember("seq_num", false, (short)-1,  false,(TypeCode) TypeCode.TC_ULONG,2 , false);__i++;
        sm[__i]=new  StructMember("timestamp_sec", false, (short)-1,  false,(TypeCode) TypeCode.TC_LONG,3 , false);__i++;
        sm[__i]=new  StructMember("timestamp_usec", false, (short)-1,  false,(TypeCode) TypeCode.TC_ULONG,4 , false);__i++;
        sm[__i]=new  StructMember("latency_ping", false, (short)-1,  false,(TypeCode) TypeCode.TC_LONG,5 , false);__i++;
        sm[__i]=new  StructMember("bin_data", false, (short)-1,  false,(TypeCode) new TypeCode((com.rti.perftest.gen.MAX_BINDATA_SIZE.VALUE), TypeCode.TC_OCTET),6 , false);__i++;

        tc = TypeCodeFactory.TheTypeCodeFactory.create_struct_tc("TestData_t",ExtensibilityKind.EXTENSIBLE_EXTENSIBILITY,  sm);        
        return tc;
    }
}

