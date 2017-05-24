/*
 * (c) 2005-2016  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */

package com.rti.perftest.ddsimpl;

import java.util.List;
import com.rti.dds.topic.TypeSupportImpl;
import com.rti.perftest.TestMessage;
import com.rti.perftest.gen.TestData_t;
import com.rti.perftest.gen.TestData_tSeq;
import com.rti.perftest.gen.TestData_tTypeSupport;
import com.rti.perftest.harness.PerfTest;


public class DataTypeHelper implements TypeHelper<TestData_t> {

    public DataTypeHelper(int MAX_PERFTEST_SAMPLE_SIZE) {
        _myData = new TestData_t();
        _MAX_PERFTEST_SAMPLE_SIZE = MAX_PERFTEST_SAMPLE_SIZE;

    }


    public DataTypeHelper(TestData_t myData, int MAX_PERFTEST_SAMPLE_SIZE) {
        _myData = myData;
        _MAX_PERFTEST_SAMPLE_SIZE = MAX_PERFTEST_SAMPLE_SIZE;
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
        return new DataTypeHelper(_myData, _MAX_PERFTEST_SAMPLE_SIZE);
    }

    @SuppressWarnings("rawtypes")
    public List createSequence() {
        return new TestData_tSeq();
    }

    @SuppressWarnings("rawtypes")
    public int getMAX_PERFTEST_SAMPLE_SIZE() {
        return _MAX_PERFTEST_SAMPLE_SIZE;
    }

    private int _MAX_PERFTEST_SAMPLE_SIZE = PerfTest.MAX_PERFTEST_SAMPLE_SIZE_JAVA;
    private TestData_t _myData;

}
