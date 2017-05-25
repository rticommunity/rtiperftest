
#include <iostream>
#include <string>
#include <sstream>
#include <iomanip> // set precision

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
    clock_t _lastCPU, _lastSysCPU, _lastUserCPU;
#elif defined(RTI_WIN32)
    ULARGE_INTEGER _lastCPU, _lastSysCPU, _lastUserCPU;
    HANDLE self;
#endif

};
