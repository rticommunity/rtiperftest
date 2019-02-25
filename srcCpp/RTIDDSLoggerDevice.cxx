/*
 * (c) 2005-2018  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */

#include "RTIDDSLoggerDevice.h"

RTIDDSLoggerDevice::RTIDDSLoggerDevice() : _shmemErrors(false) {}

void RTIDDSLoggerDevice::write(const NDDS_Config_LogMessage *message)
{
    if (message && !_shmemErrors) {
        if (message->level == NDDS_CONFIG_LOG_LEVEL_ERROR) {
            if (std::string(message->text).find(
                    NDDS_TRANSPORT_LOG_SHMEM_FAILED_TO_INIT_RESOURCE)
                    != std::string::npos) {
                _shmemErrors = true;
            }
        }
        if (!_shmemErrors) {
            printf("%s", message->text);
        }
    }
}

bool RTIDDSLoggerDevice::checkShmemErrors()
{
    return _shmemErrors;
}

void RTIDDSLoggerDevice::close()
{

}

