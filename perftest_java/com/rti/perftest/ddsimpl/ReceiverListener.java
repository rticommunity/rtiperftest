/* $Id: ReceiverListener.java,v 1.1.2.1 2014/04/01 11:56:54 juanjo Exp $

(c) 2005-2012  Copyright, Real-Time Innovations, Inc.  All rights reserved.    	
Permission to modify and use for internal purposes granted.   	
This software is provided "as is", without warranty, express or implied.

modification history:
--------------------
02apr08,rbw Fixed syntax error in printf() calls
01apr08,rbw Follow Java naming conventions
01apr08,rbw Created
=========================================================================== */

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
import com.rti.perftest.IMessagingCB;
import com.rti.perftest.TestMessage;
import com.rti.perftest.gen.MAX_BINDATA_SIZE;
import com.rti.perftest.gen.TestData_t;
import com.rti.perftest.gen.TestData_tDataReader;
import com.rti.perftest.gen.TestData_tSeq;


// ===========================================================================

/**
 * ReceiverListener
 */
/*package*/ final class ReceiverListener extends DataReaderAdapter {
    // -----------------------------------------------------------------------
    // Private Fields
    // -----------------------------------------------------------------------

    private TestData_tSeq _dataSeq = new TestData_tSeq();
    private SampleInfoSeq _infoSeq = new SampleInfoSeq();
    private TestMessage   _message  = new TestMessage();
    {
        _message.data = new byte[MAX_BINDATA_SIZE.VALUE];
    }
    private IMessagingCB _callback;



    // -----------------------------------------------------------------------
    // Public Methods
    // -----------------------------------------------------------------------

    // --- Constructors: -----------------------------------------------------

    public ReceiverListener(IMessagingCB callback) {
        _callback = callback;
    }


    // --- From DataReaderListener: ------------------------------------------

    @Override
    public void on_data_available(DataReader reader) {
        TestData_tDataReader datareader = (TestData_tDataReader)reader;
        try {
            try {
                datareader.take(
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
                    TestData_t msg = (TestData_t) _dataSeq.get(i);
                    _message.entity_id = msg.entity_id;
                    _message.seq_num = msg.seq_num;
                    _message.timestamp_sec = msg.timestamp_sec;
                    _message.timestamp_usec = msg.timestamp_usec;
                    _message.latency_ping = msg.latency_ping;
                    _message.size = msg.bin_data.size();
                    _message.data =
                        (byte[]) msg.bin_data.getPrimitiveArray();

                    _callback.processMessage(_message);
                }
            }
        } finally {
            try {
                datareader.return_loan(_dataSeq, _infoSeq);
            } catch (RETCODE_ERROR err) {
                System.out.println(
                    "Error during return loan: " + err.getMessage());
            }
        }
    }
}

// ===========================================================================
// End of $Id: ReceiverListener.java,v 1.1.2.1 2014/04/01 11:56:54 juanjo Exp $
