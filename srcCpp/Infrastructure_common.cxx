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

void PerftestTimer::setTimeout(
        unsigned int executionTimeInSeconds,
        void (*function)(void))
{

    _handlerFunction = function;

  #ifdef RTI_WIN32
    CreateTimerQueueTimer(
            &_hTimer,
            _hTimerQueue,
            (WAITORTIMERCALLBACK)PerftestTimer::timeoutTask,
            NULL,
            executionTimeInSeconds * 1000,
            0,
            0);
  #else
    signal(SIGALRM, PerftestTimer::timeoutTask);
    alarm(executionTimeInSeconds);
  #endif
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
