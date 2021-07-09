/*
 * (c) 2005-2021 Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */

using System;
using System.Threading;
using System.Diagnostics;

namespace PerformanceTest
{
    public class CpuMonitor
    {
        // CPU usage (%) resgitered in the last call to GetCpuInstant()
        private double cpuUsage;
        // Sum of the CPU usages registered since the creation of the object. When divided by Counter, we get
        // CPU usage on average
        private double cpuUsageTotal;
        // The moment where the current space of time for next measurement began
        private DateTime startTime;
        // The CPU usage at the beginning of the current space of time
        private TimeSpan startCpuUsage;
        // Registers the amount of times the method GetCpuInstant() has been called so it can calculate the
        // average accordingly
        private long counter;

        public CpuMonitor()
        {
            // The first interval starts as soon as the object is created
            startTime = DateTime.UtcNow;
            startCpuUsage = Process.GetCurrentProcess().TotalProcessorTime;
        }

        /*
        It gets the CPU usage in the space of time that began with the previous call to this function and
        ends with the current call to it
        */
        public double GetCpuInstant()
        {
            // Checking if the space of time is bigger enough to guarantee accuracy. In case it isn't,
            // it will return the measurements of the previous interval, but this is almost impossible to
            // happen with perftest.
            if (EnoughTimePassed())
            {
                // The interval ends now
                var endTime = DateTime.UtcNow;
                var endCpuUsage = Process.GetCurrentProcess().TotalProcessorTime;
                // CPU usage in the interval is calculated now that we have all the information needed
                var cpuUsedMs = (endCpuUsage - startCpuUsage).TotalMilliseconds;
                var totalMsPassed = (endTime - startTime).TotalMilliseconds;

                cpuUsage = cpuUsedMs / (Environment.ProcessorCount * totalMsPassed) * 100;

                // The next interval starts when the current one ends
                startTime = DateTime.UtcNow;
                startCpuUsage = Process.GetCurrentProcess().TotalProcessorTime;
                // Update of CpuUsageTotal and Counter for average calculation
                cpuUsageTotal += cpuUsage;
                counter++;
            }

            return cpuUsage;
        }

        // It returns the CPU usage on average from all the previous intervals
        public double GetCpuAverage()
        {
            if (counter == 0)
            {
                startTime = DateTime.UtcNow;
                startCpuUsage = Process.GetCurrentProcess().TotalProcessorTime;

                Thread.Sleep(1000);

                // In the case that the CpuMonitor was just initialized.
                GetCpuInstant();
            }
            return (double) (cpuUsageTotal / counter);
        }

        // Checks if enough time has passed to ensure the measurement is accurate
        private bool EnoughTimePassed()
        {
            // Check if at least 1/4 of a second passed since last update.
            // This will make the measurements more accurate.
            return (DateTime.Now - startTime).TotalMilliseconds > 250;
        }
    }
}