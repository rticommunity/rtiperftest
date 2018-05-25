#ifndef __RTILOGGERDEVICE_H__
#define __RTILOGGERDEVICE_H__

/*
 * (c) 2005-2017  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */

#include <string>
#include <stdio.h>
#include <stdlib.h>
#include "ndds/ndds_cpp.h"

const std::string SHMEM_ERROR =
        "NDDS_Transport_Shmem_create_recvresource_rrEA:failed to initialize shared memory resource segment for key";

class RTIDDSLoggerDevice : public NDDSConfigLoggerDevice {

  private:
    bool shmem_issue;
  public:
    RTIDDSLoggerDevice();

    virtual void write(const NDDS_Config_LogMessage *message);

    bool get_shmem_issue();

    virtual void close() ;
};

#endif // __RTILOGGERDEVICE_H__
