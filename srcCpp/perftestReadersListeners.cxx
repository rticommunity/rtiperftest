/*
 * (c) 2005-2018  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */

#include "perftestReadersListeners.h"


/* ----------------------------------------------------------------*/
/* ---------------------- ThroughputListener ----------------------*/

ThroughputListener::ThroughputListener(
        IMessagingWriter *writer,
        IMessagingReader *reader,
        bool UseCft,
        int numPublishers)
        : packetsReceived(0),
          bytesReceived(0),
          missingPackets(0),
          lastDataLength(-1),
          changeSize(false),
          intervalDataLength(-1),
          intervalPacketsReceived(0),
          intervalBytesReceived(0),
          intervalMissingPackets(0),
          intervalTime(0),
          beginTime(0),
          missingPacketsPercent(0.0),
          _reader(reader),
          _writer(writer),
          _numPublishers(numPublishers),
          _useCft(UseCft)
{
    end_test = false;
    _lastSeqNum = new unsigned long[numPublishers];
    for (int i = 0; i < numPublishers; i++) {
        _lastSeqNum[i] = 0;
    }
}

ThroughputListener::~ThroughputListener()
{
    if (_lastSeqNum != NULL) {
        delete[] _lastSeqNum;
    }
}

IMessagingReader *ThroughputListener::get_reader()
{
    return _reader;
}

void ThroughputListener::ProcessMessage(TestMessage &message)
{
    if (message.entity_id >= _numPublishers || message.entity_id < 0) {
        printf("ProcessMessage: message content no valid. "
                "message.entity_id out of bounds\n");
        return;
    }
    // Check for test initialization messages
    if (message.size == perftest_cpp::INITIALIZE_SIZE) {
        _writer->Send(message);
        _writer->Flush();
        return;
    } else if (message.size == perftest_cpp::FINISHED_SIZE) {
        /*
         * PERFTEST-97
         * We check the entity_id of the publisher to see if it has already
         * send a FINISHED_SIZE message. If he has we ignore any new one.
         * Else, we add it to a vector. Once that vector contains all the
         * ids of the publishers the subscriber is suppose to know, that
         * means that all the publishers have finished sending data samples,
         * so it is time to finish the subscriber.
         */
        if (std::find(
                    _finishedPublishers.begin(),
                    _finishedPublishers.end(),
                    message.entity_id)
                != _finishedPublishers.end()) {
            return;
        }

        if (end_test) {
            return;
        }

        _finishedPublishers.push_back(message.entity_id);

        if (_finishedPublishers.size() >= (unsigned int) _numPublishers) {
            print_summary(message);
            end_test = true;
        }
        return;
    }

    // Send back a packet if this is a ping
    if ((message.latency_ping == perftest_cpp::_SubID)
            || (_useCft && message.latency_ping != -1)) {
        _writer->Send(message);
        _writer->Flush();
    }

    // Always check if need to reset internals
    if (message.size == perftest_cpp::LENGTH_CHANGED_SIZE) {
        print_summary(message);
        changeSize = true;
        return;
    }

    // case where not running a scan
    if (message.size != lastDataLength) {
        packetsReceived = 0;
        bytesReceived = 0;
        missingPackets = 0;

        for (int i = 0; i < _numPublishers; i++) {
            _lastSeqNum[i] = 0;
        }

        beginTime = perftest_cpp::GetTimeUsec();

        if (perftest_cpp::_PrintIntervals) {
            printf("\n\n********** New data length is %d\n",
                    message.size + perftest_cpp::OVERHEAD_BYTES);
            fflush(stdout);
        }
    }

    lastDataLength = message.size;
    ++packetsReceived;
    bytesReceived
            += (unsigned long long) (message.size + perftest_cpp::OVERHEAD_BYTES);

    if (!_useCft) {
        // detect missing packets
        if (_lastSeqNum[message.entity_id] == 0) {
            _lastSeqNum[message.entity_id] = message.seq_num;
        } else {
            if (message.seq_num != ++_lastSeqNum[message.entity_id]) {
                // only track if skipped, might have restarted pub
                if (message.seq_num > _lastSeqNum[message.entity_id]) {
                    missingPackets
                            += message.seq_num - _lastSeqNum[message.entity_id];
                }
                _lastSeqNum[message.entity_id] = message.seq_num;
            }
        }
    }
}

