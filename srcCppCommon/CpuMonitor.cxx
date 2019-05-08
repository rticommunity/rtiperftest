/*
 * (c) 2005-2017  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */

#include "CpuMonitor.h"

CpuMonitor::CpuMonitor()
{
    _counter = 0;
    _cpuUsageTotal = 0.0;

#if defined(RTI_LINUX)

    _lastCPU = 0;
    _lastSysCPU = 0;
    _lastUserCPU = 0;

    FILE* file;
    char line[128];

    file = fopen("/proc/cpuinfo", "r");
    _numProcessors = 0;
    while (fgets(line, 128, file) != NULL) {
        if (strncmp(line, "processor", 9) == 0) {
            _numProcessors++;
        }
    }
    fclose(file);

#elif defined(RTI_DARWIN)

    _lastCPU = 0;
    _lastSysCPU = 0;
    _lastUserCPU = 0;

    int mib[4];
    std::size_t len = sizeof(_numProcessors);

    /* set the mib for hw.ncpu */
    mib[0] = CTL_HW;
    mib[1] = HW_AVAILCPU;  // alternatively, try HW_NCPU;

    /* get the number of CPUs from the system */
    sysctl(mib, 2, &_numProcessors, &len, NULL, 0);

    if (_numProcessors < 1) {
        mib[1] = HW_NCPU;
        sysctl(mib, 2, &_numProcessors, &len, NULL, 0);
        if (_numProcessors < 1) {
            _numProcessors = 1;
        }
    }

#elif defined(RTI_WIN32)

    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    _numProcessors = sysInfo.dwNumberOfProcessors;

#else
    fprintf(stderr, "[WARNING] get CPU consumption feature, is not available in this OS\n");

#endif
}

void CpuMonitor::initialize()
{

#if defined(RTI_LINUX) || defined(RTI_DARWIN)

    struct tms timeSample;
    _lastCPU = times(&timeSample);
    _lastSysCPU = timeSample.tms_stime;
    _lastUserCPU = timeSample.tms_utime;

#elif defined(RTI_WIN32)

    FILETIME ftime, fsys, fuser;
    GetSystemTimeAsFileTime(&ftime);
    memcpy(&_lastCPU, &ftime, sizeof(FILETIME));

    self = GetCurrentProcess();
    GetProcessTimes(self, &ftime, &ftime, &fsys, &fuser);
    memcpy(&_lastSysCPU, &fsys, sizeof(FILETIME));
    memcpy(&_lastUserCPU, &fuser, sizeof(FILETIME));

#endif
}

std::string CpuMonitor::get_cpu_instant()
{
    double percent = 0.0;

#if defined(RTI_LINUX) || defined(RTI_DARWIN)

    struct tms timeSample;
    clock_t now;
    now = times(&timeSample);
    if (now > _lastCPU && timeSample.tms_stime >= _lastSysCPU &&
            timeSample.tms_utime >= _lastUserCPU) {
        percent = (timeSample.tms_stime - _lastSysCPU) +
                (timeSample.tms_utime - _lastUserCPU);
        percent /= (now - _lastCPU);
        percent /= _numProcessors;
        percent *= 100;
        _lastCPU = now;
        _lastSysCPU = timeSample.tms_stime;
        _lastUserCPU = timeSample.tms_utime;
    }

#elif defined(RTI_WIN32)

    FILETIME ftime, fsys, fuser;
    ULARGE_INTEGER now, sys, user;
    GetSystemTimeAsFileTime(&ftime);
    memcpy(&now, &ftime, sizeof(FILETIME));
    if (now.QuadPart - _lastCPU.QuadPart>0.0) {
        GetProcessTimes(self, &ftime, &ftime, &fsys, &fuser);
        memcpy(&sys, &fsys, sizeof(FILETIME));
        memcpy(&user, &fuser, sizeof(FILETIME));
        percent = (double)(sys.QuadPart - _lastSysCPU.QuadPart) +
                (user.QuadPart - _lastUserCPU.QuadPart);
        percent /= (double)(now.QuadPart - _lastCPU.QuadPart);
        _lastCPU = now;
        _lastUserCPU = user;
        _lastSysCPU = sys;
        percent /= _numProcessors;
        percent *= 100;
    }

#endif

    _cpuUsageTotal += percent;
    _counter++;
    std::ostringstream strs;
    strs <<  std::fixed << std::setprecision(2)  << percent;
    return ("CPU: " + strs.str() + "%");
}

std::string CpuMonitor::get_cpu_average()
{
    std::ostringstream strs;
    if (_counter == 0) {
        // In the case where the CpuMonitor was just initialized, get_cpu_instant
        get_cpu_instant();
    }
    strs <<  std::fixed << std::setprecision(2)
         << (double)(_cpuUsageTotal/_counter);

    return ("CPU: " + strs.str() + "%");
}

