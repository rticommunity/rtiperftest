/* $Id: RTISubscriber.java,v 1.1.2.1 2014/04/01 11:56:54 juanjo Exp $

(c) 2005-2012  Copyright, Real-Time Innovations, Inc.  All rights reserved.    	
Permission to modify and use for internal purposes granted.   	
This software is provided "as is", without warranty, express or implied.

modification history:
--------------------
04may08,hhw Added WaitForWriters()
02apr08,rbw Fixed syntax error in printf() calls
01apr08,rbw Follow Java naming conventions
01apr08,rbw Created
=========================================================================== */

package com.rti.perftest.ddsimpl;

import com.rti.dds.infrastructure.ConditionSeq;
import com.rti.dds.infrastructure.Duration_t;
import com.rti.dds.infrastructure.RETCODE_ERROR;
import com.rti.dds.infrastructure.RETCODE_NO_DATA;
import com.rti.dds.infrastructure.ResourceLimitsQosPolicy;
import com.rti.dds.infrastructure.StatusCondition;
import com.rti.dds.infrastructure.StatusKind;
import com.rti.dds.infrastructure.WaitSet;
import com.rti.dds.infrastructure.WaitSetProperty_t;
import com.rti.dds.subscription.SubscriptionMatchedStatus;
import com.rti.dds.subscription.DataReader;
import com.rti.dds.subscription.InstanceStateKind;
import com.rti.dds.subscription.SampleInfo;
import com.rti.dds.subscription.SampleInfoSeq;
import com.rti.dds.subscription.SampleStateKind;
import com.rti.dds.subscription.ViewStateKind;
import com.rti.perftest.IMessagingReader;
import com.rti.perftest.TestMessage;
import com.rti.perftest.gen.MAX_BINDATA_SIZE;
import com.rti.perftest.gen.TestData_t;
import com.rti.perftest.gen.TestData_tDataReader;
import com.rti.perftest.gen.TestData_tSeq;


// ===========================================================================

/*package*/ final class RTISubscriber implements IMessagingReader {
    // -----------------------------------------------------------------------
    // Private Fields
    // -----------------------------------------------------------------------

    private static final Duration_t INFINITE = new Duration_t(
            Duration_t.DURATION_INFINITE_SEC,
            Duration_t.DURATION_INFINITE_NSEC);

    private TestData_tDataReader _reader;
    private TestData_tSeq _dataSeq = new TestData_tSeq();
    private SampleInfoSeq _infoSeq = new SampleInfoSeq();
    private TestMessage _message = new TestMessage();
    {
        _message.data = new byte[MAX_BINDATA_SIZE.VALUE];
    }
    private WaitSet _waitset = null;
    private ConditionSeq _activeConditions = new ConditionSeq();

    private int _dataIdx = 0;
    private boolean _noData = true;



    // -----------------------------------------------------------------------
    // Public Methods
    // -----------------------------------------------------------------------

    // --- Constructors: -----------------------------------------------------

    public RTISubscriber(DataReader reader) {
        _reader = (TestData_tDataReader) reader;

        // null listener means using receive thread
        if (_reader.get_listener() == null) {
            WaitSetProperty_t property = new WaitSetProperty_t();
            property.max_event_count = RTIDDSImpl.waitsetEventCount;
            property.max_event_delay.sec =
                RTIDDSImpl.waitsetDelayUsec / 1000000;
            property.max_event_delay.nanosec =
                (RTIDDSImpl.waitsetDelayUsec % 1000000) * 1000;

            _waitset = new WaitSet(property);

            StatusCondition reader_status = reader.get_statuscondition();
            reader_status.set_enabled_statuses(
                    StatusKind.DATA_AVAILABLE_STATUS);
            _waitset.attach_condition(reader_status);
        }
    }


    // --- From IMessagingReader: --------------------------------------------

    public void shutdown() {
        // loan may be outstanding during shutdown
        _reader.return_loan(_dataSeq, _infoSeq);
    }


    public TestMessage receiveMessage() {
        while (true) {
            // no outstanding reads
            if (_noData) {
                _waitset.wait(_activeConditions, INFINITE);

                if (_activeConditions.size() == 0) {
                    //System.out.print("Read thread woke up but no data\n");
                    //return null;
                    continue;
                }

                try {
                    _reader.take(
                            _dataSeq, _infoSeq,
                            ResourceLimitsQosPolicy.LENGTH_UNLIMITED,
                            SampleStateKind.ANY_SAMPLE_STATE,
                            ViewStateKind.ANY_VIEW_STATE,
                            InstanceStateKind.ANY_INSTANCE_STATE);
                } catch (RETCODE_NO_DATA noData) {
                    //System.out.print("Called back no data\n");
                    //return null;
                    continue;
                } catch (RETCODE_ERROR err) {
                    System.out.println(
                            "Error during taking data: " +
                            err.getMessage());
                    return null;
                }

                _dataIdx = 0;
                _noData = false;
            }

            int seq_size = _dataSeq.size();
            // check to see if hit end condition
            if (_dataIdx == seq_size) {
                _reader.return_loan(_dataSeq, _infoSeq);
                _noData = true;
                // for some reason, woke up, only got meta-data messages
                continue;
            }

            // skip non-valid data
            while ( (!((SampleInfo) _infoSeq.get(_dataIdx)).valid_data) && 
                    (++_dataIdx < seq_size)) {
                // empty
            }

            // may have hit end condition
            if (_dataIdx == seq_size) {
                continue;
            }

            TestData_t msg = (TestData_t) _dataSeq.get(_dataIdx);
            _message.entity_id = msg.entity_id;
            _message.seq_num = msg.seq_num;
            _message.timestamp_sec = msg.timestamp_sec;
            _message.timestamp_usec = msg.timestamp_usec;
            _message.latency_ping = msg.latency_ping;
            _message.size = msg.bin_data.size();
            _message.data = (byte[]) msg.bin_data.getPrimitiveArray();

            ++_dataIdx;

            return _message;
        }
    }

    public void waitForWriters(int numPublishers) {
        SubscriptionMatchedStatus status = new SubscriptionMatchedStatus();

        while (true) {
            _reader.get_subscription_matched_status(status);
            if (status.current_count >= numPublishers) {
                break;
            }
            try {
                Thread.sleep(1000);
            } catch (InterruptedException ix) {
                System.out.println("Wait interrupted");
                return;
            }
        }
    }
}

// ===========================================================================
// End of $Id: RTISubscriber.java,v 1.1.2.1 2014/04/01 11:56:54 juanjo Exp $
