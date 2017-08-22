/*
 * (c) 2005-2017  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */

package com.rti.perftest.ddsimpl;

import com.rti.dds.dynamicdata.DynamicData;
import com.rti.perftest.gen.TestDataKeyed_t;
import com.rti.perftest.gen.TestDataKeyed_tTypeCode;
import com.rti.perftest.gen.TestData_t;
import com.rti.perftest.gen.TestData_tTypeCode;
import com.rti.perftest.gen.TestDataKeyedLarge_t;
import com.rti.perftest.gen.TestDataKeyedLarge_tTypeCode;
import com.rti.perftest.gen.TestDataLarge_t;
import com.rti.perftest.gen.TestDataLarge_tTypeCode;
import com.rti.perftest.harness.PerfTest;
import com.rti.perftest.gen.MAX_SYNCHRONOUS_SIZE;
import com.rti.perftest.gen.MAX_BOUNDED_SEQ_SIZE;

import java.util.StringTokenizer;
// ===========================================================================

/**
 * The "main" class.
 */
public final class PerfTestLauncher {

    // -----------------------------------------------------------------------
    // Public Methods
    // -----------------------------------------------------------------------

    public static void main(String[] argv) {

        parseConfig(argv);
        int MAX_PERFTEST_SAMPLE_SIZE = Math.max((int)_dataLen,
                PerfTest.LENGTH_CHANGED_SIZE);

        if(_useUnbounded > 0) {
            System.err.println("Using unbounded Sequences, allocation_threshold " + Long.toString(_useUnbounded) + ".");
            if (_isKeyed) {
                System.err.println("Using keyed Data.");

                if (_isDynamicData) {
                    System.err.println("Using Dynamic Data.");
                    PerfTest.runTest(
                            new RTIDDSImpl<DynamicData>(
                                    new DynamicDataTypeHelper(
                                            TestDataKeyedLarge_tTypeCode.VALUE,
                                            _isKeyed,
                                            MAX_PERFTEST_SAMPLE_SIZE)),
                            argv);
                } else {
                    PerfTest.runTest(
                            new RTIDDSImpl<TestDataKeyedLarge_t>(
                                    new DataTypeKeyedLargeHelper(
                                            MAX_PERFTEST_SAMPLE_SIZE)),
                            argv);
                }
            } else {
                System.err.println("Using unkeyed Data.");

                if (_isDynamicData) {
                    System.err.println("Using Dynamic Data.");
                    PerfTest.runTest(
                            new RTIDDSImpl<DynamicData>(
                                    new DynamicDataTypeHelper(
                                            TestDataLarge_tTypeCode.VALUE,
                                            _isKeyed,
                                            MAX_PERFTEST_SAMPLE_SIZE)),
                            argv);
                } else {
                    PerfTest.runTest(
                            new RTIDDSImpl<TestDataLarge_t>(
                                    new DataTypeLargeHelper(
                                            MAX_PERFTEST_SAMPLE_SIZE))
                            ,argv);
                }
            }
        } else {
            if (_isKeyed) {
                System.err.println("Using keyed Data.");

                if (_isDynamicData) {
                    System.err.println("Using Dynamic Data.");
                    PerfTest.runTest(
                            new RTIDDSImpl<DynamicData>(
                                    new DynamicDataTypeHelper(
                                            TestDataKeyed_tTypeCode.VALUE,
                                            _isKeyed,
                                            MAX_PERFTEST_SAMPLE_SIZE)),
                            argv);
                } else {
                    PerfTest.runTest(
                            new RTIDDSImpl<TestDataKeyed_t>(
                                    new DataTypeKeyedHelper(
                                            MAX_PERFTEST_SAMPLE_SIZE)),
                            argv);
                }
            } else {
                System.err.println("Using unkeyed Data.");

                if (_isDynamicData) {
                    System.err.println("Using Dynamic Data.");
                    PerfTest.runTest(
                            new RTIDDSImpl<DynamicData>(
                                    new DynamicDataTypeHelper(
                                            TestData_tTypeCode.VALUE,
                                            _isKeyed,
                                            MAX_PERFTEST_SAMPLE_SIZE)),
                            argv);
                } else {
                    PerfTest.runTest(
                            new RTIDDSImpl<TestData_t>(new DataTypeHelper(
                                    MAX_PERFTEST_SAMPLE_SIZE)),
                            argv);
                }
            }
        }
    }

