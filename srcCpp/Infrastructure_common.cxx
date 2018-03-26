/*
 * (c) 2005-2018  Copyright, Real-Time Innovations, Inc. All rights reserved.
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
void (*PerftestTimer::handler_function)(void) = NULL;

PerftestTimer::PerftestTimer()
{
#ifdef RTI_WIN32
    if (_hTimerQueue == NULL) {
        _hTimerQueue = CreateTimerQueue();
    }
    QueryPerformanceFrequency(&_ClockFrequency);
#endif
    handler_function = NULL;
}

PerftestTimer::~PerftestTimer()
{
#ifdef RTI_WIN32
    if (_hTimerQueue != NULL) {
        DeleteTimerQueue(_hTimerQueue);
    }
#endif
}

void PerftestTimer::SetTimeout(unsigned int executionTimeInSeconds,
        void (*function)(void)) {

    handler_function = function;

#ifdef RTI_WIN32
    CreateTimerQueueTimer(
            &_hTimer,
            _hTimerQueue,
            (WAITORTIMERCALLBACK)PerftestTimer::TimeoutTask,
            NULL,
            executionTimeInSeconds * 1000,
            0,
            0);
#else
    signal(SIGALRM, PerftestTimer::TimeoutTask);
    alarm(executionTimeInSeconds);
#endif
}

#ifdef RTI_WIN32

VOID CALLBACK PerftestTimer::TimeoutTask(PVOID lpParam, BOOLEAN timerOrWaitFired) {
    /* This is to avoid the warning of non using lpParam */
    (void) lpParam;
    PerftestTimer::handler_function();
}
#pragma warning(pop)

#else

void PerftestTimer::TimeoutTask(int sign) {
    PerftestTimer::handler_function();
}

#endif
