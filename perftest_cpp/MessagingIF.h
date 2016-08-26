#ifndef __MESSAGINGIF_H__
#define __MESSAGINGIF_H__

/* $Id: MessagingIF.h,v 1.1.2.1 2014/04/01 11:56:51 juanjo Exp $

 (c) 2005-2012  Copyright, Real-Time Innovations, Inc.  All rights reserved.    	
 Permission to modify and use for internal purposes granted.   	
 This software is provided "as is", without warranty, express or implied.

 Modification History
 --------------------
 1.0a,13jul10,jsr Added waitForPingResponse with timeout
 1.0a,02may08,hhw Added GetBatchSize().
 1.0a,15apr08,fcs Added entity_id and WaitForWriters
 1.0a,19mar08,hhw Created.
===================================================================== */



class TestMessage
{
  public:
    char         *data;
    int          size;
    unsigned char key[4];
    int          entity_id;
    unsigned int seq_num;
    int          timestamp_sec;
    unsigned int timestamp_usec;
    int          latency_ping;

    static const int MAX_DATA_SIZE = 63000;
};

class IMessagingCB
{
  public:
    bool  end_test;

  public: 
    virtual ~IMessagingCB() {}
    virtual void ProcessMessage(TestMessage &message) = 0;
};

class IMessagingReader
{
  public:
    virtual ~IMessagingReader() {}

    virtual void WaitForWriters(int numPublishers) = 0;

    // only used for non-callback test
    virtual TestMessage *ReceiveMessage() = 0;

    // only used for non-callback test to cleanup  
    // the thread
    virtual void Shutdown() {}
};

class IMessagingWriter
{
  public:
    virtual ~IMessagingWriter() {}
    virtual void WaitForReaders(int numSubscribers) = 0;
    virtual bool Send(TestMessage &message) = 0;
    virtual void Flush() = 0;
    
    virtual bool waitForPingResponse() {
	// Implementation required only if
	// support for LatencyTest is desired.
	// The implementation may consist of just
	// a binary semaphore TAKE operation
	return true;
    };
    virtual bool waitForPingResponse(int timeout) {
        // Implementation required only if
	// support for LatencyTest is desired.
	// The implementation may consist of just
	// a binary semaphore TAKE time out operation
	return true;
    };
    virtual bool notifyPingResponse() {
	// Implementation required only if
	// support for LatencyTest is desired.
	// The implementation may consist of just
	// a binary semaphore GIVE operation
	return true;
    };
};

class IMessaging
{
  public:
    virtual ~IMessaging() {}
    virtual bool Initialize(int argc, char *argv[]) = 0;

    virtual void PrintCmdLineHelp() = 0;

    virtual void Shutdown() = 0;

    // if the implementation supports batching and the test scenario is
    // using batching, this function should return the size of the batch
    // in bytes
    virtual int GetBatchSize() = 0;

    // Used only for scan mode.
    // The maximum size of a message's binary payload. If the size
    // exceeds this during a scan, the test will stop.
    virtual int GetMaxBinDataSize() = 0;

    virtual IMessagingWriter *CreateWriter(const char *topic_name) = 0;
    
    // Pass null for callback if using IMessagingSubscriber.ReceiveMessage()
    // to get data
    virtual IMessagingReader *CreateReader(const char *topic_name, IMessagingCB *callback) = 0;
};


#endif // __MESSAGINGIF_H__
