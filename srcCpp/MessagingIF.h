#ifndef __MESSAGINGIF_H__
#define __MESSAGINGIF_H__

/*
 * (c) 2005-2016  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */

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

    TestMessage():
        data(NULL),
        size(0),
        entity_id(0),
        seq_num(0),
        timestamp_sec(0),
        timestamp_usec(0),
        latency_ping(0)
    {
        key[0]=0;
        key[1]=0;
        key[2]=0;
        key[3]=0;
    }

    ~TestMessage()
    {
        if (data != NULL) {
            delete data;
        }
    }
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
    virtual bool Send(const TestMessage &message) = 0;
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
    virtual unsigned int getPulledSampleCount() {
        return 0;
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

    virtual IMessagingWriter *CreateWriter(const char *topic_name) = 0;

    // Pass null for callback if using IMessagingSubscriber.ReceiveMessage()
    // to get data
    virtual IMessagingReader *CreateReader(const char *topic_name, IMessagingCB *callback) = 0;
};


#endif // __MESSAGINGIF_H__
