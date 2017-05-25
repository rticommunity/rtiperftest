/*
 * (c) 2005-2017  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */

package com.rti.perftest.ddsimpl;

import java.util.List;

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
import com.rti.perftest.harness.PerfTest;


// ===========================================================================

/**
 * ReceiverListener
 */
/*package*/ final class ReceiverListener<T> extends DataReaderAdapter {
    // -----------------------------------------------------------------------
    // Private Fields
    // -----------------------------------------------------------------------

    @SuppressWarnings("rawtypes")
    private List _dataSeq = null;
    private SampleInfoSeq _infoSeq = new SampleInfoSeq();
    private TestMessage   _message  = new TestMessage();
    private IMessagingCB _callback;
    private TypeHelper<T> _myDataType = null;

    // -----------------------------------------------------------------------
    // Public Methods
    // -----------------------------------------------------------------------

    // --- Constructors: -----------------------------------------------------

    @SuppressWarnings("rawtypes")
    public ReceiverListener(IMessagingCB callback, TypeHelper<T> myDatatype) {
        _callback = callback;
        _myDataType = myDatatype;
        // We will avoid allocating this member of _message, since we won't
        // make use of it. This will not have any impact with regards to the
        // behavior of the test, but will allow us to test bigger sample sizes.
        // _message.data = new byte[_myDataType.getMaxPerftestSampleSize()];
        _dataSeq =(List)_myDataType.createSequence();
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
