/*
 * (c) 2005-2017 Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */


package com.rti.perftest.harness;

import java.util.HashMap;
import java.util.Map;

public class PerftestThreadPriorities {

    public int main;
    public int receive;
    public int dbAndEvent;
    public boolean isSet;

    private static HashMap<Character, Integer> defaultPriorities = new HashMap<Character, Integer>();

    public PerftestThreadPriorities() {
        main = 0;
        receive = 0;
        dbAndEvent = 0;
        isSet = false;

        defaultPriorities.put('h', 99);
        defaultPriorities.put('n', 50);
        defaultPriorities.put('l', 1);

    }

}