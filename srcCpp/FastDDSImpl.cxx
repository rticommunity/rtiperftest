/*
 * (c) 2005-2017  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */
#include "MessagingIF.h"
#include "perftest_cpp.h"
#include "FastDDSImpl.h"

/*********************************************************
 * Shutdown
 */
template <typename T>
void FastDDSImpl<T>::Shutdown()
{
}

/*********************************************************

 * Validate and manage the parameters
 */
template <typename T>
bool FastDDSImpl<T>::validate_input()
{
    return true;
}

/*********************************************************
 * PrintConfiguration
 */
template <typename T>
std::string FastDDSImpl<T>::PrintConfiguration()
{

    std::ostringstream stringStream;

    stringStream << "Test: ";

    return stringStream.str();
}

/*********************************************************
 * FastDDSPublisher
 */

template<typename T>
class FastDDSPublisher : public IMessagingWriter
{
  protected:
    ParameterManager *_PM;

 public:
    FastDDSPublisher(
            unsigned long num_instances,
            PerftestSemaphore *pongSemaphore,
            int instancesToBeWritten,
            ParameterManager *PM)
    {
        _PM = PM;
    }

    ~FastDDSPublisher()
    {
    }

    void Shutdown()
    {
    }

    void Flush()
    {
    }

    void WaitForReaders(int numSubscribers)
    {
    }

    bool waitForPingResponse()
    {
        return true;
    }

    /* time out in milliseconds */
    bool waitForPingResponse(int timeout)
    {
        return true;
    }

    bool notifyPingResponse()
    {
        return true;
    }

    unsigned int getPulledSampleCount()
    {
        return 0;
    }

    unsigned int getSampleCount()
    {
        return 0;
    }

    unsigned int getSampleCountPeak()
    {
        return 0;
    }

    void waitForAck(int sec, unsigned int nsec) {
    }

    bool Send(const TestMessage &message, bool isCftWildCardKey)
    {
        return true;
    }
};

/*********************************************************
 * FastDDSSubscriber
 */

template <typename T>
class FastDDSSubscriber : public IMessagingReader
{
  protected:
    ParameterManager       *_PM;

    void Shutdown()
    {
    }

    void WaitForWriters(int numPublishers)
    {
        while (true) {
            PerftestClock::milliSleep(PERFTEST_DISCOVERY_TIME_MSEC);
        }
    }

  public:

    FastDDSSubscriber(ParameterManager *PM)
    {
        _PM = PM;
    }

    ~FastDDSSubscriber()
    {
        Shutdown();
    }


    bool unblock()
    {
        return true;
    }

    unsigned int getSampleCount()
    {
        return 0;
    }

    unsigned int getSampleCountPeak()
    {
        return 0;
    }

    TestMessage *ReceiveMessage()
    {
        return NULL;
    }
};

/*********************************************************
 * Initialize
 */
template <typename T>
bool FastDDSImpl<T>::Initialize(ParameterManager &PM, perftest_cpp *parent)
{
    // Assign ParameterManager
    _PM = &PM;
    return true;
}

/*********************************************************
 * GetInitializationSampleCount
 */
template <typename T>
unsigned long FastDDSImpl<T>::GetInitializationSampleCount()
{
    return 50;
}

/*********************************************************
 * CreateWriter
 */
template <typename T>
IMessagingWriter *FastDDSImpl<T>::CreateWriter(const char *topic_name)
{
    return new FastDDSPublisher<T>(
            _PM->get<long>("instances"),
            NULL,
            _PM->get<long>("writeInstance"),
            _PM);
}

/*********************************************************
 * CreateReader
 */
template <typename T>
IMessagingReader *FastDDSImpl<T>::CreateReader(
        const char *topic_name,
        IMessagingCB *callback)
{
    return new FastDDSSubscriber<T>(_PM);
}

// template class FastDDSImpl<TestDataKeyed_t>;
// template class FastDDSImpl<TestData_t>;
// template class FastDDSImpl<TestDataKeyedLarge_t>;
// template class FastDDSImpl<TestDataLarge_t>;

#if defined(RTI_WIN32) || defined(RTI_INTIME)
  #pragma warning(pop)
#endif
