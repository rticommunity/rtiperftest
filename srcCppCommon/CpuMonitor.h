#ifndef __CPUMONITOR_H__
#define __CPUMONITOR_H__

/*
 * (c) 2005-2017  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */

#include <iostream>
#include <string>
#include <sstream>
#include <iomanip> // set precision

#if defined(RTI_LINUX)
    #include "stdlib.h"
    #include "stdio.h"
    #include "string.h"
    #include "sys/times.h"
#elif defined(RTI_DARWIN)
    #include "stdlib.h"
    #include "stdio.h"
    #include "string.h"
    #include "sys/times.h"
    #include <sys/sysctl.h>
    #include <cstddef>
#elif defined(RTI_WIN32)
    #include "windows.h"
#elif defined(RTI_INTIME)
#endif

class CpuMonitor
{
public:
    CpuMonitor();

    void initialize();

    std::string get_cpu_instant();

    std::string get_cpu_average();

private:
    int _numProcessors;
    unsigned long long _counter;
    long double _cpuUsageTotal;
#if defined(RTI_LINUX) || defined(RTI_DARWIN)
    clock_t _lastCPU, _lastSysCPU, _lastUserCPU;
#elif defined(RTI_WIN32)
    ULARGE_INTEGER _lastCPU, _lastSysCPU, _lastUserCPU;
    HANDLE self;
#endif

};

#endif // __CPUMONITOR_H__
