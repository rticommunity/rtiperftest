/*
 * (c) 2005-2016  Copyright, Real-Time Innovations, Inc.  All rights reserved.
 * Permission to modify and use for internal purposes granted.
 * This software is provided "as is", without warranty, express or implied.
 */

package com.rti.perftest.ddsimpl;

import com.rti.dds.infrastructure.ByteSeq;
import com.rti.dds.topic.TypeSupportImpl;
import com.rti.dds.util.AbstractSequence;
import com.rti.perftest.TestMessage;
import com.rti.perftest.gen.TestDataKeyed_t;
import com.rti.perftest.gen.TestDataKeyed_tSeq;
import com.rti.perftest.gen.TestDataKeyed_tTypeSupport;

public class DataTypeKeyedHelper implements TypeHelper<TestDataKeyed_t> {

    public DataTypeKeyedHelper() {
        _myData = new TestDataKeyed_t();
    }

    public DataTypeKeyedHelper(TestDataKeyed_t myData) {
        _myData = myData;
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

    public TestMessage copyFromSeqToMessage(AbstractSequence dataSeq, int index) {

        TestDataKeyed_t msg = (TestDataKeyed_t)dataSeq.get(index);
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

    public TestDataKeyed_t getData() {
        return _myData;
    }

    public ByteSeq getBindata() {
        return _myData.bin_data;
    }

    public TypeSupportImpl getTypeSupport() {
        return TestDataKeyed_tTypeSupport.getInstance();
    }

    public TypeHelper<TestDataKeyed_t> clone(){
        return new DataTypeKeyedHelper(_myData);
    }

    public AbstractSequence createSequence() {
        return new TestDataKeyed_tSeq();
    }

    private TestDataKeyed_t _myData;

}
