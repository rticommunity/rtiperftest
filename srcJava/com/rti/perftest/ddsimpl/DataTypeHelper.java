/*
 * (c) 2005-2017  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */

package com.rti.perftest.ddsimpl;

import java.util.List;
import com.rti.dds.topic.TypeSupportImpl;
import com.rti.dds.cdr.CdrEncapsulation;
import com.rti.perftest.TestMessage;
import com.rti.perftest.gen.TestData_t;
import com.rti.perftest.gen.TestData_tSeq;
import com.rti.perftest.gen.TestData_tTypeSupport;
import com.rti.perftest.harness.PerfTest;

public class DataTypeHelper implements TypeHelper<TestData_t> {

    public DataTypeHelper(int maxPerftestSampleSize) {
        _myData = new TestData_t();
        _maxPerftestSampleSize = maxPerftestSampleSize;
    }


    public DataTypeHelper(TestData_t myData, int maxPerftestSampleSize) {
        _myData = new TestData_t(myData);
        _maxPerftestSampleSize = maxPerftestSampleSize;
    }

    public void fillKey(int value) {

        _myData.key[0] = (byte) (value);
        _myData.key[1] = (byte) (value >>> 8);
        _myData.key[2] = (byte) (value >>> 16);
        _myData.key[3] = (byte) (value >>> 24);

    }

    public void copyFromMessage(TestMessage message) {

        _myData.entity_id = message.entity_id;
        _myData.seq_num = message.seq_num;
        _myData.timestamp_sec = message.timestamp_sec;
        _myData.timestamp_usec = message.timestamp_usec;
        _myData.latency_ping = message.latency_ping;
        _myData.bin_data.loan(message.data, message.size);

    }

    @SuppressWarnings("rawtypes")
    public TestMessage copyFromSeqToMessage(List dataSeq, int index) {

        TestData_t msg = (TestData_t)((List)dataSeq).get(index);
        TestMessage message = new TestMessage();

        message.entity_id = msg.entity_id;
        message.seq_num = msg.seq_num;
        message.timestamp_sec = msg.timestamp_sec;
        message.timestamp_usec = msg.timestamp_usec;
        message.latency_ping = msg.latency_ping;
        message.size = msg.bin_data.size();
        message.data = (byte[]) msg.bin_data.getPrimitiveArray();

        return message;
    }

    public TestData_t getData() {
        return _myData;
    }

    public void setBinDataMax(int newSize) {
        _myData.bin_data.setMaximum(newSize);
        _myData.bin_data.setSize(newSize);
    }

    public void bindataUnloan() {
        _myData.bin_data.unloan();
    }

    public TypeSupportImpl getTypeSupport() {
        return TestData_tTypeSupport.getInstance();
    }

    public TypeHelper<TestData_t> clone() {
        return new DataTypeHelper(_myData, _maxPerftestSampleSize);
    }

    @SuppressWarnings("rawtypes")
    public List createSequence() {
        return new TestData_tSeq();
    }

    @SuppressWarnings("rawtypes")
    public int getMaxPerftestSampleSize() {
        return _maxPerftestSampleSize;
    }

    public long getSerializedOverheadSize() {
        TestData_t  myType = new  TestData_t();
        myType.entity_id = 0;
        myType.seq_num = 0;
        myType.timestamp_sec = 0;
        myType.timestamp_usec = 0;
        myType.latency_ping = 0;
        myType.bin_data.setMaximum(0);
        myType.bin_data.setSize(0);

        /*
         * Setting the input buffer to null, this function will return the
         * serialize sample size. Also the length will be ignored.
         */
        return getTypeSupport().serialize_to_cdr_buffer(null, 0, myType)
                - CdrEncapsulation.CDR_ENCAPSULATION_HEADER_SIZE;
    }

    private int _maxPerftestSampleSize = PerfTest.getMaxPerftestSampleSizeJava();
    private TestData_t _myData;

}
