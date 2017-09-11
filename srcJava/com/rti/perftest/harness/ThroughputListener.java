/*
 * (c) 2005-2017  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */

package com.rti.perftest.harness;

import java.util.ArrayList;
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
    public boolean end_test = false;
    public int  lastDataLength = -1;

    // store info for the last data set
    public int  intervalDataLength = -1;
    public long intervalPacketsReceived = 0;
    public long intervalBytesReceived = 0;
    public long intervalMissingPackets = 0;
    public long intervalTime = 0;
    public CpuMonitor CpuMonitor = new CpuMonitor();
    public boolean change_size = false;

    // -----------------------------------------------------------------------
    // Private Fields
    // -----------------------------------------------------------------------

    private IMessagingWriter _writer;
    private IMessagingReader _reader = null;
    private long _last_seq_num[];
    private long _beginTime;
    private int _numPublishers;
    private ArrayList<Integer> _finished_publishers;
    private boolean _useCft = false;



    // -----------------------------------------------------------------------
    // Public Methods
    // -----------------------------------------------------------------------

    // --- Constructors: -----------------------------------------------------

    public ThroughputListener(IMessagingWriter writer,
            boolean UseCft,
            int numPublishers) {
        _writer = writer;
        _last_seq_num = new long[numPublishers];
        _numPublishers = numPublishers;
        _finished_publishers = new ArrayList<Integer>();
        _useCft = UseCft;
    }


    public ThroughputListener(IMessagingWriter writer,
                              IMessagingReader reader,
                              boolean UseCft,
                              int numPublishers) {
        _writer = writer;
        _reader = reader;
        _last_seq_num = new long[numPublishers];
        _numPublishers = numPublishers;
        _finished_publishers = new ArrayList<Integer>();
        _useCft = UseCft;
    }


    // -----------------------------------------------------------------------

    /**
     * Used for receiving data using a thread instead of callback.
     */
    public void ReadThread() {
        TestMessage message = null;
        while (!end_test) {
            // Receive message should block until a message is received
            message = _reader.receiveMessage();
            if (message != null) {
                processMessage(message);
            }
        }
    }
    
    
    // --- From IMessagingCB: ------------------------------------------------

    public void processMessage(TestMessage message) {

        if (message.entity_id >= _numPublishers ||
            message.entity_id < 0) {
            System.out.println("ProcessMessage: message content no valid. message.entity_id out of bounds");
            return;
        }
        // Check for test initialization messages
        if (message.size == PerfTest.INITIALIZE_SIZE) {
            _writer.send(message, false); // Don't check return; exiting anyway
            _writer.flush();
            return;
        }
        if (message.size == PerfTest.FINISHED_SIZE) {

            /*
             * PERFTEST-97
             * We check the entity_id of the publisher to see if it has already
             * send a FINISHED_SIZE message. If he has we ignore any new one.
             * Else, we add it to a vector. Once that vector contains all the
             * ids of the publishers the subscriber is suppose to know, that
             * means that all the publishers have finished sending data samples,
             * so it is time to finish the subscriber.
             */
            if (_finished_publishers.contains(message.entity_id)) {
                return;
            }

            if (end_test) {
                return;
            }
            _finished_publishers.add(message.entity_id);
            if (_finished_publishers.size() >= _numPublishers) {
                print_summary(message);
                end_test = true;
            }
            return;
        }

        // Send back a packet if this is a ping
        if ((message.latency_ping == PerfTest.subID)  ||
                (_useCft && message.latency_ping != -1)) {
            boolean sent = _writer.send(message, false);
            _writer.flush();
            if (!sent) {
                System.out.println("*** send() failure");
                return;
            }
        }

        // reset internals
        if (message.size == PerfTest.LENGTH_CHANGED_SIZE) {
            print_summary(message);
            change_size = true;
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
        if (!_useCft) {
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

    public void print_summary(TestMessage message) {

        // store the info for this interval
        long now = PerfTest.getTimeUsec();

        if (intervalDataLength != lastDataLength) {

            if (!_useCft) {
                // detect missing packets
                if (message.seq_num != _last_seq_num[message.entity_id]) {
                    // only track if skipped, might have restarted pub
                    if (message.seq_num > _last_seq_num[message.entity_id])
                    {
                        missingPackets +=
                            message.seq_num - _last_seq_num[message.entity_id];
                    }
                }
            }

            intervalTime = now - _beginTime;
            intervalPacketsReceived = packetsReceived;
            intervalBytesReceived = bytesReceived;
            intervalMissingPackets = missingPackets;
            intervalDataLength = lastDataLength;

            String outputCpu = "";
            if (PerfTest._showCpu) {
                outputCpu = CpuMonitor.get_cpu_average();
            }
            System.out.printf("Length: %1$5d  Packets: %2$8d  Packets/s(ave): %3$7.0f  " +
                    "Mbps(ave): %4$7.1f  Lost: %5$d" + outputCpu + "\n",
                    intervalDataLength + PerfTest.OVERHEAD_BYTES,
                    intervalPacketsReceived,
                    intervalPacketsReceived * 1000000.0 / intervalTime,
                    intervalBytesReceived * 1000000.0 / intervalTime *8.0/1000.0/1000.0,
                    intervalMissingPackets
            );
            System.out.flush();
        }

        packetsReceived = 0;
        bytesReceived = 0;
        missingPackets = 0;
        // length changed only used in scan mode in which case
        // there is only 1 publisher with ID 0
        _last_seq_num[0] = 0;
        _beginTime = now;
    }

}

// ===========================================================================

