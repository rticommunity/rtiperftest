/*
 * (c) 2005-2020  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */

#include "Infrastructure_common.h"

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
#if !defined(PERFTEST_RTI_MICRO) && !defined(PERFTEST_RTI_PRO)

/********************************************************************/
/* Perftest Semaphore class */

PerftestSemaphore* PerftestSemaphore_new(unsigned int count)
{
    return new PerftestSemaphore(count);
}

PerftestSemaphore* PerftestSemaphore_new()
{
    PerftestSemaphore *semaphore = NULL;
    semaphore = new PerftestSemaphore(PERFTEST_SEMAPHORE_COUNT);
    return semaphore;
}

void PerftestSemaphore_delete(PerftestSemaphore* semaphore)
{
    delete semaphore;
}

bool PerftestSemaphore_give(PerftestSemaphore* semaphore)
{
    return semaphore->give();
}

bool PerftestSemaphore_take(PerftestSemaphore* semaphore, int timeout)
{
    return semaphore->take();
}

/********************************************************************/
/* Perftest Mutex class */

PerftestMutex* PerftestMutex_new()
{
    PerftestMutex* mutex;
    mutex = new PerftestMutex();
    return mutex;
}

void PerftestMutex_delete(std::mutex* mutex)
{
    delete mutex;
}

void PerftestMutex_give(std::mutex* mutex)
{
    mutex->unlock();
}

bool PerftestMutex_take(std::mutex* mutex)
{
    return mutex->try_lock();
}

/********************************************************************/
/* Perftest Clock class */

#define SPIN_MACRO(spinCount)                 \
{                                                \
    unsigned long long int spin;                 \
    unsigned long long int ad, bd, cd;           \
    volatile unsigned long long int *a, *b, *c;  \
    a = &ad;                                     \
    b = &bd;                                     \
    c = &cd;                                     \
    for (spin = 0; spin < (spinCount); ++spin) { \
        *a = 3;                                  \
        *b = 1;                                  \
        *c = (*a / (*b)) * spin;                 \
    }                                            \
}

PerftestClock &PerftestClock::getInstance()
{
    static PerftestClock instance;
    return instance;
}

unsigned long long PerftestClock::getTime()
{
    clock_gettime(CLOCK_MONOTONIC, &timeStruct);
    return (timeStruct.tv_sec * ONE_MILLION) + timeStruct.tv_nsec/1000;
}

void PerftestClock::milliSleep(unsigned int millisec)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(millisec));
}

void PerftestClock::sleep(const struct DDS_Duration_t& sleep_period)
{
    NDDSUtility::sleep(sleep_period);
}

void NDDSUtility::sleep(const struct DDS_Duration_t &durationIn)
{
    std::this_thread::sleep_for(
            std::chrono::seconds(durationIn.sec)
            + std::chrono::nanoseconds(durationIn.nanosec));
}

void NDDSUtility::spin(unsigned long long int spinCount)
{
    SPIN_MACRO(spinCount);
}

unsigned long long int
NDDSUtility::get_spin_per_microsecond(unsigned int precision)
{
    /* Same default values used by DDS */
    unsigned int spinCount = 200000;
    unsigned long long clockCalculationLoopCountMax = 100;

    unsigned long long usec = 0;
    unsigned long long iterations = 0;

    PerftestClock clock = PerftestClock::getInstance();

    do{
        usec = clock.getTime(); // Initial time
        SPIN_MACRO(spinCount * iterations);
        usec = clock.getTime() - usec; // Final time
        iterations++;
        /*
        * If the the clock have a low precision, increase spinCount
        * until we measure some us or reach a maximun count loop
        */
    } while (usec < precision && iterations < clockCalculationLoopCountMax);

    /*
    * The measure time can be zero due to lack of resolution or non
    * monotonic clocks and a really fast machine.
    * We may end on a exception condition, unable to calculate the
    * spins per micro-seconds.
    */
    if (usec == 0) {
        fprintf(stderr,
                "Unable to calculate the number of spins per"
                "micro-seconds\n");
        return 0;
    }
    return (unsigned long long) (iterations * spinCount) / usec;
}

/********************************************************************/
/* Perftest Thread class */

struct PerftestThreadOnSpawnedMethod
{
    ThreadOnSpawnedMethod method;
    void *thread_param;

}

