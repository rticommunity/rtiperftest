//using the algorithm to get cpu from http://stackoverflow.com/questions/63166/how-to-determine-cpu-and-memory-consumption-from-inside-a-process
#include "clock/clock_highResolution.h"
#include "osapi/osapi_ntptime.h"
#include <iostream>
#include <string>
#include <sstream>
#include <iomanip> // setprecision
#if defined(RTI_LINUX)
    #include "stdlib.h"
    #include "stdio.h"
    #include "string.h"
    #include "sys/times.h"
    #include "sys/vtimes.h"
#elif defined(RTI_DARWIN)
    #include "stdlib.h"
    #include "stdio.h"
    #include "string.h"
    #include "sys/times.h"
    #include <sys/sysctl.h>
    #include <cstddef>
#elif defined(RTI_WIN32)
    #include "windows.h"
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
    clock_t lastCPU, lastSysCPU, lastUserCPU;
#elif defined(RTI_WIN32)
    ULARGE_INTEGER lastCPU, lastSysCPU, lastUserCPU;
    HANDLE self;
#endif

};
