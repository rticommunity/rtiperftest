/*
 * (c) 2005-2018  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */

/*
 * This class is used to keep the values of the priorities in the case where
 * the user provide them.
 */
#ifdef RTI_WIN32
#include <windows.h>
#else
#include <sched.h>
#endif
#include <map>

class PerftestThreadPriorities {
public:
    int main;
    int receive;
    int dbAndEvent;
    bool isSet;

    std::map<char, int> defaultPriorities;

    PerftestThreadPriorities();
    bool set_priorities(char x, char y, char z);
};