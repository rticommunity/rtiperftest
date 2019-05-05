/*
 * (c) 2005-2017  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */

#include <string>

/* Forward declaration of perftest_cpp to avoid circular dependencies */
class perftest_cpp;

#ifndef __MESSAGINGIF_H__
#define __MESSAGINGIF_H__

#include "ParameterManager.h"

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
    virtual bool Initialize(ParameterManager &PM, perftest_cpp *parent) = 0;
    virtual std::string PrintConfiguration() = 0;
    virtual void Shutdown() = 0;

    /*
     * Get an estimation of the minimum number of samples that need to be send
     * before starting the test to ensure that most memory allocations will be
     * done in the subscriber side (when sending a burst of that data).
     */
    virtual unsigned long GetInitializationSampleCount() = 0;


    virtual IMessagingWriter *CreateWriter(const std::string &topic_name) = 0;

    /*
     * Pass null for callback if using IMessagingReader.ReceiveMessage()
     * to get data
     */
    virtual IMessagingReader *CreateReader(
            const std::string &topic_name,
            IMessagingCB *callback) = 0;
};


#endif // __MESSAGINGIF_H__
