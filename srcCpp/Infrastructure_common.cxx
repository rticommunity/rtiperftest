/*
 * (c) 2005-2019  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */

#include "Infrastructure_common.h"

/* Perftest Timmer class */
#ifdef RTI_WIN32
LARGE_INTEGER PerftestTimer::_ClockFrequency = {0, 0};
/* This parameter is not thread safe */
HANDLE PerftestTimer::_hTimerQueue = NULL;
HANDLE PerftestTimer::_hTimer = NULL;
#endif
void (*PerftestTimer::_handlerFunction)(void) = NULL;

void *PerftestTimer::waitAndExecuteHandler(void *timerSeconds) 
{
    const unsigned int *t = static_cast<const unsigned int *>(timerSeconds);

    // Sleep for t milliseconds
    PerftestClock::milliSleep((*t) * 1000u);

    // Call the scheduled function with the args
    PerftestTimer::timeoutTask(0);
}

PerftestTimer::PerftestTimer()
{
  #ifdef RTI_WIN32
    if (_hTimerQueue == NULL) {
        _hTimerQueue = CreateTimerQueue();
    }
    QueryPerformanceFrequency(&_ClockFrequency);
  #endif
    _handlerFunction = NULL;
}

PerftestTimer::~PerftestTimer()
{
  #ifdef RTI_WIN32
    if (_hTimerQueue != NULL) {
        DeleteTimerQueue(_hTimerQueue);
    }
  #endif

}

PerftestTimer &PerftestTimer::getInstance()
{
    static PerftestTimer instance;
    return instance;
}

PerftestThread* PerftestTimer::setTimeout(
        unsigned int &executionTimeInSeconds,
        void (*function)(void))
{
    struct PerftestThread *timerThread = NULL;

    _handlerFunction = function;

    // We have to create a new pointer to the timer so 
    // it is not removed when this function ends
    timerThread = PerftestThread_new(
            "timerThread", 
            Perftest_THREAD_PRIORITY_DEFAULT,
            Perftest_THREAD_OPTION_DEFAULT,
            waitAndExecuteHandler,
            &executionTimeInSeconds);
    if (timerThread == NULL) {
        fprintf(stderr, "Problem creating timer thread.\n");
    }

    return timerThread;
}

#ifdef RTI_WIN32
VOID CALLBACK PerftestTimer::timeoutTask(PVOID lpParam, BOOLEAN timerOrWaitFired) {
    /* This is to avoid the warning of non using lpParam */
    (void) lpParam;
    PerftestTimer::_handlerFunction();
}
#else
void PerftestTimer::timeoutTask(int sign) {
    PerftestTimer::_handlerFunction();
}
#endif
