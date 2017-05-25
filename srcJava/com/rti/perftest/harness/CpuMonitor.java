/*
 * (c) 2005-2017  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */

package com.rti.perftest.harness;

import java.lang.management.ManagementFactory;
import com.sun.management.OperatingSystemMXBean;


//===========================================================================
/*package*/ class CpuMonitor {
    // -----------------------------------------------------------------------
    // Public Fields
    // -----------------------------------------------------------------------
    private long _counter;
    private double _cpuUsageTotal;
    private OperatingSystemMXBean _monitor;


    // --- Constructors: -----------------------------------------------------
    /*package*/ CpuMonitor() {
        _counter = 0;
        _cpuUsageTotal = 0;
        _monitor = ManagementFactory.getPlatformMXBean(OperatingSystemMXBean.class);
    }

    // -----------------------------------------------------------------------
    // package Methods
    // -----------------------------------------------------------------------
    /*package*/ String get_cpu_instant() {
        double cpu = _monitor.getProcessCpuLoad() * 100;
        _counter++;
        _cpuUsageTotal += cpu;
        return String.format(" CPU: %,.1f percent ",cpu);
    }

    /*package*/ String  get_cpu_average() {
        double cpu_avg = 0.0;
        if (_counter > 0) {
            cpu_avg = (double)(_cpuUsageTotal / _counter);
        }
        return String.format(" CPU: %,.1f percent ",cpu_avg);
    }
}