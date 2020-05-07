/*
 * (c) 2005-2019 Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */

#ifndef INFRASTRUCTURE_COMMON_H_
#define INFRASTRUCTURE_COMMON_H_

#include "perftest.h"
#include <fstream>

#include <sys/types.h>
#include <sys/stat.h>

#ifndef RTI_PERFTEST_NANO_CLOCK
  #define PERFT_TIME_UNIT "us"
#else
  #define PERFT_TIME_UNIT "ns"
#endif

#ifdef RTI_MICRO
  #include "Infrastructure_micro.h"
#else
  #include "Infrastructure_pro.h"
#endif

#ifdef RTI_VXWORKS
  #include <unistd.h>
  #include <sys/time.h>
#endif

/* Disable certain warnings for Windows */
#if defined(RTI_WIN32) || defined(RTI_INTIME)
  #pragma warning(push)
  #pragma warning(disable : 4996)
#endif

/*
 * Since std::to_string is not defined until c++11
 * we will define it here.
 */
namespace std {
    template<typename T>
    std::string to_string(const T &n) {
        std::ostringstream s;
        s << n;
        return s.str();
    }
}

/* Perftest Timer Class */
class PerftestTimer
{
  private:
    static void *waitAndExecute(void *scheduleInfo);

  public:
    struct ScheduleInfo {
        unsigned int timer;
        void (*handlerFunction)(void);
    };

    PerftestTimer() {}  // We need empty constructor and destructor for VxWorks
    ~PerftestTimer() {}
    static PerftestTimer &getInstance();
    PerftestThread *setTimeout(ScheduleInfo &info);
};


class PerftestFileHandler
{
  public:
    static bool path_is_file(std::string const& path);
    static int get_file_size(std::string const& fileName);
    static int read_file(
            std::string const& filename,
            char * fileData,
            unsigned int bytesToRead = 0,
            unsigned int startPos = 0);
};

#endif /* INFRASTRUCTURE_COMMON_H_ */