    // -----------------------------------------------------------------------
    // Private Methods
    // -----------------------------------------------------------------------

    private PerfTestLauncher() {
        // do nothing
    }

    private static boolean parseConfig(String[] argv) {
        int argc = argv.length;
        if (argc < 0) {
            return false;
        }

        for (int i = 0; i < argc; ++i) {
            if ("-keyed".toLowerCase().startsWith(argv[i].toLowerCase())) {
                // We can return here since we were just looking for this.
                _isKeyed = true;
            } else if ("-dynamicData".toLowerCase().startsWith(argv[i].toLowerCase())) {
                // We can return here since we were just looking for this.
                _isDynamicData = true;
            } else if ("-dataLen".toLowerCase().startsWith(argv[i].toLowerCase()))
            {
                if ((i == (argc - 1)) || argv[++i].startsWith("-"))
                {
                    System.err.print("Missing <length> after -dataLen\n");
                    return false;
                }
                try {
                    _dataLen = Long.parseLong(argv[i]);
                } catch (NumberFormatException nfx) {
                    System.err.print("Bad dataLen\n");
                    return false;
                }
                if (_dataLen < PerfTest.OVERHEAD_BYTES) {
                    System.err.println("dataLen must be >= " + PerfTest.OVERHEAD_BYTES);
                    return false;
                }
                if (_dataLen > PerfTest.getMaxPerftestSampleSizeJava()) {
                    System.err.println("dataLen must be <= " + PerfTest.getMaxPerftestSampleSizeJava());
                    return false;
                }
                if (_useUnbounded == 0 && _dataLen > MAX_BOUNDED_SEQ_SIZE.VALUE) {
                    _useUnbounded = Math.min(
                            MAX_BOUNDED_SEQ_SIZE.VALUE, 2 * _dataLen);
                }
            }else if ("-unbounded".toLowerCase().startsWith(argv[i].toLowerCase())) {
                if ((i == (argc - 1)) || argv[i+1].startsWith("-")) {
                     _useUnbounded = Math.min(
                             MAX_BOUNDED_SEQ_SIZE.VALUE, 2 * _dataLen);
                } else {
                    ++i;
                    try {
                        _useUnbounded = Long.parseLong(argv[i]);
                    } catch (NumberFormatException nfx) {
                        System.err.print("Bad allocation_threshold value.\n");
                        return false;
                    }
                }
                if (_useUnbounded < PerfTest.OVERHEAD_BYTES) {
                    System.err.println("-unbounded <value> must be >= " +
                            PerfTest.OVERHEAD_BYTES);
                    return false;
                }
                if (_useUnbounded > MAX_BOUNDED_SEQ_SIZE.VALUE) {
                    System.err.println("-unbounded <value> must be <= " +
                            MAX_BOUNDED_SEQ_SIZE.VALUE);
                    return false;
                }
            }else if ("-scan".toLowerCase().startsWith(argv[i].toLowerCase())) {
                _isScan = true;
                if ((i != (argc - 1)) && !argv[1+i].startsWith("-")) {
                    ++i;
                    long _scan_max_size = 0;
                    long aux_scan;
                    StringTokenizer st = new StringTokenizer(argv[i], ":", true);
                    while (st.hasMoreTokens()) {
                        String s = st.nextToken();
                        if (!s.equals(":")) {
                            aux_scan = Long.parseLong(s);
                            if (aux_scan >= _scan_max_size) {
                                _scan_max_size = aux_scan;
                            }
                        }
                    }
                    // Check if large data or small data
                    if (_scan_max_size > Math.min(MAX_SYNCHRONOUS_SIZE.VALUE,MAX_BOUNDED_SEQ_SIZE.VALUE)) {
                        if (_useUnbounded == 0) {
                            _useUnbounded = MAX_BOUNDED_SEQ_SIZE.VALUE;
                        }
                    } else if (_scan_max_size <= Math.min(MAX_SYNCHRONOUS_SIZE.VALUE,MAX_BOUNDED_SEQ_SIZE.VALUE)) {
                        if (_useUnbounded != 0) {
                            System.err.printf("Unbounded will be ignored since -scan is present.");
                            _useUnbounded = 0;
                        }
                    } else {
                        return false;
                    }
                }
            }
        }

        return true;
    }

    private static boolean _isKeyed = false;
    private static boolean _isScan = false;
    private static boolean _isDynamicData = false;
    private static long _useUnbounded = 0;
    private static long _dataLen = 100;
}

// ===========================================================================
