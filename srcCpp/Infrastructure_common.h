/*
 * (c) 2005-2019 Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */

#ifndef INFRASTRUCTURE_COMMON_H_
#define INFRASTRUCTURE_COMMON_H_

#include "perftest.h"

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

#if defined(RTI_ANDROID)

#include <android/log.h>
typedef int (*RTIAndroidOnPrintfMethod)(const char *format, va_list ap);
static RTIAndroidOnPrintfMethod publisher_onPrintf = NULL;

#define printf Android_printf
#define fprintf Android_fprintf

static int Android_printf(const char *format, ...) {
    int len;
    va_list ap;
    va_start(ap, format);

    if (publisher_onPrintf!= NULL) {
        len = publisher_onPrintf(format, ap);
    } else {
        len = __android_log_vprint(ANDROID_LOG_INFO, "RTIConnextLog", format, ap);
    }

    va_end(ap);
    return len;
}

static int Android_fprintf(FILE *fptr, const char *format, ...) {
    int len;
    va_list ap;
    va_start(ap, format);

    if (publisher_onPrintf!= NULL) {
        len = publisher_onPrintf(format, ap);
    } else {
        len = __android_log_vprint(ANDROID_LOG_INFO, "RTIConnextLog", format, ap);
    }

    va_end(ap);
    return len;
}

extern "C" void RTIAndroid_registerOnPrintf(RTIAndroidOnPrintfMethod onPrintf) {
    publisher_onPrintf = onPrintf;
}

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

#endif /* INFRASTRUCTURE_COMMON_H_ */