void ThroughputListener::print_summary(TestMessage &message)
{
    // store the info for this interval
    unsigned long long now = perftest_cpp::GetTimeUsec();

    if (intervalDataLength != lastDataLength) {
        if (!_useCft) {
            // detect missing packets
            if (message.seq_num != _lastSeqNum[message.entity_id]) {
                // only track if skipped, might have restarted pub
                if (message.seq_num > _lastSeqNum[message.entity_id]) {
                    missingPackets += message.seq_num
                            - _lastSeqNum[message.entity_id];
                }
            }
        }

        intervalTime = now - beginTime;
        intervalPacketsReceived = packetsReceived;
        intervalBytesReceived = bytesReceived;
        intervalMissingPackets = missingPackets;
        intervalDataLength = lastDataLength;
        missingPacketsPercent = 0;

        // Calculations of missing package percent
        if (intervalPacketsReceived + intervalMissingPackets != 0) {
            missingPacketsPercent = (float) ((intervalMissingPackets * 100.0)
                    / (float) (intervalPacketsReceived
                    + intervalMissingPackets));
        }

        std::string outputCpu = "";
        if (perftest_cpp::_showCpu) {
            outputCpu = cpu.get_cpu_average();
        }
        printf("Length: %5d  Packets: %8llu  Packets/s(ave): %7llu  "
                "Mbps(ave): %7.1lf  Lost: %5llu (%1.2f%%) %s\n",
                intervalDataLength + perftest_cpp::OVERHEAD_BYTES,
                intervalPacketsReceived,
                intervalPacketsReceived * 1000000 / intervalTime,
                intervalBytesReceived * 1000000.0 / intervalTime * 8.0
                        / 1000.0 / 1000.0,
                intervalMissingPackets,
                missingPacketsPercent,
                outputCpu.c_str());
        fflush(stdout);
    }

    packetsReceived = 0;
    bytesReceived = 0;
    missingPackets = 0;
    // length changed only used in scan mode in which case
    // there is only 1 publisher with ID 0
    _lastSeqNum[0] = 0;
    beginTime = now;
}


/* ------------------------------------------------------------------*/
/* ---------------------- AnnouncementListener ----------------------*/
void AnnouncementListener::ProcessMessage(TestMessage &message)
{
    /*
     * The subscriberList vector contains the list of discovered
     * subscribers.
     *
     * - If the message.size is INITIALIZE or LENGTH_CHANGED and the
     *   subscriber is not in the list, it will be added.
     * - If the message.size is FINISHED_SIZE and the
     *   subscriber is in the list, it will be removed.
     *
     * The publisher access to this list to verify:
     * - If all the subscribers are discovered or notified about the length
     *   being changed.
     * - If all the subscribers are notified that the test has finished.
     */
    if ((message.size == perftest_cpp::INITIALIZE_SIZE
            || message.size == perftest_cpp::LENGTH_CHANGED_SIZE)
            && std::find(
                    subscriberList.begin(),
                    subscriberList.end(),
                    message.entity_id)
                    == subscriberList.end()) {
        subscriberList.push_back(message.entity_id);
    } else if (message.size == perftest_cpp::FINISHED_SIZE) {
        std::vector<int>::iterator position = std::find(
                subscriberList.begin(),
                subscriberList.end(),
                message.entity_id);
        if (position != subscriberList.end()) {
            subscriberList.erase(position);
        }
    }
}


/* -------------------------------------------------------------*/
/* ---------------------- LatencyListener ----------------------*/
LatencyListener::LatencyListener(
        unsigned int numLatency,
        IMessagingReader *reader,
        IMessagingWriter *writer)
        : _latencySum(0),
          _latencySumSquare(0),
          _count(0),
          _latencyMin(perftest_cpp::LATENCY_RESET_VALUE),
          _latencyMax(0),
          _lastDataLength(0),
          _clockSkewCount(0),
          _writer(writer),
          _reader(reader)
{
    if (numLatency > 0) {
        _numLatency = numLatency;

        /*
         * PERFTEST-109
         * _numLatency can be a big number, so this "new array" could
         * easily return a bad_alloc exception (specially in embedded
         * platforms with low memory settings). Therefore we catch the
         * exception to log this specific problem and then rethrow it.
         */
        try {
            _latencyHistory = new unsigned long[_numLatency];
        } catch (const std::bad_alloc &) {
            fprintf(stderr,
                    "LatencyListener: Not able to allocate %ul "
                    "elements in _latencyHistory array",
                    _numLatency);
            throw;
        }

    } else {
        _latencyHistory = NULL;
        _numLatency = 0;
    }

    end_test = false;
}

