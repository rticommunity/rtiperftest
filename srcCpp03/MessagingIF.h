/*
 * Copyright 2016
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __MESSAGINGIF_H__
#define __MESSAGINGIF_H__

#include <dds/dds.hpp>

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

    static const int MAX_SYNCHRONOUS_SIZE = 63000;
    static const int MAX_DATA_SIZE = 131072;
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
    virtual bool Send(TestMessage &message) = 0;
    virtual void Flush() = 0;
    virtual void waitForPingResponse() {
	// Implementation required only if
	// support for LatencyTest is desired.
	// The implementation may consist of just
	// a binary semaphore TAKE operation
    };
    virtual void waitForPingResponse(int timeout) {
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

    virtual IMessagingWriter *CreateWriter(const std::string topic_name) = 0;

    // Pass null for callback if using IMessagingSubscriber.ReceiveMessage()
    // to get data
    virtual IMessagingReader *CreateReader(const std::string topic_name, IMessagingCB *callback) = 0;
};


#endif // __MESSAGINGIF_H__
