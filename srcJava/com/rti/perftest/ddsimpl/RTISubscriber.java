/*
 * (c) 2005-2017  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */

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
import com.rti.perftest.harness.PerfTest;

import java.util.List;


// ===========================================================================

final class RTISubscriber<T> implements IMessagingReader {

    // -----------------------------------------------------------------------
    // Private Fields
    // -----------------------------------------------------------------------

    private static final Duration_t INFINITE = new Duration_t(
            Duration_t.DURATION_INFINITE_SEC,
            Duration_t.DURATION_INFINITE_NSEC);

    private DataReader _reader;
    @SuppressWarnings("rawtypes")
    private List _dataSeq = null;
    private SampleInfoSeq _infoSeq = new SampleInfoSeq();
    private TypeHelper<T> _myDataType = null;
    
    private TestMessage _message = new TestMessage();
    private WaitSet _waitset = null;
    private ConditionSeq _activeConditions = new ConditionSeq();

    private int _dataIdx = 0;
    private boolean _noData = true;
    
    
    // -----------------------------------------------------------------------
    // Public Methods
    // -----------------------------------------------------------------------

    // --- Constructors: -----------------------------------------------------

    @SuppressWarnings("rawtypes")
    public RTISubscriber(DataReader reader, TypeHelper<T> myDatatype) {
        _reader = reader;
        _myDataType = myDatatype;
        // We will avoid allocating this member of _message, since we won't
        // make use of it. This will not have any impact with regards to the
        // behavior of the test, but will allow us to test bigger sample sizes.
        // _message.data = new byte[_myDataType.getMaxPerftestSampleSize()];
        _dataSeq =(List)_myDataType.createSequence();

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
        _reader.return_loan_untyped(_dataSeq, _infoSeq);
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
                    _reader.take_untyped(
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
                _reader.return_loan_untyped(_dataSeq, _infoSeq);
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

            _message = _myDataType.copyFromSeqToMessage(_dataSeq,_dataIdx);

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
