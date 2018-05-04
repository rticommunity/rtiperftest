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
import com.rti.perftest.gen.KEY_SIZE;

public class DynamicDataTypeHelper implements TypeHelper<DynamicData> {
    public DynamicDataTypeHelper(TypeCode typeCode, boolean isKeyed, int maxPerftestSampleSize) {
        _isKeyed = isKeyed;
        _myData = new DynamicData(typeCode, DynamicData.PROPERTY_DEFAULT);
        _maxPerftestSampleSize = maxPerftestSampleSize;
        _byteArray = new byte[KEY_SIZE.VALUE];
    }

    public DynamicDataTypeHelper(DynamicData myData, int maxPerftestSampleSize) {
        _myData = myData;
        _maxPerftestSampleSize = maxPerftestSampleSize;
        _byteArray = new byte[KEY_SIZE.VALUE];
    }

    public void fillKey(int value) {
        _byteArray[0] = (byte) (value);
        _byteArray[1] = (byte) (value >>> 8);
        _byteArray[2] = (byte) (value >>> 16);
        _byteArray[3] = (byte) (value >>> 24);
        _myData.set_byte_array(
                "key",
                DynamicDataMembersId.getInstance().at("key"),
                _byteArray);
    }

    public void copyFromMessage(TestMessage message) {
        if (_last_message_size != message.size) {
            _myData.clear_all_members();
            // We won't copy the content of the message, since Perftest does not
            // fill it with actual data, it just allocates a buffer and sends it.
            ByteSeq bin_data = new ByteSeq();
            bin_data.setMaximum(message.size);
            bin_data.setSize(message.size);
            _myData.set_byte_seq(
                    "bin_data",
                    DynamicDataMembersId.getInstance().at("bin_data"),
                    bin_data);
            _last_message_size = message.size;
        }

        _myData.set_int(
                "entity_id",
                DynamicDataMembersId.getInstance().at("entity_id"),
                message.entity_id);
        _myData.set_int(
                "seq_num",
                DynamicDataMembersId.getInstance().at("seq_num"),
                message.seq_num);
        _myData.set_int(
                "timestamp_sec",
                DynamicDataMembersId.getInstance().at("timestamp_sec"),
                message.timestamp_sec);
        _myData.set_int(
                "timestamp_usec",
                DynamicDataMembersId.getInstance().at("timestamp_usec"),
                message.timestamp_usec);
        _myData.set_int(
                "latency_ping",
                DynamicDataMembersId.getInstance().at("latency_ping"),
                message.latency_ping);
    }

    @SuppressWarnings("rawtypes")
    public TestMessage copyFromSeqToMessage(List dataSeq, int index) {
        DynamicData msg = (DynamicData)((DynamicDataSeq)dataSeq).get(index);
        TestMessage message = new TestMessage();

        message.entity_id = msg.get_int(
                "entity_id",
                DynamicDataMembersId.getInstance().at("entity_id"));
        message.seq_num = msg.get_int(
                "seq_num",
                DynamicDataMembersId.getInstance().at("seq_num"));
        message.timestamp_sec = msg.get_int(
                "timestamp_sec",
                DynamicDataMembersId.getInstance().at("timestamp_sec"));
        message.timestamp_usec = msg.get_int(
                "timestamp_usec",
                DynamicDataMembersId.getInstance().at("timestamp_usec"));
        message.latency_ping = msg.get_int(
                "latency_ping",
                DynamicDataMembersId.getInstance().at("latency_ping"));

        ByteSeq bin_data = new ByteSeq();
        msg.get_byte_seq(
                bin_data,
                "bin_data",
                DynamicDataMembersId.getInstance().at("bin_data"));
        // bin_data already has the size, not necessary to call toArrayByte()
        message.size = bin_data.size();

        return message;
    }

    public DynamicData getData() {
        return _myData;
    }

    public void setBinDataMax(int newSize) {
        ByteSeq bin_data = new ByteSeq();
        _myData.get_byte_seq(
                bin_data,
                "bin_data",
                DynamicDataMembersId.getInstance().at("bin_data"));
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
    private int _last_message_size;
    private byte[] _byteArray;
}
