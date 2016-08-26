

/*
WARNING: THIS FILE IS AUTO-GENERATED. DO NOT MODIFY.

This file was generated from .idl using "rtiddsgen".
The rtiddsgen tool is part of the RTI Connext distribution.
For more information, type 'rtiddsgen -help' at a command shell
or consult the RTI Connext manual.
*/

package com.rti.perftest.gen;

import com.rti.dds.infrastructure.*;
import com.rti.dds.infrastructure.Copyable;
import java.io.Serializable;
import com.rti.dds.cdr.CdrHelper;

public class TestData_t   implements Copyable, Serializable{

    public byte [] key=  new byte [(com.rti.perftest.gen.KEY_SIZE.VALUE)];
    public int entity_id= 0;
    public int seq_num= 0;
    public int timestamp_sec= 0;
    public int timestamp_usec= 0;
    public int latency_ping= 0;
    public ByteSeq bin_data =  new ByteSeq(((com.rti.perftest.gen.MAX_BINDATA_SIZE.VALUE)));

    public TestData_t() {

    }
    public TestData_t (TestData_t other) {

        this();
        copy_from(other);
    }

    public static Object create() {

        TestData_t self;
        self = new  TestData_t();
        self.clear();
        return self;

    }

    public void clear() {

        for(int i1__ = 0; i1__< (com.rti.perftest.gen.KEY_SIZE.VALUE); ++i1__){

            key[i1__] =  0;
        }

        entity_id= 0;
        seq_num= 0;
        timestamp_sec= 0;
        timestamp_usec= 0;
        latency_ping= 0;
        if (bin_data != null) {
            bin_data.clear();
        }
    }

    public boolean equals(Object o) {

        if (o == null) {
            return false;
        }        

        if(getClass() != o.getClass()) {
            return false;
        }

        TestData_t otherObj = (TestData_t)o;

        for(int i1__ = 0; i1__< (com.rti.perftest.gen.KEY_SIZE.VALUE); ++i1__){

            if(key[i1__] != otherObj.key[i1__]) {
                return false;
            }
        }

        if(entity_id != otherObj.entity_id) {
            return false;
        }
        if(seq_num != otherObj.seq_num) {
            return false;
        }
        if(timestamp_sec != otherObj.timestamp_sec) {
            return false;
        }
        if(timestamp_usec != otherObj.timestamp_usec) {
            return false;
        }
        if(latency_ping != otherObj.latency_ping) {
            return false;
        }
        if(!bin_data.equals(otherObj.bin_data)) {
            return false;
        }

        return true;
    }

    public int hashCode() {
        int __result = 0;
        for(int i1__ = 0; i1__< (com.rti.perftest.gen.KEY_SIZE.VALUE); ++i1__){

            __result += (int)key[i1__];
        }

        __result += (int)entity_id;
        __result += (int)seq_num;
        __result += (int)timestamp_sec;
        __result += (int)timestamp_usec;
        __result += (int)latency_ping;
        __result += bin_data.hashCode(); 
        return __result;
    }

    /**
    * This is the implementation of the <code>Copyable</code> interface.
    * This method will perform a deep copy of <code>src</code>
    * This method could be placed into <code>TestData_tTypeSupport</code>
    * rather than here by using the <code>-noCopyable</code> option
    * to rtiddsgen.
    * 
    * @param src The Object which contains the data to be copied.
    * @return Returns <code>this</code>.
    * @exception NullPointerException If <code>src</code> is null.
    * @exception ClassCastException If <code>src</code> is not the 
    * same type as <code>this</code>.
    * @see com.rti.dds.infrastructure.Copyable#copy_from(java.lang.Object)
    */
    public Object copy_from(Object src) {

        TestData_t typedSrc = (TestData_t) src;
        TestData_t typedDst = this;

        System.arraycopy(typedSrc.key,0,
        typedDst.key,0,
        typedSrc.key.length); 

        typedDst.entity_id = typedSrc.entity_id;
        typedDst.seq_num = typedSrc.seq_num;
        typedDst.timestamp_sec = typedSrc.timestamp_sec;
        typedDst.timestamp_usec = typedSrc.timestamp_usec;
        typedDst.latency_ping = typedSrc.latency_ping;
        typedDst.bin_data.copy_from(typedSrc.bin_data);

        return this;
    }

    public String toString(){
        return toString("", 0);
    }

    public String toString(String desc, int indent) {
        StringBuffer strBuffer = new StringBuffer();        

        if (desc != null) {
            CdrHelper.printIndent(strBuffer, indent);
            strBuffer.append(desc).append(":\n");
        }

        CdrHelper.printIndent(strBuffer, indent+1);
        strBuffer.append("key: ");
        for(int i1__ = 0; i1__< (com.rti.perftest.gen.KEY_SIZE.VALUE); ++i1__){

            strBuffer.append(key[i1__]).append(", ");
        }

        strBuffer.append("\n");
        CdrHelper.printIndent(strBuffer, indent+1);        
        strBuffer.append("entity_id: ").append(entity_id).append("\n");  
        CdrHelper.printIndent(strBuffer, indent+1);        
        strBuffer.append("seq_num: ").append(seq_num).append("\n");  
        CdrHelper.printIndent(strBuffer, indent+1);        
        strBuffer.append("timestamp_sec: ").append(timestamp_sec).append("\n");  
        CdrHelper.printIndent(strBuffer, indent+1);        
        strBuffer.append("timestamp_usec: ").append(timestamp_usec).append("\n");  
        CdrHelper.printIndent(strBuffer, indent+1);        
        strBuffer.append("latency_ping: ").append(latency_ping).append("\n");  
        CdrHelper.printIndent(strBuffer, indent+1);
        strBuffer.append("bin_data: ");
        for(int i__ = 0; i__ < bin_data.size(); ++i__) {
            if (i__!=0) strBuffer.append(", ");
            strBuffer.append(bin_data.get(i__));
        }
        strBuffer.append("\n"); 

        return strBuffer.toString();
    }

}
