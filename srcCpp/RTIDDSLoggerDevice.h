#ifndef __RTIDDSLOGGERDEVICE_H__
#define __RTIDDSLOGGERDEVICE_H__

/*
 * (c) 2005-2018  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */

#include <string>
#include <stdio.h>
#include <stdlib.h>
#include "ndds/ndds_cpp.h"

/*
 *  After setting "UPDv4 | SHMEM" as default transport_builtin mask. The user may
 *  get the following error running a default scenario:
 *
 *      [D0001|ENABLE]NDDS_Transport_Shmem_create_recvresource_rrEA:failed to initialize shared memory resource segment for key 0x40894a
 *      [D0001|ENABLE]NDDS_Transport_Shmem_create_recvresource_rrEA:failed to initialize shared memory resource segment for key 0x40894c
 *      [D0001|ENABLE]DDS_DomainParticipantPresentation_reserve_participant_index_entryports:!enable reserve participant index
 *      [D0001|ENABLE]DDS_DomainParticipant_reserve_participant_index_entryports:Unusable shared memory transport. For a more in-depth explanation of the possible problem and solution, please visit http://community.rti.com/kb/osx510.
 *      [D0001|ENABLE]DDS_DomainParticipant_enableI:Automatic participant index failed to initialize. PLEASE VERIFY CONSISTENT TRANSPORT / DISCOVERY CONFIGURATION.
 *      [NOTE: If the participant is running on a machine where the network interfaces can change, you should manually set wire protocol's participant id]
 *      DDSDomainParticipant_impl::createI:ERROR: Failed to auto-enable entity.
 *
 *  By using our own implementation of LoggerDevice, we can filter those errors.
 *  In the case that those errors appear, the execution will be stopped and
 *  a message will be printed showing:
 *      - A link (http://community.rti.com/kb/osx510) about how to solve the issue
 *      - How to setup a different transport via command-line parameters.
 */

const std::string NDDS_TRANSPORT_LOG_SHMEM_FAILED_TO_INIT_RESOURCE =
        "NDDS_Transport_Shmem_create_recvresource_rrEA:failed to initialize shared memory resource segment for key";

class RTIDDSLoggerDevice : public NDDSConfigLoggerDevice {

  private:
    /*
     *   _shmemErrors: 'False' by default. In the case that SHMEM issues appear,
     *       it will be set to 'True'.
     */
    bool _shmemErrors;
  public:

    /*
     *   @brief This function is the constructor of our internal logging device.
     */
    RTIDDSLoggerDevice();

    /*
     *   @brief This function is used to filter the log messages and write them
     *       through the logger device.
     *       _shmemErrors will be set to 'True' if the log message is the known SHMEM issue.
     *   @param message \b In. Message to log.
     */
    void write(const NDDS_Config_LogMessage *message);

    /*
     *   @brief Close the logging device.
     */
    void close();

    /*
     *   @brief Get the value of the variable _shmemErrors.
     *   @return _shmemErrors
     */
    bool checkShmemErrors();
};

#endif // __RTIDDSLOGGERDEVICE_H__
