/*
 * (c) 2005-2016  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */
 using System;
 using System.Collections.Generic;
 using System.Linq;
 using System.Text;
 using System.Runtime.InteropServices;
 using ComTypes = System.Runtime.InteropServices.ComTypes;
 using System.Threading;
 using System.Diagnostics;


namespace PerformanceTest
{
    public class CpuMonitor
    {
         [DllImport("kernel32.dll", SetLastError = true)]
         static extern bool GetSystemTimes(
                out ComTypes.FILETIME lpIdleTime,
                out ComTypes.FILETIME lpKernelTime,
                out ComTypes.FILETIME lpUserTime
        );
        ComTypes.FILETIME _prevSysKernel;
        ComTypes.FILETIME _prevSysUser;
        TimeSpan _prevProcTotal;
        Int16 _cpuUsage;
        Int16 _cpuUsageTotal;
        DateTime _lastRun;
        long _counter;

        public CpuMonitor()
        {
            _cpuUsage = 0;
            _counter = 0;
            _cpuUsageTotal = 0;
            _lastRun = DateTime.MinValue;
            _prevSysUser.dwHighDateTime = _prevSysUser.dwLowDateTime = 0;
            _prevSysKernel.dwHighDateTime = _prevSysKernel.dwLowDateTime = 0;
            _prevProcTotal = TimeSpan.MinValue;
        }

        public void initialize()
        {
            _lastRun = DateTime.Now;
            Process process = Process.GetCurrentProcess();
            _prevProcTotal = process.TotalProcessorTime;
            ComTypes.FILETIME sysIdle, sysKernel, sysUser;
            GetSystemTimes(out sysIdle, out sysKernel, out sysUser);
            _prevSysKernel = sysKernel;
            _prevSysUser = sysUser;
        }

        public String get_cpu_instant()
        {
            short cpuCopy = _cpuUsage;
            if (EnoughTimePassed()) {

                ComTypes.FILETIME sysIdle, sysKernel, sysUser;
                TimeSpan procTime;

                Process process = Process.GetCurrentProcess();
                procTime = process.TotalProcessorTime;

                if (GetSystemTimes(out sysIdle, out sysKernel, out sysUser)) {
                    UInt64 sysKernelDiff = SubtractTimes(
                            sysKernel, _prevSysKernel);
                    UInt64 sysUserDiff = SubtractTimes(sysUser, _prevSysUser);
                    UInt64 sysTotal = sysKernelDiff + sysUserDiff;
                    Int64 procTotal = procTime.Ticks - _prevProcTotal.Ticks;
                    if (sysTotal > 0) {
                        _cpuUsage = (short)((100.0 * procTotal) / sysTotal);
                    }
                    _prevProcTotal = procTime;
                    _prevSysKernel = sysKernel;
                    _prevSysUser = sysUser;
                    _lastRun = DateTime.Now;
                    cpuCopy = _cpuUsage;
                }
            }
            _cpuUsageTotal += cpuCopy;
            _counter++;
            return " CPU: " + cpuCopy + "%";
        }

        public string get_cpu_average()
        {
            if (_counter == 0) {
                //in the case that the CpuMonitor was just initialize, get_cpu_instant
                get_cpu_instant();
            }
            return " CPU: " + (double)(_cpuUsageTotal/_counter) + "%";
        }

        private UInt64 SubtractTimes(ComTypes.FILETIME a, ComTypes.FILETIME b)
        {
            UInt64 aInt = ((UInt64)(a.dwHighDateTime << 32)) |
                    (UInt64)a.dwLowDateTime;
            UInt64 bInt = ((UInt64)(b.dwHighDateTime << 32)) |
                    (UInt64)b.dwLowDateTime;
            return aInt - bInt;
        }

        private bool EnoughTimePassed ()
        {
            const int minimumElapsedMS = 250;
            TimeSpan sinceLast = DateTime.Now - _lastRun;
            return sinceLast.TotalMilliseconds > minimumElapsedMS;
        }
    }
}