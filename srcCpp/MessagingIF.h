#ifndef __MESSAGINGIF_H__
#define __MESSAGINGIF_H__

/*
 * (c) 2005-2017  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */

#include <string>
#include "ParameterManager.h"
#include "Infrastructure_common.h"

/* Forward declaration of perftest_cpp to avoid circular dependencies */
class perftest_cpp;

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

    TestMessage() :
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

};

class IMessagingCB
{
  public:
    bool  end_test;
    PerftestSemaphore *syncSemaphore;

  public:
    IMessagingCB() : end_test(false), syncSemaphore(NULL){}

    virtual ~IMessagingCB() {
        if (syncSemaphore != NULL) {
            PerftestSemaphore_delete(syncSemaphore);
            syncSemaphore = NULL;
        }
    }

    /* Create a semaphore if is not been created yet, and then return it */
    PerftestSemaphore *get_synchronization_semaphore()
    {
        if (syncSemaphore == NULL) {
            syncSemaphore = PerftestSemaphore_new();

            if (syncSemaphore == NULL) {
                fprintf(stderr,
                        "Fail to create a Semaphore for IMessagingCB\n");
                return NULL;
            }
        }
        return syncSemaphore;
    }

    virtual void ProcessMessage(TestMessage &message) = 0;

};

class IMessagingReader
{
  public:
    virtual ~IMessagingReader() {}

    virtual void WaitForWriters(int numPublishers) = 0;

    // only used for non-callback test
    virtual TestMessage *ReceiveMessage() = 0;

    // Unblock a receive function. Needed whe a thread is blocked receiving data
    virtual bool unblock() {return true;}

    // only used for non-callback test to cleanup
    // the thread
    virtual void Shutdown() {}

    virtual unsigned int getSampleCount() {
        return 0;
    }

    virtual unsigned int getSampleCountPeak() {
        return 0;
    }

};

class IMessagingWriter
{
  public:
    virtual ~IMessagingWriter() {}
    virtual void WaitForReaders(int numSubscribers) = 0;
    virtual bool Send(const TestMessage &message, bool isCftWildCardKey = false) = 0;
    virtual void Flush() = 0;
    virtual bool waitForPingResponse() {
        // Implementation required only if
        // support for LatencyTest is desired.
        // The implementation may consist of just
        // a binary semaphore TAKE operation
        return true;
    };
    virtual bool waitForPingResponse(int /*timeout*/) {
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
    virtual unsigned int getSampleCount() {
        return 0;
    };
    virtual unsigned int getSampleCountPeak() {
        return 0;
    };
    virtual void waitForAck(int /*sec*/, unsigned int /*nsec*/) {
    };
#ifdef RTI_CUSTOM_TYPE
  private:
    virtual bool is_sentinel_size(int size) {
        return 0;
    };
    virtual bool get_serialize_size_custom_type_data(unsigned int &size) {
        return 0;
    };
#endif
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

    virtual IMessagingWriter *CreateWriter(const char *topic_name) = 0;

    /*
     * Pass null for callback if using IMessagingReader.ReceiveMessage()
     * to get data
     */
    virtual IMessagingReader *CreateReader(const char *topic_name, IMessagingCB *callback) = 0;

    /* Get information about witch features are supported by the medleware */
    virtual bool supports_listener() = 0;
    virtual bool supports_discovery() = 0;

};


#endif // __MESSAGINGIF_H__
