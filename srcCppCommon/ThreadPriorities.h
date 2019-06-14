/*
 * (c) 2005-2018  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */

#ifndef THREADPRIORITIES_H
#define THREADPRIORITIES_H

/*
 * This class is used to keep the values of the priorities in the case where
 * the user provide them.
 */
#ifdef RTI_WIN32
  #include <windows.h>
#elif defined(RTI_UNIX)
  #include <sched.h>
  #include <pthread.h>
#endif
#include <errno.h>
#include <map>
#include <stdio.h>
#include <string.h>
#include <string>

class ThreadPriorities {
  public:
    int main;
    int receive;
    int dbAndEvent;
    bool isSet;

    std::map<char, int> defaultPriorities;

    ThreadPriorities();
    bool set_priorities(char x, char y, char z);
    bool set_main_thread_priority();
    bool check_priority_range(int value);
    bool parse_priority(std::string arg);
};

#endif