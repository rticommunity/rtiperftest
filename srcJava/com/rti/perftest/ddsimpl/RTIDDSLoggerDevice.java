/*
 * (c) 2005-2018  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */

package com.rti.perftest.ddsimpl;

import com.rti.ndds.config.LoggerDevice;
import com.rti.ndds.config.LogMessage;
import com.rti.ndds.config.LogLevel;

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
 *  a message, with two approaches, will be printed:
 *      - A link (http://community.rti.com/kb/osx510) about how to solve the issue
 *      - Explain how to set another different transport through command line parameter.
 */

 public class RTIDDSLoggerDevice implements LoggerDevice {

    /*
    *   shmem_issue: 'False' by default. In the case that SHMEM issues appear,
    *       it will be set to 'True'.
    */
    private boolean shmem_issue = false;
    private static String NDDS_TRANSPORT_LOG_SHMEM_FAILED_TO_INIT_RESOURCE =
            "NDDS_Transport_Shmem_create_recvresource_rrEA:failed to initialize shared memory resource segment for key";

    /*
    *   @brief This function is the constructor of our internal logging device.
    */
    public RTIDDSLoggerDevice()
    {
    	this.shmem_issue = false;
    }

    /*
    *   @brief This function is used to filter the log messages and write them
    *       through the logger device.
    *       shmem_issue will be set to 'True' if the log message is the known SHMEM issue.
    *   @param message \b In. Message to log.
    */
    public void write(LogMessage message)
    {
        if (!shmem_issue) {
            if (message.level == LogLevel.NDDS_CONFIG_LOG_LEVEL_ERROR) {
                if (message.text.contains(NDDS_TRANSPORT_LOG_SHMEM_FAILED_TO_INIT_RESOURCE)) {
                    shmem_issue = true;
                }
            }
            if (!shmem_issue) {
                System.out.print(message.text);
            }
        }
    }

    /*
    *   @brief Close the logging device.
    */
    public void close()
    {

    }

    /*
    *   @brief Get the value of the variable shmem_issue.
    *   @return shmem_issue
    */
    public boolean get_shmem_issue()
    {
       return this.shmem_issue;
    }
}

// ===========================================================================
