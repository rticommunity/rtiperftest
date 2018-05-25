/*
 * (c) 2005-2017  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */

#include "RTIDDSLoggerDevice.h"

RTIDDSLoggerDevice::RTIDDSLoggerDevice() : shmem_issue(false) {}

void RTIDDSLoggerDevice::write(const NDDS_Config_LogMessage *message)
{
    if (message && !shmem_issue) {
        if (message->level == NDDS_CONFIG_LOG_LEVEL_ERROR) {
            if (std::string(message->text).find(NDDS_TRANSPORT_LOG_SHMEM_FAILED_TO_INIT_RESOURCE) != std::string::npos) {
                shmem_issue = true;
            }
        }
        if (!shmem_issue) {
            printf("%s\n", message->text);
        }
    }
}

bool RTIDDSLoggerDevice::get_shmem_issue()
{
    return shmem_issue;
}

void RTIDDSLoggerDevice::close()
{

}

