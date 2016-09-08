/*
 * Copyright 2016
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
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
import com.rti.dds.util.AbstractSequence;
import com.rti.perftest.IMessagingReader;
import com.rti.perftest.TestMessage;
import com.rti.perftest.gen.MAX_BINDATA_SIZE;


// ===========================================================================

/*package*/ final class RTISubscriber<T> implements IMessagingReader {
    // -----------------------------------------------------------------------
    // Private Fields
    // -----------------------------------------------------------------------

    private static final Duration_t INFINITE = new Duration_t(
            Duration_t.DURATION_INFINITE_SEC,
            Duration_t.DURATION_INFINITE_NSEC);

    private DataReader _reader;
    private AbstractSequence _dataSeq = null;
    private SampleInfoSeq _infoSeq = new SampleInfoSeq();
    private TypeHelper<T> _myDataType = null;
    
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

    public RTISubscriber(DataReader reader, TypeHelper<T> myDatatype) {
        _reader = reader;
        _myDataType = myDatatype;
        _dataSeq =_myDataType.createSequence();

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
// End of $Id: RTISubscriber.java,v 1.4 2014/08/12 10:21:28 jmorales Exp $
