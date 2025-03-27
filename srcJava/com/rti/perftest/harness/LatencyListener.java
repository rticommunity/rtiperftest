/*
 * (c) 2005-2017  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */

package com.rti.perftest.harness;

import static java.lang.Math.abs;
import static java.lang.Math.sqrt;
import java.util.Arrays;

import com.rti.perftest.harness.PerfTest;
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
    private int   _latencyMin = PerfTest.LATENCY_RESET_VALUE;
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

    private void resetLatencyCounters() {
        _latencySum = 0;
        _latencySumSquare = 0;
        _latencyMin = PerfTest.LATENCY_RESET_VALUE;
        _latencyMax = 0;
        _count = 0;
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

        if (_latencyMin == PerfTest.LATENCY_RESET_VALUE) {
            _latencyMin = latency;
            _latencyMax = latency;
        } else {
            if (latency < _latencyMin) {
                _latencyMin = latency;
            } else if (latency > _latencyMax) {
                _latencyMax = latency;
            }
        }

        ++_count;
        _latencySum += latency;
        _latencySumSquare += ((long)latency * (long)latency);

        double outputCpu = 0;
        if (PerfTest._showCpu) {
            outputCpu = CpuMonitor.get_cpu_instant();
        }

        // if data size changed, print out stats and zero counters
        if (_lastDataLength != message.size) {
            _lastDataLength = message.size;
            PerfTest._printer.set_data_length(message.size +
                    PerfTest.OVERHEAD_BYTES);
            PerfTest._printer.print_latency_header();
            resetLatencyCounters();

        } else if (PerfTest.printIntervals) {
            double latency_ave = (double)_latencySum / (double)_count;

            double latency_std = sqrt(
                (double)_latencySumSquare / (double)_count - (latency_ave * latency_ave));

            PerfTest._printer.print_latency_interval(
                    latency,
                    latency_ave,
                    latency_std,
                    _latencyMin,
                    _latencyMax,
                    outputCpu);
        }
        if (_writer != null) {
            _writer.notifyPingResponse();
        }
    }

    public void print_summary_latency() {
        print_summary_latency(false);
    }

    public void print_summary_latency(boolean endTest) {
        if (_count == 0) {
            if (endTest) {
                System.out.printf(
                        "\nNo Pong samples have been received in the Publisher side.\n"
                        + "If you are interested in latency results, you might need to\n"
                        + "increase the Pong frequency (using the -latencyCount option).\n"
                        + "Alternatively you can increase the number of samples sent\n"
                        + "(-numIter) or the time for the test (-executionTime). If you\n"
                        + "are sending large data, make sure you set the data size (-datalen)\n"
                        + "in the Subscriber side.\n\n");
            }
            return;
        }

        if (_clockSkewCount != 0) {
            System.out.printf(
                "The following latency result may not be accurate because clock skew happens %1$d times\n",
                _clockSkewCount);
        }

        // sort the array (in ascending order)
        Arrays.sort(_latencyHistory, 0, (int)_count);
        double latency_ave = _latencySum / (double)_count;
        double latency_std = sqrt(
                abs(_latencySumSquare / (double)_count -
                        (latency_ave * latency_ave)));
        double outputCpu = 0;
        if (PerfTest._showCpu) {
            outputCpu = CpuMonitor.get_cpu_average();
        }
        PerfTest._printer.print_latency_summary(
                latency_ave,
                latency_std,
                _latencyMin,
                _latencyMax,
                _latencyHistory,
                _count,
                outputCpu);

        System.out.flush();
        _latencySum = 0;
        _latencySumSquare = 0;
        _latencyMin = PerfTest.LATENCY_RESET_VALUE;
        _latencyMax = 0;
        _count = 0;
        _clockSkewCount = 0;
    }

}

// ===========================================================================
// End of $Id: LatencyListener.java,v 1.4 2014/06/27 15:23:02 juanjo Exp $
