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
    PerftestSemaphore *sync_semaphore;

    IMessagingCB() : end_test(false), sync_semaphore(NULL)
    {
    }

    virtual ~IMessagingCB()
    {
        if (sync_semaphore != NULL) {
            PerftestSemaphore_delete(sync_semaphore);
            sync_semaphore = NULL;
        }
    }

    /* Create a semaphore if is not been created yet, and then return it */
    PerftestSemaphore *get_synchronization_semaphore()
    {
        if (sync_semaphore == NULL) {
            sync_semaphore = PerftestSemaphore_new();

            if (sync_semaphore == NULL) {
                fprintf(stderr,
                        "Fail to create a Semaphore for IMessagingCB\n");
                return NULL;
            }
        }
        return sync_semaphore;
    }

    virtual void process_message(TestMessage &message) = 0;

};

class IMessagingReader
{
  public:
    virtual ~IMessagingReader() {}

    virtual void wait_for_writers(int numPublishers) = 0;

    // only used for non-callback test
    virtual TestMessage *receive_message() = 0;

    // Unblock a receive function. Needed whe a thread is blocked receiving data
    virtual bool unblock() {return true;}

    // only used for non-callback test to cleanup
    // the thread
    virtual void shutdown() {}

    virtual unsigned int get_sample_count() {
        return 0;
    }

    virtual unsigned int get_sample_count_peak() {
        return 0;
    }

};

class IMessagingWriter
{
  public:
    virtual ~IMessagingWriter() {}
    virtual void wait_for_readers(int numSubscribers) = 0;
    virtual bool send(const TestMessage &message, bool isCftWildCardKey = false) = 0;
    virtual void flush() = 0;
    virtual bool wait_for_ping_response() {
        // Implementation required only if
        // support for LatencyTest is desired.
        // The implementation may consist of just
        // a binary semaphore TAKE operation
        return true;
    };
    virtual bool wait_for_ping_response(int /*timeout*/) {
        // Implementation required only if
        // support for LatencyTest is desired.
        // The implementation may consist of just
        // a binary semaphore TAKE time out operation
        return true;
    };
    virtual bool notify_ping_response() {
        // Implementation required only if
        // support for LatencyTest is desired.
        // The implementation may consist of just
        // a binary semaphore GIVE operation
        return true;
    };
    virtual unsigned int get_pulled_sample_count() {
        return 0;
    };
    virtual unsigned int get_sample_count() {
        return 0;
    };
    virtual unsigned int get_sample_count_peak() {
        return 0;
    };
    virtual void wait_for_ack(int /*sec*/, unsigned int /*nsec*/) {
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

    virtual bool initialize(ParameterManager &PM, perftest_cpp *parent) = 0;

    virtual std::string print_configuration() = 0;

    virtual void shutdown() = 0;

    /*
     * Get an estimation of the minimum number of samples that need to be send
     * before starting the test to ensure that most memory allocations will be
     * done in the subscriber side (when sending a burst of that data).
     */
    virtual unsigned long get_initial_burst_size() = 0;

    virtual IMessagingWriter *create_writer(const char *topic_name) = 0;

    /*
     * Pass null for callback if using IMessagingReader.receive_message()
     * to get data
     */
    virtual IMessagingReader *create_reader(const char *topic_name, IMessagingCB *callback) = 0;

    /* Get information about witch features are supported by the medleware */
    virtual bool supports_listeners() = 0;

    /*
     * @brief This function calculates the overhead bytes added by all the
     * members on the TestData_* type, excluding the content of the sequence.
     *
     * @param size \b InOut. The size of the overhead of the data type.
     *
     * @return true if the operation was successful, otherwise false.
     */
    virtual bool get_serialized_overhead_size(unsigned int &overhead_size)
    {
        /* If the function is not defined by the middleware just return true */
        return true;
    };
};


#endif // __MESSAGINGIF_H__
