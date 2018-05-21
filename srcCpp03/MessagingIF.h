/*
 * (c) 2005-2017  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */

/*
 * Names of the topics for all the comunications, they will be used for the
 * class Perftest_cpp, RTIDDSImpl and PerftestTransport to retrieve the
 * corresponding address.
 */
#define LATENCY_TOPIC_NAME "Latency"
#define ANNOUNCEMENT_TOPIC_NAME "Announcement"
#define THROUGHPUT_TOPIC_NAME "Throughput"

#ifndef __MESSAGINGIF_H__
#define __MESSAGINGIF_H__

class TestMessage
{
  public:
    dds::core::vector<unsigned char> data;
    int          size;
    unsigned char key[4];
    int          entity_id;
    unsigned int seq_num;
    int          timestamp_sec;
    unsigned int timestamp_usec;
    int          latency_ping;

    TestMessage():
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
    virtual void waitForWriters(int numPublishers) = 0;
    // only used for non-callback test
    virtual TestMessage *ReceiveMessage() = 0;
    virtual void ReceiveAndProccess(IMessagingCB *listener) = 0;
    // only used for non-callback test to cleanup
    // the thread
    virtual void Shutdown() {}
};

class IMessagingWriter
{
  public:
    virtual ~IMessagingWriter() {}
    virtual void waitForReaders(int numSubscribers) = 0;
    virtual bool send(TestMessage &message, bool isCftWildCardKey = false) = 0;
    virtual void flush() = 0;
    virtual void waitForPingResponse() {
        // Implementation required only if
        // support for LatencyTest is desired.
        // The implementation may consist of just
        // a binary semaphore TAKE operation
    };
    virtual void waitForPingResponse(int /*timeout*/) {
        // Implementation required only if
        // support for LatencyTest is desired.
        // The implementation may consist of just
        // a binary semaphore TAKE time out operation
    };
    virtual void notifyPingResponse() {
        // Implementation required only if
        // support for LatencyTest is desired.
        // The implementation may consist of just
        // a binary semaphore GIVE operation
    };
    virtual unsigned int getPulledSampleCount() {
        return -1;
    };
    virtual void waitForAck(long /*sec*/, unsigned long /*nsec*/) {
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
    virtual unsigned int GetBatchSize() = 0;


    virtual IMessagingWriter *CreateWriter(const std::string &topic_name) = 0;

    // Pass null for callback if using IMessagingSubscriber.ReceiveMessage()
    // to get data
    virtual IMessagingReader *CreateReader(
            const std::string &topic_name,
            IMessagingCB *callback) = 0;
};


#endif // __MESSAGINGIF_H__
