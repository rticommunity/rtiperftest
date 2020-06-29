/*
 * (c) 2005-2020  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */

#ifndef __FASTDDSIMPL_H__
#define __FASTDDSIMPL_H__

#ifdef EPROSIMA_PERF_FASTDDS

#include <stdexcept> // This header is part of the error handling library.
#include <string>
#include <algorithm>
#include <map>
#include "MessagingIF.h"
#include "PerftestTransport.h"
#include "Infrastructure_common.h"

#define RTIPERFTEST_MAX_PEERS 1024

/* Forward declaration of perftest_cpp to avoid circular dependencies */
class perftest_cpp;

template <typename T>
class FastDDSImpl : public IMessaging
{
public:

    FastDDSImpl() :
        _parent(NULL)
    {
    }

    ~FastDDSImpl()
    {
        Shutdown();
    }

    bool validate_input();

    std::string PrintConfiguration();

    bool Initialize(ParameterManager &PM, perftest_cpp *parent);

    void Shutdown();

    unsigned long GetInitializationSampleCount();

    IMessagingWriter *CreateWriter(const char *topic_name);

    /*
     * Pass null for callback if using IMessagingSubscriber.ReceiveMessage()
     * to get data
     */
    IMessagingReader *CreateReader(const char *topic_name, IMessagingCB *callback);

    bool supports_listener()
    {
        return true;
    };

    bool supports_discovery()
    {
        return true;
    };

protected:
    ParameterManager            *_PM;
    perftest_cpp                *_parent;
};

#endif // EPROSIMA_PERF_FASTDDS
#endif // __FASTDDSIMPL_H__
