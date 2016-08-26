/* ===================================================================
 (c) 2005-2012  Copyright, Real-Time Innovations, Inc.  All rights reserved.
 Permission to modify and use for internal purposes granted.
 This software is provided "as is", without warranty, express or implied.

 Modification History
 --------------------
 5.2.0,21jul15,jm  PERF-53 Changes for CR-789.
 5.2.0,03nov14,jm  PERF-53 Created. Using ../perftest_cpp as template for the
                   Product behavior.
===================================================================== */

#ifndef __RTIDDSIMPL_H__
#define __RTIDDSIMPL_H__

#include <iostream>
#include <vector>
#include "test.hpp"
#include "MessagingIF.h"
#include "rti/config/Logger.hpp"
#include <dds/dds.hpp>


template <typename T>
class RTIDDSImpl : public IMessaging
{
  public:

    RTIDDSImpl();

    ~RTIDDSImpl() 
    {
        Shutdown();
    }

    void PrintCmdLineHelp();

    bool ParseConfig(int argc, char *argv[]);

    bool Initialize(int argc, char *argv[]);

    void Shutdown();

    int GetBatchSize()
    {
        return _BatchSize;
    }

    int GetMaxBinDataSize()
    {
        return MAX_BINDATA_SIZE;
    }

    IMessagingWriter *CreateWriter(const char *topic_name);
    // Pass null for callback if using IMessagingSubscriber.ReceiveMessage()
    // to get data
    IMessagingReader *CreateReader(const char *topic_name, IMessagingCB *callback);


  private:

    int          _SendQueueSize;
    int          _DataLen;
    int          _DomainID;
    const char  *_Nic;
    const char  *_ProfileFile;
    bool         _TurboMode;
    bool         _AutoThrottle;
    bool         _IsReliable;
    bool         _IsMulticast;
    int          _BatchSize;
    int          _InstanceCount;
    int          _InstanceMaxCountReader;
    int          _InstanceHashBuckets;
    int          _Durability;
    bool         _DirectCommunication;
    unsigned int _KeepDurationUsec;
    bool         _UsePositiveAcks;
    bool         _UseSharedMemory;
    bool         _LatencyTest;
    bool         _UseTcpOnly;
    bool         _IsDebug;
    bool         _isLargeData;
    bool         _isScan;
    int          _WaitsetEventCount;
    unsigned int _WaitsetDelayUsec;

    dds::core::Duration   _HeartbeatPeriod;
    dds::core::Duration   _FastHeartbeatPeriod;

    const char          *THROUGHPUT_MULTICAST_ADDR;
    const char          *LATENCY_MULTICAST_ADDR;
    const char          *ANNOUNCEMENT_MULTICAST_ADDR;
    const char          *_ProfileLibraryName;

    dds::domain::DomainParticipant _participant;
    dds::sub::Subscriber _subscriber;
    dds::pub::Publisher _publisher;
    dds::sub::DataReader<T> _reader;

    rti::core::Semaphore _pongSemaphore;

};


#endif // __RTIDDSIMPL_H__

