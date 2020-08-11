/*
 * (c) 2005-2019  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */

#include "Infrastructure_common.h"

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

int PerftestFileHandler::get_file_size(std::string const& fileName)
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
int PerftestFileHandler::read_file(
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
 * (c) 2005-2020  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */

/********************************************************************/
/* Perftest Clock class */

#ifdef PERTEST_RTI_MICRO
  #include "Infrastructure_micro.h"
#elif PERFTEST_RTI_PRO
  #include "Infrastructure_pro.h"
#else // General

  PerftestSemaphore* PerftestSemaphore_new(unsigned int count)
  {
      PerftestSemaphore *semaphore = NULL;
      semaphore = new PerftestSemaphore(count);
      return semaphore;
  };

  PerftestSemaphore* PerftestSemaphore_new()
  {
      PerftestSemaphore *semaphore = NULL;
      semaphore = new PerftestSemaphore(PERFTEST_SEMAPHORE_COUNT);
      return semaphore;
  };

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

  PerftestMutex* PerftestMutex_new()
  {
      PerftestMutex* mutex;
      return mutex;
  }

  void PerftestMutex_delete(std::mutex* mutex)
  {
      if (mutex != NULL ) {
          delete mutex;
      }
  }

  void PerftestMutex_give(std::mutex* mutex)
  {
      mutex->unlock();
  }

  bool PerftestMutex_take(std::mutex* mutex)
  {
      return mutex->try_lock();
  }

  PerftestClock::PerftestClock()
  {
  }

  PerftestClock::~PerftestClock()
  {
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

  void NDDSUtility::sleep(const struct DDS_Duration_t& durationIn)
  {
      std::this_thread::sleep_for(std::chrono::seconds(durationIn.sec));
      std::this_thread::sleep_for(std::chrono::nanoseconds(durationIn.nanosec));
  };

  void NDDSUtility::spin(unsigned long long int spinCount)
  {
      NDDS_Utility_spin(spinCount);
  };

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
          NDDS_Utility_spin(spinCount * iterations);
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

  const std::string GetDDSVersionString()
  {
      return "Eprosima FastDDS";
  }

  void PerftestConfigureVerbosity(int verbosityLevel)
  {
        #ifdef PERFTEST_EPROSIMA_FASTDDS
          using namespace eprosima::fastdds::dds;
          switch (verbosityLevel) {
              case 0: fprintf(stderr, "[Error]: Cannot set verbosity to SILENT\n");
                  break;
              case 1: Log::SetVerbosity(Log::Error);
                  fprintf(stderr, "Setting verbosity to ERROR\n");
                  break;
              case 2: Log::SetVerbosity(Log::Warning);
                  fprintf(stderr, "Setting verbosity to WARNING\n");
                  break;
              case 3: Log::SetVerbosity(Log::Info);
                  fprintf(stderr, "Setting verbosity to INFO\n");
                  break;
              default: fprintf(stderr,
                      "[Error]: Invalid value for the verbosity parameter. Using default\n");
                  break;
          }
        #endif
  }

  struct PerftestMicroThreadOnSpawnedMethod
  {
      MicroThreadOnSpawnedMethod method;
      void *thread_param;

  };

  PerftestThread* PerftestThread_new(
        const char *name,
        int threadPriority,
        int threadOptions,
        MicroThreadOnSpawnedMethod method,
        void *threadParam)
  {
      PerftestThread *thread = new PerftestThread(method, threadParam);
      if( thread->get_id() != std::this_thread::get_id()) {
          return NULL;
      }
      return thread;
  }

  void PerftestThread_delete(PerftestThread* thread)
  {
      if (thread != NULL) {
          delete thread;
      }
  }
#endif