void LatencyListener::print_summary_latency()
{
    double latency_ave;
    double latency_std;
    std::string outputCpu = "";
    if (_count == 0) {
        return;
    }

    if (_clockSkewCount != 0) {
        fprintf(stderr,
                "The following latency result may not be accurate because "
                "clock skew happens %lu times\n",
                _clockSkewCount);
        fflush(stderr);
    }

    // sort the array (in ascending order)
    std::sort(_latencyHistory, _latencyHistory + _count);
    latency_ave = (double) _latencySum / _count;
    latency_std
            = sqrt((double) _latencySumSquare / (double) _count
                    - (latency_ave * latency_ave));

    if (perftest_cpp::_showCpu) {
        outputCpu = cpu.get_cpu_average();
    }

    printf("Length: %5d  Latency: Ave %6.0lf us  Std %6.1lf us  "
            "Min %6lu us  Max %6lu us  50%% %6lu us  90%% %6lu us  99%% "
            "%6lu us  99.99%% %6lu us  99.9999%% %6lu us %s\n",
            _lastDataLength + perftest_cpp::OVERHEAD_BYTES,
            latency_ave,
            latency_std,
            _latencyMin,
            _latencyMax,
            _latencyHistory[_count * 50 / 100],
            _latencyHistory[_count * 90 / 100],
            _latencyHistory[_count * 99 / 100],
            _latencyHistory[(int) (_count * (9999.0 / 10000))],
            _latencyHistory[(int) (_count * (999999.0 / 1000000))],
            outputCpu.c_str());
    fflush(stdout);

    _latencySum = 0;
    _latencySumSquare = 0;
    _latencyMin = perftest_cpp::LATENCY_RESET_VALUE;
    _latencyMax = 0;
    _count = 0;
    _clockSkewCount = 0;

    return;
}

LatencyListener::~LatencyListener()
{
    if (_latencyHistory != NULL) {
        delete[] _latencyHistory;
    }
}

IMessagingReader *LatencyListener::get_reader()
{
    return _reader;
}

void LatencyListener::ProcessMessage(TestMessage &message)
{
    unsigned long long now, sentTime;
    unsigned long latency;
    int sec;
    unsigned int usec;
    double latency_ave;
    double latency_std;
    std::string outputCpu = "";

    now = perftest_cpp::GetTimeUsec();

    switch (message.size) {
    // Initializing message, don't process
    case perftest_cpp::INITIALIZE_SIZE:
        return;

    // Test finished message
    case perftest_cpp::FINISHED_SIZE:
        return;

    // Data length is changing size
    case perftest_cpp::LENGTH_CHANGED_SIZE:
        print_summary_latency();
        return;

    default:
        break;
    }

    if (_lastDataLength != message.size) {
        _latencySum = 0;
        _latencySumSquare = 0;
        _latencyMin = perftest_cpp::LATENCY_RESET_VALUE;
        _latencyMax = 0;
        _count = 0;
    }

    sec = message.timestamp_sec;
    usec = message.timestamp_usec;
    sentTime = ((unsigned long long) sec << 32) | (unsigned long long) usec;

    if (now >= sentTime) {
        latency = (unsigned long) (now - sentTime);

        // keep track of one-way latency
        latency /= 2;
    } else {
        fprintf(stderr,
                "Clock skew suspected: received time %llu usec, sent time "
                "%llu usec",
                now,
                sentTime);
        ++_clockSkewCount;
        return;
    }

    // store value for percentile calculations
    if (_latencyHistory != NULL) {
        if (_count >= _numLatency) {
            fprintf(stderr,
                    "Too many latency pongs received.  Do you have more "
                    "than 1 app with -pidMultiPubTest = 0 or "
                    "-sidMultiSubTest 0?\n");
            return;
        } else {
            _latencyHistory[_count] = latency;
        }
    }

    if (_latencyMin == perftest_cpp::LATENCY_RESET_VALUE) {
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
    _latencySumSquare
            += ((unsigned long long) latency
                * (unsigned long long) latency);

    // if data sized changed, print out stats and zero counters
    if (_lastDataLength != message.size) {
        _lastDataLength = message.size;

        if (perftest_cpp::_PrintIntervals) {
            printf("\n\n********** New data length is %d\n",
                    _lastDataLength + perftest_cpp::OVERHEAD_BYTES);
        }
    } else {
        if (perftest_cpp::_PrintIntervals) {
            latency_ave = (double) _latencySum / (double) _count;
            latency_std
                    = sqrt((double) _latencySumSquare / (double) _count
                            - (latency_ave * latency_ave));

            if (perftest_cpp::_showCpu) {
                outputCpu = cpu.get_cpu_instant();
            }
            printf("One way Latency: %6lu us  Ave %6.0lf us  Std %6.1lf us "
                    " Min %6lu us  Max %6lu %s\n",
                    latency,
                    latency_ave,
                    latency_std,
                    _latencyMin,
                    _latencyMax,
                    outputCpu.c_str());
        }
    }

    if (_writer != NULL) {
        _writer->notifyPingResponse();
    }
}