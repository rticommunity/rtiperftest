/*
 * (c) 2005-2020 Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */

#ifndef INFRASTRUCTURE_FASTDDS_H_
#define INFRASTRUCTURE_FASTDDS_H_

/*
 * For the time being, the implementation of FastDDS will rely on the 
 * infrastructure for Micro, still, we will leave this file in place so we can
 * replace it at some point.
 */

#ifdef PERFTEST_EPROSIMA_FASTDDS

#include "PerftestTransport.h"

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/rtps/transport/shared_mem/SharedMemTransportDescriptor.h>
#include <fastdds/rtps/transport/UDPv4TransportDescriptor.h>
#include <fastrtps/log/Log.h>

#ifdef RTI_WIN32
  #include "windows.h"
#endif

#include <sstream>

/********************************************************************/
/* Transport Related functions */

bool PerftestConfigureTransport(
        PerftestTransport &transport,
        eprosima::fastdds::dds::DomainParticipantQos &qos,
        ParameterManager *_PM);

/********************************************************************/


#endif // PERFTEST_EPROSIMA_FASTDDS
#endif /* INFRASTRUCTURE_FASTDDS_H_ */
