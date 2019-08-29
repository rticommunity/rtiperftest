/*
 * (c) 2005-2018  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */

#include "RTIDDSLoggerDevice.h"

RTIDDSLoggerDevice::RTIDDSLoggerDevice(): 
        _shmemErrors(false), 
        _zerocopyErrors(false),
        _outOfLoanedSamples(false) {
}

void RTIDDSLoggerDevice::write(const NDDS_Config_LogMessage *message)
{
    if (message) {
        if (message->level == NDDS_CONFIG_LOG_LEVEL_ERROR) {
            if (std::string(message->text).find(
                    NDDS_TRANSPORT_LOG_SHMEM_FAILED_TO_INIT_RESOURCE)
                    != std::string::npos) {
                if (!_shmemErrors) {
                    printf("%s", message->text);
                }
                _shmemErrors = true;
            } else if (std::string(message->text).find(
                    NDDS_TRANSPORT_LOG_SHMEM_SHARED_MEMORY_INSUFFICIENT)
                    != std::string::npos) {

                if (!_shmemErrors) {
                    printf("%s", message->text);
                }

                _shmemErrors = true;
            } else if (std::string(message->text).find(
                    NDDS_ZEROCOPY_NULL_OFFSET_ERROR) != std::string::npos) {

                if (!_zerocopyErrors) {
                    printf("Warning: The Publisher is reusing memory to send different samples before the original samples are processed by the subscriber, leading to inconsistent samples.\n");
                    printf("This is expected with the current perftest configuration. See more on the User manual: 22.5.1.3 Checking data consistency with Zero Copy transfer over shared memory on page 870 .  \n");
                    printf("Unconsistent samples will be reported as lost.\n");
                }

                _zerocopyErrors = true;
            } else if (
                std::string(message->text).find(
                        NDDS_OUT_OF_LOANED_SAMPLES) != std::string::npos ||
                std::string(message->text).find(
                        NDDS_OUT_OF_LOANED_SAMPLES_2) != std::string::npos) {

                if (!_outOfLoanedSamples) {
                    printf("%sThis is expected. Try to reduce it or avoid it by increasing the send queue size.\n", message->text);
                }

                _outOfLoanedSamples = true;
            } else {
                printf("%s", message->text);
            }
        } else {
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

