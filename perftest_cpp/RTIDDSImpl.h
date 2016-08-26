#ifndef __RTIDDSIMPL_H__
#define __RTIDDSIMPL_H__

/* $Id: RTIDDSImpl.h,v 1.2.2.1 2014/04/01 11:56:52 juanjo Exp $

 (c) 2005-2012  Copyright, Real-Time Innovations, Inc.  All rights reserved.    	
 Permission to modify and use for internal purposes granted.   	
 This software is provided "as is", without warranty, express or implied.

 Modification History
 --------------------
 5.1.0,19dec13,jmc PERFTEST-3 Added autothrottle and turbomode
 5.1.0,19dec13,jmc PERFTEST-2 window size in batching path and
                   domain id now is 1
 1.0a,07apr10,acr Added latency test
 1.0a,10mar10,gn  Added tcp feature
 1.0a,26may09,fcs Added _reader
 1.0a,08may09,jsr Fixed default profile name
 1.0a,03dec08,jsr Added HearbeatPeriod and FastHeartbeatPeriod
 1.0a,02aug08,eys Added instanceHashBuckets parameter
 1.0a,09may08,ch  Support for multiple instances and durability
 1.0a,01may08,hhw Removed singleCore option.
 1.0a,26apr08,fcs Changed ANNOUNCEMENT_MULTICAST_ADDR
 1.0a,11apr08,fcs Added ANNOUNCEMENT_MULTICAST_ADDR
 1.0a,19mar08,hhw Created.
===================================================================== */

#include "MessagingIF.h"
#include "testSupport.h"

class RTIDDSImpl : public IMessaging
{
  public:

    RTIDDSImpl()
    {
        _SendQueueSize = 50;
        _DataLen = 100;
        _DomainID = 1;
        _Nic = "";
        _ProfileFile = "perftest.xml";
        _ConfigFile = "perftest.ini";
        _AutoThrottle = false;
        _TurboMode = false;
        _IsReliable = true;
        _IsMulticast = false;
        _BatchSize = 0;
        _InstanceCount = 1;
        _InstanceHashBuckets = -1;
        _Durability = DDS_VOLATILE_DURABILITY_QOS;
        _DirectCommunication = true;
        _KeepDurationUsec = 1000;
        _UsePositiveAcks = true;
        _UseSharedMemory = false;
	    _LatencyTest = false;
        _UseTcpOnly = false;
        _IsDebug = false;
		
        _HeartbeatPeriod.sec = 0;
        _HeartbeatPeriod.nanosec = 0;
        _FastHeartbeatPeriod.sec = 0;
        _FastHeartbeatPeriod.nanosec = 0;

        _WaitsetEventCount = 5;
        _WaitsetDelayUsec = 100;
        THROUGHPUT_MULTICAST_ADDR = "239.255.1.1";
        LATENCY_MULTICAST_ADDR = "239.255.1.2";
        ANNOUNCEMENT_MULTICAST_ADDR = "239.255.1.100";
        _ProfileLibraryName = "PerftestQosLibrary";

        _factory = NULL;
        _participant = NULL;
        _subscriber = NULL;
        _publisher = NULL;
        _reader = NULL;
        _typename = TestData_tTypeSupport::get_type_name();

	_pongSemaphore = NULL;
    }

    ~RTIDDSImpl() 
    {
        Shutdown();
    }

    void PrintCmdLineHelp();

    bool ParseConfig(int argc, char *argv[]);

    bool Initialize(int argc, char *argv[]);

    void Shutdown();

    int GetBatchSize() { return _BatchSize; }

    int GetMaxBinDataSize() { return MAX_BINDATA_SIZE; }

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
    const char  *_ConfigFile;
    bool		 _TurboMode;
    bool 		 _AutoThrottle;
    bool         _IsReliable;
    bool         _IsMulticast;
    int          _BatchSize;
    int          _InstanceCount;
    int          _InstanceHashBuckets;
    int          _Durability;
    bool         _DirectCommunication;
    unsigned int _KeepDurationUsec;
    bool         _UsePositiveAcks;
    bool         _UseSharedMemory;
    bool	 _LatencyTest;
    bool         _UseTcpOnly;
    bool         _IsDebug;
	
    DDS_Duration_t   _HeartbeatPeriod;
    DDS_Duration_t   _FastHeartbeatPeriod;

    const char          *THROUGHPUT_MULTICAST_ADDR;
    const char          *LATENCY_MULTICAST_ADDR;
    const char          *ANNOUNCEMENT_MULTICAST_ADDR;
    const char          *_ProfileLibraryName;

    DDSDomainParticipantFactory *_factory;
    DDSDomainParticipant        *_participant;
    DDSSubscriber               *_subscriber;
    DDSPublisher                *_publisher;
    DDSDataReader               *_reader;
    const char                  *_typename;

    RTIOsapiSemaphore		*_pongSemaphore;

  public:

    static int          _WaitsetEventCount;
    static unsigned int _WaitsetDelayUsec;
};


#endif // __RTIDDSIMPL_H__

