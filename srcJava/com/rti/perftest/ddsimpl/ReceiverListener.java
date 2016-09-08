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

import com.rti.dds.infrastructure.RETCODE_ERROR;
import com.rti.dds.infrastructure.RETCODE_NO_DATA;
import com.rti.dds.infrastructure.ResourceLimitsQosPolicy;
import com.rti.dds.subscription.DataReader;
import com.rti.dds.subscription.DataReaderAdapter;
import com.rti.dds.subscription.InstanceStateKind;
import com.rti.dds.subscription.SampleInfo;
import com.rti.dds.subscription.SampleInfoSeq;
import com.rti.dds.subscription.SampleStateKind;
import com.rti.dds.subscription.ViewStateKind;
import com.rti.dds.util.AbstractSequence;
import com.rti.perftest.IMessagingCB;
import com.rti.perftest.TestMessage;
import com.rti.perftest.gen.MAX_BINDATA_SIZE;


// ===========================================================================

/**
 * ReceiverListener
 */
/*package*/ final class ReceiverListener<T> extends DataReaderAdapter {
    // -----------------------------------------------------------------------
    // Private Fields
    // -----------------------------------------------------------------------

    private AbstractSequence _dataSeq = null;
    private SampleInfoSeq _infoSeq = new SampleInfoSeq();
    private TestMessage   _message  = new TestMessage();
    {
        _message.data = new byte[MAX_BINDATA_SIZE.VALUE];
    }
    private IMessagingCB _callback;
    private TypeHelper<T> _myDataType = null;



    // -----------------------------------------------------------------------
    // Public Methods
    // -----------------------------------------------------------------------

    // --- Constructors: -----------------------------------------------------

    public ReceiverListener(IMessagingCB callback, TypeHelper<T> myDatatype) {
        _callback = callback;
        _myDataType = myDatatype;
        _dataSeq =_myDataType.createSequence();
    }


    // --- From DataReaderListener: ------------------------------------------

    @Override
    public void on_data_available(DataReader reader) {
        try {
            try {
                reader.take_untyped(
                        _dataSeq, _infoSeq,
                        ResourceLimitsQosPolicy.LENGTH_UNLIMITED,
                        SampleStateKind.ANY_SAMPLE_STATE,
                        ViewStateKind.ANY_VIEW_STATE,
                        InstanceStateKind.ANY_INSTANCE_STATE);
            } catch (RETCODE_NO_DATA noData) {
                System.out.print("called back no data\n");
                return;
            } catch (RETCODE_ERROR err) {
                System.out.println(
                    "Error during taking data: " + err.getMessage());
                return;
            }

            int seq_size = _dataSeq.size();
            for (int i = 0; i < seq_size; ++i) {
                if (((SampleInfo) _infoSeq.get(i)).valid_data) {
                    
                    _message = _myDataType.copyFromSeqToMessage(_dataSeq, i);
                    _callback.processMessage(_message);
                }
    }
        } finally {
            try {
                reader.return_loan_untyped(_dataSeq, _infoSeq);
            } catch (RETCODE_ERROR err) {
                System.out.println(
                    "Error during return loan: " + err.getMessage());
            }
        }
    }
}

// ===========================================================================
// End of $Id: ReceiverListener.java,v 1.4 2014/08/12 10:21:28 jmorales Exp $
