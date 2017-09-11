/*
 * (c) 2005-2017  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */

package com.rti.perftest.harness;

import static java.lang.Math.abs;
import static java.lang.Math.sqrt;
import java.util.Arrays;

import com.rti.perftest.IMessagingCB;
import com.rti.perftest.IMessagingReader;
import com.rti.perftest.IMessagingWriter;
import com.rti.perftest.TestMessage;

//===========================================================================

/**
 * Data listener for the Publisher side.
 *
 * Receives latency ping from Subscriber and does
 * round trip latency calculations
 */
/*package*/ final class LatencyListener implements IMessagingCB {
    // -----------------------------------------------------------------------
    // Private Fields
    // -----------------------------------------------------------------------

    private long  _latencySum = 0;
    private long  _latencySumSquare = 0;
    private long  _count = 0;
    private int   _latencyMin = 0;
    private int   _latencyMax = 0;
    private int   _lastDataLength = 0;
    private int[] _latencyHistory = null;
    private int   _clockSkewCount = 0;
    private int   _num_latency = 0;

    private IMessagingReader _reader = null;
    private IMessagingWriter _writer = null;    
    private CpuMonitor CpuMonitor = new CpuMonitor();


    // -----------------------------------------------------------------------
    // Public Methods
    // -----------------------------------------------------------------------

    public boolean end_test = false;

    // --- Constructors: -----------------------------------------------------

    public LatencyListener(int num_latency, IMessagingWriter writer) {
        if (num_latency > 0) {
            _latencyHistory = new int[num_latency];
            _num_latency = num_latency;
        }
        _writer = writer;
    }

    public LatencyListener(IMessagingReader reader, IMessagingWriter writer, int num_latency) {
        if (num_latency > 0) {
            _latencyHistory = new int[num_latency];
            _num_latency = num_latency;
        }
        _reader = reader;
        _writer = writer;
    }


    // -----------------------------------------------------------------------

    public void readThread() {
        TestMessage message;
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
        long now = PerfTest.getTimeUsec();      // may be negative!

        switch (message.size) {
            // Initializing message, don't process
            case PerfTest.INITIALIZE_SIZE:
                return;

            // Test finished message
            case PerfTest.FINISHED_SIZE:
                return;

            // Data length is changing size
            case PerfTest.LENGTH_CHANGED_SIZE:
                print_summary_latency();
                return;

            default:
                break;
        }

        /* Note that we're not really treating 'sentTime' as a time in
         * microseconds: we're just taking 2 32-bit numbers and
         * assembling them into a 64-bit number.
         */
        long sec  = message.timestamp_sec;
        long usec = message.timestamp_usec & (~0L >>> 32);
        long sentTime = (sec << 32) | usec;     // may be negative!
        int  latency;

        if (now >= sentTime) {
            latency = (int)(now - sentTime);

            // keep track of one-way latency
            latency /= 2;
        } else {
            System.out.printf("Clock skew suspected: received time %1$d usec, sent time %2$d usec",
                              now, sentTime);
            ++_clockSkewCount;
            return;
        }

        // store value for percentile calculations
        if (_latencyHistory != null) {
            if (_count >= _num_latency)
            {
                System.out.print("Too many latency pongs received.  Do you have more than 1 app with -pidMultiPubTest = 0 or -sidMultiSubTest?\n");
                return;
            }
            else 
            {
                _latencyHistory[(int)_count] = latency;
            }
        }

        if (_latencyMin == 0) {
            _latencyMin = latency;
            _latencyMax = latency;
        } else {
            if (latency < _latencyMin) {
                _latencyMin = latency;
            }
            if (latency > _latencyMax) {
                _latencyMax = latency;
            }
        }

        ++_count;
        _latencySum += latency;
        _latencySumSquare += ((long)latency * (long)latency);

        String outputCpu = "";
        if (PerfTest._showCpu) {
            outputCpu = CpuMonitor.get_cpu_instant();
        }

        // if data sized changed
        if (_lastDataLength != message.size) {
            _lastDataLength = message.size;

            if (_lastDataLength != 0 && PerfTest.printIntervals) {
                System.out.printf(
                        "\n\n********** New data length is %1$d\n",
                        _lastDataLength + PerfTest.OVERHEAD_BYTES);
            }
        } else if (PerfTest.printIntervals) {
            double latency_ave = (double)_latencySum / (double)_count;

            double latency_std = sqrt(
                (double)_latencySumSquare / (double)_count - (latency_ave * latency_ave));

            System.out.printf(
                "One-Way Latency: %1$6d us  Ave %2$6.0f us  Std %3$6.1f us  Min %4$6d us  Max %5$6d" + outputCpu + "\n",
                latency,
                latency_ave,
                latency_std,
                _latencyMin,
                _latencyMax
            );
        }
        if (_writer != null) {
            _writer.notifyPingResponse();
        }
    }

    public void print_summary_latency(){
        if (_count == 0) {
            return;
        }

        if (_clockSkewCount != 0) {
            System.out.printf("The following latency result may not be accurate because clock skew happens %1$d times\n",
                    _clockSkewCount);
        }

        // sort the array (in ascending order)
        Arrays.sort(_latencyHistory, 0, (int)_count);
        double latency_ave = _latencySum / (double)_count;
        // TODO: This std dev calculation isn't correct!
        double latency_std = sqrt(
                abs(_latencySumSquare / (double)_count -
                        (latency_ave * latency_ave)));
        String outputCpu = "";
        if (PerfTest._showCpu) {
            outputCpu = CpuMonitor.get_cpu_average();
        }

        System.out.printf(
                "Length: %1$5d  Latency: Ave %2$6.0f us  Std %3$6.1f us  " +
                "Min %4$6d us  Max %5$6d us  50%% %6$6d us  90%% %7$6d us  99%% %8$6d us  99.99%% %9$6d us  99.9999%% %10$6d us" + outputCpu + "\n",
                _lastDataLength + PerfTest.OVERHEAD_BYTES,
                latency_ave,
                latency_std,
                _latencyMin,
                _latencyMax,
                _latencyHistory[(int)(_count * 50 / (double)100)],
                _latencyHistory[(int)(_count * 90 / (double)100)],
                _latencyHistory[(int)(_count * 99 / (double)100)],
                _latencyHistory[(int)(_count * (9999.0 / (double)10000))],
                _latencyHistory[(int)(_count * (999999.0 / (double)1000000))]
        );
        System.out.flush();
        _latencySum = 0;
        _latencySumSquare = 0;
        _latencyMin = 0;
        _latencyMax = 0;
        _count = 0;
        _clockSkewCount = 0;
    }

}

// ===========================================================================
// End of $Id: LatencyListener.java,v 1.4 2014/06/27 15:23:02 juanjo Exp $
