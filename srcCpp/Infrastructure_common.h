/*
 * (c) 2005-2020 Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */

#ifndef INFRASTRUCTURE_COMMON_H_
#define INFRASTRUCTURE_COMMON_H_

#if defined(PERFTEST_RTI_PRO) || defined(PERFTEST_RTI_MICRO)
  #include "perftest.h"
#elif defined(RTI_PERF_TSS)
  #include "gen/perftest.h"
#endif
#include <fstream>
#include <sstream>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>

#ifdef RTI_PERF_TSS_PRO
  #include "ndds/ndds_cpp.h"
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
 * Define the units for time when using Usec or Nsec
 */
#ifndef RTI_PERFTEST_NANO_CLOCK
  #define PERFT_TIME_UNIT "μs"
#else
  #define PERFT_TIME_UNIT "ns"
#endif

/*
 * Connext DDS Pro and Micro have their own implementation for:
 *
 * - Semaphore
 * - Mutex
 * - Clock
 * - Threads
 *
 * Therefore we will use those, to ensure not having to use the C++11 code,
 * which will make us compatible with less OSs. In other cases we do need a
 * implementation for these classes.
 */

#if defined(PERFTEST_RTI_PRO) || defined(RTI_PERF_TSS_PRO)
  #include "Infrastructure_pro.h"
#elif defined(PERFTEST_RTI_MICRO) || defined(RTI_PERF_TSS_MICRO)
  #include "Infrastructure_micro.h"
#endif

#if defined(RTI_USE_CPP_11_INFRASTRUCTURE) || (!defined(PERFTEST_RTI_MICRO) && !defined(PERFTEST_RTI_PRO))

#include <chrono>
#include <condition_variable>
#include <mutex>
#include <thread>

/*
 * TODO
 * We will keep this here for now, as depending on the DDS implementation, the
 * generated variable for a string is different. We need to move it to a generic
 * place and remove it from here in the future.
 */
#if !defined(PERFTEST_RTI_PRO) && !defined(PERFTEST_RTI_MICRO) && !defined(RTI_PERF_TSS)
static const char *const THROUGHPUT_TOPIC_NAME = "Throughput";
static const char *const LATENCY_TOPIC_NAME = "Latency";
static const char *const ANNOUNCEMENT_TOPIC_NAME = "Announcement";
#elif defined(RTI_PERF_TSS)
static const char *const THROUGHPUT_TOPIC_NAME = THROUGHPUT_TOPIC_NAME_STRING.buffer();
static const char *const LATENCY_TOPIC_NAME = LATENCY_TOPIC_NAME_STRING.buffer();
static const char *const ANNOUNCEMENT_TOPIC_NAME = ANNOUNCEMENT_TOPIC_NAME_STRING.buffer();
#endif


/********************************************************************/
/* Perftest Semaphore class */

#define PERFTEST_SEMAPHORE_COUNT 1
#define PERFTEST_SEMAPHORE_TIMEOUT_INFINITE -1

class PerftestSemaphore {

private:
    unsigned int count;
    std::mutex mutex;
    std::condition_variable condition;

public:
    explicit PerftestSemaphore(unsigned int count) : count(count)
    {
    }

    bool take()
    {
        try {
            std::unique_lock<std::mutex> lock(mutex);
            condition.wait(lock, [&]() -> bool { return count > 0; });
            --count;
            return true;
        } catch (std::exception& e) {
            std::cerr << "[Exception]: "<<  e.what() << std::endl;
            return false;
        }
    }

    bool take(int timeout_millisec)
    {
        try {
            std::chrono::milliseconds timeout(timeout_millisec);
            std::unique_lock<std::mutex> lock(mutex);
            condition.wait_for(lock, timeout, [&]() -> bool { return count > 0; });
            --count;
            return true;
        } catch (std::exception& e) {
            std::cerr << "[Exception]: "<<  e.what() << std::endl;
            return false;
        }
    }

    bool give()
    {
        try {
            std::unique_lock<std::mutex> lock(mutex);
            ++count;
            condition.notify_one();
            return true;
        } catch (std::exception& e) {
            std::cerr << "[Exception]: " << e.what() << std::endl;
            return false;
        }
    }
};

PerftestSemaphore *PerftestSemaphore_new(unsigned int count);
PerftestSemaphore *PerftestSemaphore_new();
void PerftestSemaphore_delete(PerftestSemaphore *semaphore);
bool PerftestSemaphore_give(PerftestSemaphore *semaphore);
bool PerftestSemaphore_take(PerftestSemaphore *semaphore, int timeout);


/********************************************************************/
/* Perftest Mutex class */

#define PerftestMutex std::mutex
PerftestMutex *PerftestMutex_new();
void PerftestMutex_delete(std::mutex *mutex);
bool PerftestMutex_give(std::mutex *mutex);
bool PerftestMutex_take(std::mutex *mutex);

/********************************************************************/
/* Perftest Clock class */

#define PERFTEST_DISCOVERY_TIME_MSEC 1000  // 1 second
#define ONE_BILLION 1000000000L  // 1 billion (US) == 1 second in ns
#define ONE_MILLION 1000000L     // 1 million == 1 second in us

class PerftestClock {
public:
    static PerftestClock &getInstance();
    unsigned long long getTime();
    static void milliSleep(unsigned int millisec);
    static void sleep(const struct DDS_Duration_t &sleep_period);
};

/********************************************************************/
/* Perftest Thread class */

#define PerftestThread std::thread
#define Perftest_THREAD_PRIORITY_DEFAULT 0
#define Perftest_THREAD_SETTINGS_REALTIME_PRIORITY 0
#define Perftest_THREAD_SETTINGS_PRIORITY_ENFORCE 0
#define Perftest_THREAD_OPTION_DEFAULT 0

typedef void *(*ThreadOnSpawnedMethod)(void *);

PerftestThread *PerftestThread_new(
        const char *name,
        int threadPriority,
        int threadOptions,
        ThreadOnSpawnedMethod method,
        void *threadParam);

void PerftestThread_delete(PerftestThread *thread);

#if !defined(PERFTEST_RTI_PRO) && !defined(PERFTEST_RTI_MICRO) && !defined(RTI_PERF_TSS)
struct DDS_Duration_t {
    int sec;
    unsigned int nanosec;
};
#endif

#if !defined(PERFTEST_RTI_PRO) && !defined(PERFTEST_RTI_MICRO) && !defined(RTI_PERF_TSS_PRO)
class NDDSUtility {
public:
    /*e \dref_Utility_sleep */
    static void sleep(const struct DDS_Duration_t &durationIn);

    /*e \dref_Utility_spin */
    static void spin(unsigned long long int spinCount);

    static unsigned long long int
            get_spin_per_microsecond(unsigned int precision = 100);
};
#endif

#endif

/********************************************************************/
/* Perftest Timer class */

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

/********************************************************************/
/* Perftest FileHandler class */

class PerftestFileHandler
{
  public:
    static bool path_is_file(std::string const& path);
    static long get_file_size(std::string const& fileName);
    static long read_file(
            std::string const& filename,
            char * fileData,
            unsigned int bytesToRead = 0,
            unsigned int startPos = 0);
};

/*
 * Since std::to_string is not defined until c++11
 * we will define it here.
 */
namespace perftest {
    template<typename T>
    std::string to_string(const T &n) {
        std::ostringstream s;
        s << n;
        return s.str();
    }
}

bool is_ip_address(std::string ip_string);

#endif /* INFRASTRUCTURE_COMMON_H_ */
