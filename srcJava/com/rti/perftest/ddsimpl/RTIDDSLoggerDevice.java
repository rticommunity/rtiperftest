/*
 * (c) 2005-2017  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */

package com.rti.perftest.ddsimpl;

import com.rti.ndds.config.LoggerDevice;
import com.rti.ndds.config.LogMessage;
import com.rti.ndds.config.LogLevel;


// ===========================================================================
public class RTIDDSLoggerDevice implements LoggerDevice {

    private boolean shmem_issue = false;
    private static String SHMEM_ERROR =
            "NDDS_Transport_Shmem_create_recvresource_rrEA:failed to initialize shared memory resource segment for key";

    public RTIDDSLoggerDevice()
    {
    	this.shmem_issue = false;
    }

    public void write(LogMessage message)
    {
        if (!shmem_issue) {
            if (message.level == LogLevel.NDDS_CONFIG_LOG_LEVEL_ERROR) {
                if (message.text.contains(SHMEM_ERROR)) {
                    shmem_issue = true;
                }
            }
            if (!shmem_issue) {
                System.out.print(message.text);
            }
        }
    }

    public void close()
    {

    }

    boolean get_shmem_issue()
    {
       return this.shmem_issue;
    }

}

// ===========================================================================
