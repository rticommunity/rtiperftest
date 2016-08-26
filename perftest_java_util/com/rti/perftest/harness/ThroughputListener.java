/* $Id: ThroughputListener.java,v 1.1.2.1 2014/04/01 11:56:55 juanjo Exp $

(c) 2005-2012  Copyright, Real-Time Innovations, Inc.  All rights reserved.    	
Permission to modify and use for internal purposes granted.   	
This software is provided "as is", without warranty, express or implied.

modification history:
--------------------
14may09,fcs Fixed scan output
23oct08,rbw Fail test if send() fails
12may08,hhw Fixed some length checks to accommodate 32 byte overhead.
02may08,hhw Augmented for N to 1 tests.
22apr08,fcs Fixed missing packets count
21apr08,ch  Output modifications for automation
08apr08,rbw Fixed compile errors; eliminated duplicate _beginTime init
07apr08,hhw Now printing end of test in listener to avoid race condition.
03apr08,rbw Fixed initialization of test's beginning time
02apr08,rbw Moved to package com.rti.perftest.harness to distinguish between
            (1) RTI-specific test implementation and (2) generic test harness
01apr08,rbw Follow Java naming conventions
01apr08,rbw Created
=========================================================================== */

package com.rti.perftest.harness;

import com.rti.perftest.IMessagingCB;
import com.rti.perftest.IMessagingReader;
import com.rti.perftest.IMessagingWriter;
import com.rti.perftest.TestMessage;


//===========================================================================

/**
 * Listener for the Subscriber side
 *
 * Keeps stats on data received per second.
 * Returns a ping for latency packets
 */
