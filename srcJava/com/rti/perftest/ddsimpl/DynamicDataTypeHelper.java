/*
 * (c) 2005-2017  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */

package com.rti.perftest.ddsimpl;

import java.util.List;

import com.rti.dds.dynamicdata.DynamicData;
import com.rti.dds.dynamicdata.DynamicDataSeq;
import com.rti.dds.dynamicdata.DynamicDataTypeSupport;
import com.rti.dds.infrastructure.ByteSeq;
import com.rti.dds.topic.TypeSupportImpl;
import com.rti.dds.typecode.TypeCode;
import com.rti.perftest.TestMessage;
import com.rti.perftest.gen.TestDataKeyed_tTypeSupport;
import com.rti.perftest.gen.TestData_tTypeSupport;
import com.rti.perftest.harness.PerfTest;

public class DynamicDataTypeHelper implements TypeHelper<DynamicData> {

    public DynamicDataTypeHelper(TypeCode typeCode, boolean isKeyed, int maxPerftestSampleSize) {
        _isKeyed = isKeyed;
        _myData = new DynamicData(typeCode, DynamicData.PROPERTY_DEFAULT);
        _maxPerftestSampleSize = maxPerftestSampleSize;
    }

    public DynamicDataTypeHelper(DynamicData myData,int maxPerftestSampleSize) {
        _myData = myData;
        _maxPerftestSampleSize = maxPerftestSampleSize;
    }

    public void fillKey(int value) {

        byte[] byteArray = new byte[4];
        byteArray[0] = (byte) (value);
        byteArray[1] = (byte) (value >>> 8);
        byteArray[2] = (byte) (value >>> 16);
        byteArray[3] = (byte) (value >>> 24);
        _myData.set_byte_array("key", 1, byteArray);

    }

    public void copyFromMessage(TestMessage message) {

        ByteSeq bin_data = new ByteSeq();
        bin_data.setMaximum(message.size);
        bin_data.setSize(message.size);
        // We won't copy the content of the message, since Perftest does not
        // fill it with actual data, it just allocates a buffer and sends it.

        _myData.clear_all_members();
        _myData.set_int("entity_id", 2, message.entity_id);
        _myData.set_int("seq_num", 3, message.seq_num);
        _myData.set_int("timestamp_sec", 4, message.timestamp_sec);
        _myData.set_int("timestamp_usec", 5, message.timestamp_usec);
        _myData.set_int("latency_ping", 6, message.latency_ping);
        _myData.set_byte_seq("bin_data", 7, bin_data);
    }

    @SuppressWarnings("rawtypes")
    public TestMessage copyFromSeqToMessage(List dataSeq, int index) {

        DynamicData msg = (DynamicData)((DynamicDataSeq)dataSeq).get(index);
        TestMessage message = new TestMessage();

        message.entity_id = msg.get_int("entity_id", 2);
        message.seq_num = msg.get_int("seq_num", 3);
        message.timestamp_sec = msg.get_int("timestamp_sec", 4);
        message.timestamp_usec = msg.get_int("timestamp_usec", 5);
        message.latency_ping = msg.get_int("latency_ping", 6);

        ByteSeq bin_data = new ByteSeq();
        msg.get_byte_seq(bin_data, "bin_data", 7);
        bin_data.toArrayByte(message.data);
        message.size = bin_data.size();

        return message;
    }

    public DynamicData getData() {
        return _myData;
    }

    public void setBinDataMax(int newSize) {
        ByteSeq bin_data = new ByteSeq();
        _myData.get_byte_seq(bin_data, "bin_data", 7);
        bin_data.setMaximum(newSize);
        bin_data.setSize(newSize);
    }

    public void bindataUnloan() {
        //Nothing to be done for Dynamic Data here.
    }

    public TypeSupportImpl getTypeSupport() {
        if (!_isKeyed) {
            return new DynamicDataTypeSupport(
                    TestData_tTypeSupport.getTypeCode(),
                    DynamicDataTypeSupport.TYPE_PROPERTY_DEFAULT);
        } else {
            return new DynamicDataTypeSupport(
                    TestDataKeyed_tTypeSupport.getTypeCode(),
                    DynamicDataTypeSupport.TYPE_PROPERTY_DEFAULT);
        }
    }

    public TypeHelper<DynamicData> clone() {
        return new DynamicDataTypeHelper(_myData, _maxPerftestSampleSize);
    }

    @SuppressWarnings("rawtypes")
    public List createSequence() {
        return new DynamicDataSeq();
    }

    @SuppressWarnings("rawtypes")
    public int getMaxPerftestSampleSize() {
        return _maxPerftestSampleSize;
    }

    private int _maxPerftestSampleSize = PerfTest.getMaxPerftestSampleSizeJava();
    private DynamicData _myData;
    private boolean _isKeyed;

}