PerftestThread* PerftestThread_new(
    const char *name,
    int, // threadPriority (not used here)
    int, // threadOptions (not used here)
    ThreadOnSpawnedMethod method,
    void *threadParam)
{
    return new PerftestThread(method, threadParam);
}

void PerftestThread_delete(PerftestThread* thread)
{
    if (thread != NULL) {
        thread->join();
        delete thread;
    }
}

#endif // If not defined PRO or MICRO


/********************************************************************/
/* Perftest Timer class */

void *PerftestTimer::waitAndExecute(void *scheduleInfo)
{
    ScheduleInfo *info = static_cast<ScheduleInfo *>(scheduleInfo);

    PerftestClock::milliSleep(info->timer * 1000u);

    if (info->handlerFunction != NULL) {
        info->handlerFunction();
    }

    return NULL;
}

PerftestTimer &PerftestTimer::getInstance()
{
    static PerftestTimer instance;
    return instance;
}

PerftestThread *PerftestTimer::setTimeout(PerftestTimer::ScheduleInfo &info)
{
    struct PerftestThread *timerThread = NULL;

    timerThread = PerftestThread_new(
            "timerThread",
            Perftest_THREAD_PRIORITY_DEFAULT,
            Perftest_THREAD_OPTION_DEFAULT,
            waitAndExecute,
            &info);

    return timerThread;
}

/********************************************************************/
/* Perftest FileHandler class */

bool PerftestFileHandler::path_is_file(std::string const& path)
{

  #if defined(RTI_UNIX) || defined(RTI_WIN32)
    struct stat fileStat;
    if (stat(path.c_str(), &fileStat) == 0) {
        if (fileStat.st_mode & S_IFREG) {
            return true;
        } else {
            std::cerr << "[Error] path_is_file: \"" << path
                      << "\" is not a regular file" << std::endl;
            return false;
        }
    } else {
        std::cerr << "[Error] path_is_file: Could not open file: \"" << path
                  << "\"" << std::endl;
        return false;
    }
  #else
    std::cerr << "[Error] path_is_file: Function not implemented for this OS"
              << std::endl;
    return false;
  #endif
}

long PerftestFileHandler::get_file_size(std::string const& fileName)
{

  #if defined(RTI_UNIX) || defined(RTI_WIN32)
    std::ifstream ifs(fileName.c_str(), std::ios::binary | std::ios::ate);
    if (!ifs.good()) {
        std::cerr << "[Error] get_file_size: Could not open file: \""
                  << fileName
                  << "\""
                  << std::endl;
        return -1;
    }
    std::ifstream::pos_type pos = ifs.tellg();
    ifs.close();
    return pos;
  #else
    std::cerr << "[Error] get_file_size: Function not implemented for this OS"
              << std::endl;
    return -1;
  #endif
}

/* If the function can not read the file would return -1 */
long PerftestFileHandler::read_file(
        std::string const& fileName,
        char * outputData,
        unsigned int bytesToRead,
        unsigned int startPos)
{
  #if defined(RTI_UNIX) || defined(RTI_WIN32)
    std::ifstream ifs(fileName.c_str(), std::ios::binary | std::ios::ate);

    if (!ifs.good()) {
        std::cerr << "[Error] read_file: Could not open file: \"" << fileName
                  << "\""
                  << std::endl;
        return -1;
    }

    std::ifstream::pos_type pos = ifs.tellg();
    if (bytesToRead != 0 && bytesToRead < pos) {
        pos = bytesToRead;
    }
    else if (bytesToRead > pos) {
        std::cerr << "[Error] read_file: The file \""
                  << fileName
                  << "\" does not have the number of bytes requested ("
                  << bytesToRead
                  << ") from the start position "
                  << startPos
                  << "."
                  << std::endl;
        return -1;
    }

    /* By default startPos is 0 */
    ifs.seekg(startPos, std::ios::beg);
    ifs.read(outputData, pos);
    ifs.close();

    return pos;
  #else
    std::cerr << "[Error] read_file: Function not implemented for this OS"
              << std::endl;
    return -1;
  #endif
}

/*
 * Function to check if a given string is an ip (it does not check for ranges)
 */
bool is_ip_address(std::string ip_string)
{
    int octect1, octect2, octect3, octect4;
    if (sscanf(ip_string.c_str(),
               "%d.%d.%d.%d",
               &octect1,
               &octect2,
               &octect3,
               &octect4)
        != 4) {
        return false;
    }
    return true;
}