/*package*/ final class ThroughputListener implements IMessagingCB {
    // -----------------------------------------------------------------------
    // Public Fields
    // -----------------------------------------------------------------------

    public long packetsReceived = 0;
    public long bytesReceived = 0;
    public long missingPackets = 0;
    public boolean endTest = false;
    public int  lastDataLength = -1;

    // store info for the last data set
    public int  intervalDataLength = -1;
    public long intervalPacketsReceived = 0;
    public long intervalBytesReceived = 0;
    public long intervalMissingPackets = 0;
    public long intervalTime = 0;



    // -----------------------------------------------------------------------
    // Private Fields
    // -----------------------------------------------------------------------

    private IMessagingWriter _writer;
    private IMessagingReader _reader = null;
    private long _last_seq_num[];
    private long _beginTime;
    private int _finishCount = 0;
    private int _numPublishers;




    // -----------------------------------------------------------------------
    // Public Methods
    // -----------------------------------------------------------------------

    // --- Constructors: -----------------------------------------------------

    public ThroughputListener(IMessagingWriter writer,
                              int numPublishers) {
        _writer = writer;
        _last_seq_num = new long[numPublishers];
        _numPublishers = numPublishers;
    }


    public ThroughputListener(IMessagingWriter writer,
                              IMessagingReader reader,
                              int numPublishers) {
        _writer = writer;
        _reader = reader;
        _last_seq_num = new long[numPublishers];
        _numPublishers = numPublishers;
    }


    // -----------------------------------------------------------------------

    /**
     * Used for receiving data using a thread instead of callback.
     */
    public void ReadThread() {
        TestMessage message = null;
        while (!endTest) {
            // Receive message should block until a message is received
            message = _reader.receiveMessage();
            if (message != null) {
                processMessage(message);
            }
        }
    }
    
    
    // --- From IMessagingCB: ------------------------------------------------

    public void processMessage(TestMessage message) {
        // Check for test initialization messages
        if (message.size == PerfTest.INITIALIZE_SIZE) {
            _writer.send(message);      // don't check return; exiting anyway
            _writer.flush();
            return;
        }
        if (message.size == PerfTest.FINISHED_SIZE) {

            // only respond to publisher id 0
            if (message.entity_id != 0) {
                return;
            }

            if (endTest == true) {
                return;
            }
             
            _finishCount++;

            if (_finishCount >= _numPublishers) {
                // detect missing packets
                if (message.seq_num != _last_seq_num[message.entity_id]) {
                    // only track if skipped, might have restarted pub
                    if (message.seq_num > _last_seq_num[message.entity_id])
                    {
                        missingPackets +=
                            message.seq_num - _last_seq_num[message.entity_id];
                    }
                }

                // store the info for this interval
                long now = PerfTest.getTimeUsec();
                intervalTime = now - _beginTime;
                intervalPacketsReceived = packetsReceived;
                intervalBytesReceived = bytesReceived;
                intervalMissingPackets = missingPackets;
                intervalDataLength = lastDataLength;
                endTest = true;
            }

            boolean sent = _writer.send(message);
            _writer.flush();
            if (!sent) {
                System.out.println("*** send() failure");
                return;
            }

            if (_finishCount >= _numPublishers) {
                System.out.printf("Length: %1$5d  Packets: %2$8d  Packets/s(ave): %3$7.0f  " +
                                  "Mbps(ave): %4$7.1f  Lost: %5$d\n",
                                  intervalDataLength + PerfTest.OVERHEAD_BYTES,
                                  intervalPacketsReceived,
                                  intervalPacketsReceived * 1000000.0 / intervalTime,
                                  intervalBytesReceived * 1000000.0 / intervalTime *8.0/1000.0/1000.0,
                                  intervalMissingPackets);
                System.out.flush();
            }
            return;
        }

        // Send back a packet if this is a ping
        if (message.latency_ping == PerfTest.subID) {
            boolean sent = _writer.send(message);
            _writer.flush();
            if (!sent) {
                System.out.println("*** send() failure");
                return;
            }
        }

        // reset internals
        if (message.size == PerfTest.LENGTH_CHANGED_SIZE) {
            // store the info for this interval
            long now = PerfTest.getTimeUsec();

            // may have many length changed packets to support best effort
            if (intervalDataLength != lastDataLength)
            {
                // detect missing packets
                if (message.seq_num != _last_seq_num[message.entity_id]) {
                    // only track if skipped, might have restarted pub
                    if (message.seq_num > _last_seq_num[message.entity_id])
                    {
                        missingPackets +=
                            message.seq_num - _last_seq_num[message.entity_id];
                    }
                }

                intervalTime = now - _beginTime;
                intervalPacketsReceived = packetsReceived;
                intervalBytesReceived = bytesReceived;
                intervalMissingPackets = missingPackets;
                intervalDataLength = lastDataLength;

                System.out.printf(
                    "Length: %1$5d  Packets: %2$8d  Packets/s(ave): %3$7.0f  " +
                    "Mbps(ave): %4$7.1f  Lost: %5$d\n",
                    intervalDataLength + PerfTest.OVERHEAD_BYTES,
                    intervalPacketsReceived,
                    intervalPacketsReceived *
                        1000000.0 / intervalTime,
                    intervalBytesReceived *
                        1000000.0 / intervalTime *
                        8.0/1000.0/1000.0,
                    intervalMissingPackets);
                System.out.flush();
            }
            
            packetsReceived = 0;
            bytesReceived = 0;
            missingPackets = 0;
            // length changed only used in scan mode in which case
            // there is only 1 publisher with ID 0
            _last_seq_num[0] = 0;
            _beginTime = now;
            return;
        }

        // case where not running a scan
        if (message.size != lastDataLength) {
            packetsReceived = 0;
            bytesReceived = 0;
            missingPackets = 0;

            for (int i=0; i<_numPublishers; i++) {
                _last_seq_num[i] = 0;
            }

            _beginTime = PerfTest.getTimeUsec();

            if (PerfTest.printIntervals) {
                System.out.println(
                    "\n\n********** New data length is " +
                    (message.size + PerfTest.OVERHEAD_BYTES));
                System.out.flush();
            }
        }

        lastDataLength = message.size;
        ++packetsReceived;
        bytesReceived += (message.size + PerfTest.OVERHEAD_BYTES);

        // detect missing packets
        if (_last_seq_num[message.entity_id] == 0) {
            _last_seq_num[message.entity_id] = message.seq_num;
        } else {
            if (message.seq_num != ++_last_seq_num[message.entity_id]) {
                // only track if skipped, might have restarted pub
                if (message.seq_num > _last_seq_num[message.entity_id]) {
                    missingPackets +=
                        message.seq_num - _last_seq_num[message.entity_id];
                }
                _last_seq_num[message.entity_id] = message.seq_num;
            }
        }
    }

}

// ===========================================================================
// End of $Id: ThroughputListener.java,v 1.1.2.1 2014/04/01 11:56:55 juanjo Exp $
