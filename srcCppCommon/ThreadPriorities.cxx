/*
 * (c) 2005-2018  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */


#include "ThreadPriorities.h"

ThreadPriorities::ThreadPriorities()
        : main(0), receive(0), dbAndEvent(0), isSet(false)
{
    #ifdef RTI_WIN32
        defaultPriorities['h'] = THREAD_PRIORITY_TIME_CRITICAL;
        defaultPriorities['n'] = THREAD_PRIORITY_NORMAL;
        defaultPriorities['l'] = THREAD_PRIORITY_IDLE;
    #elif RTI_UNIX
        defaultPriorities['h'] = sched_get_priority_max(SCHED_FIFO);
        defaultPriorities['n'] = (sched_get_priority_max(SCHED_FIFO)
                + sched_get_priority_min(SCHED_FIFO)) / 2;
        defaultPriorities['l'] = sched_get_priority_min(SCHED_FIFO);
    #endif
}

bool ThreadPriorities::set_priorities(
        char mainThreadPrio,
        char receiveThreadPrio,
        char dBEventThread)
{
    if (defaultPriorities.count(mainThreadPrio)
            && defaultPriorities.count(receiveThreadPrio)
            && defaultPriorities.count(dBEventThread)) {

        main       = defaultPriorities[mainThreadPrio];
        receive    = defaultPriorities[receiveThreadPrio];
        dbAndEvent = defaultPriorities[dBEventThread];
        return true;
    }
    return false;
}

bool ThreadPriorities::set_main_thread_priority()
{
    int priority = main;

#ifdef RTI_WIN32
    unsigned long error;

    if (!SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS)) {
        error = GetLastError();
        fprintf(stderr,
                "Fail to set main thread Class to real time, ERROR: %d\n",
                error);
        return false;
    }

    if (!SetThreadPriority(GetCurrentThread(), priority)) {
        error = GetLastError();
        if (priority == error)
            fprintf(stderr,
                    "The thread is already running with priority ERROR: %d\n",
                    error);
        else {
            fprintf(stderr,
                    "Fail to set main thread priority, ERROR: %d\n",
                    error);
        }
        return false;
    }
#elif RTI_UNIX
    int error = 0;
    struct sched_param sp;

    sp.sched_priority = priority;

    error = pthread_setschedparam(pthread_self(), SCHED_FIFO, &sp);
    if (error != 0) {
        fprintf(stderr,
                "Fail to set main thread priority, %s\n",
                strerror(error));
        if (error == EPERM) {
            fprintf(stderr, "\t - Try to run it as superUser\n");
        }
        return false;
    }
#else
    fprintf(stderr, "-threadPriorities are not supported on this platform\n");
#endif

    return true;
}

bool ThreadPriorities::check_priority_range(int value)
{
    bool success = true;
#ifdef RTI_WIN32
    if (value < -7 || value > 6) {
        if (value != THREAD_PRIORITY_TIME_CRITICAL
            && value != THREAD_PRIORITY_IDLE) {
            success = false;
        }
    }

#elif RTI_UNIX
    if (value < sched_get_priority_min(SCHED_FIFO)
        || value > sched_get_priority_max(SCHED_FIFO)) {
        success = false;
    }
#else
    fprintf(stderr, "-threadPriorities are not supported on this platform\n");
    return false;
#endif

    if (!success) {
        fprintf(stderr,
                "The input priority (%d) on -threadPriorities is outside"
                " of range for this platform\n",
                value);
        return false;
    }

    return true;
}

bool ThreadPriorities::parse_priority(std::string arg)
{
    char x, y, z;

    /* If is given by numbers */
    if (sscanf(arg.c_str(), "%d:%d:%d", &main, &receive, &dbAndEvent) == 3) {
        if (!check_priority_range(main)
                || !check_priority_range(receive)
                || !check_priority_range(dbAndEvent)) {
            fprintf(stderr, "Fail to parse -threadPriorities\n");
            return false;
        }
    } else if (sscanf(arg.c_str(), "%c:%c:%c", &x, &y, &z) == 3) {
        /* Check if is given by characters */
        if (!set_priorities(x, y, z)) {
            fprintf(stderr, "Fail to parse -threadPriorities\n");
            return false;
        }
    } else {
        fprintf(stderr, "Fail to parse -threadPriorities\n");
        return false;
    }

    return true;
}
