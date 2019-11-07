/*
 * (c) 2005-2019 Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */

#ifndef INFRASTRUCTURE_COMMON_H_
#define INFRASTRUCTURE_COMMON_H_

#include "perftest.h"
#include <fstream>

#ifdef RTI_UNIX
  /* This is a C POSIX library but Microsoft Visual C++ does not include this */
  #include "dirent.h"
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

int getFileSize(std::string const& filename);

std::vector<std::string> getFilesPathFromDirectory(const std::string &dirPath);

int readFile(
        std::string const& filename,
        std::vector<char>& fileData,
        unsigned int bytesToRead = 0, /* File size */
        unsigned int startPos = 0);

#endif /* INFRASTRUCTURE_COMMON_H_ */
