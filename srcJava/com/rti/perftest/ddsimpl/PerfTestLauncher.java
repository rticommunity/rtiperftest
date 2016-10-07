/*
 * (c) 2005-2016  Copyright, Real-Time Innovations, Inc.  All rights reserved.
 * Permission to modify and use for internal purposes granted.
 * This software is provided "as is", without warranty, express or implied.
 */

package com.rti.perftest.ddsimpl;

import com.rti.dds.dynamicdata.DynamicData;
import com.rti.perftest.gen.TestDataKeyed_t;
import com.rti.perftest.gen.TestDataKeyed_tTypeCode;
import com.rti.perftest.gen.TestData_t;
import com.rti.perftest.gen.TestData_tTypeCode;
import com.rti.perftest.harness.PerfTest;


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

        if (_isKeyed) {
            System.err.println("Using keyed Data.");

            if (_isDynamicData) {
                System.err.println("Using Dynamic Data.");
                PerfTest.runTest(
                        new RTIDDSImpl<DynamicData>(
                                new DynamicDataTypeHelper(
                                        TestDataKeyed_tTypeCode.VALUE,
                                        _isKeyed)),
                        argv);
            } else {
                PerfTest.runTest(
                        new RTIDDSImpl<TestDataKeyed_t>(
                                new DataTypeKeyedHelper()),
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
                                        _isKeyed)),
                        argv);
            } else {
                PerfTest.runTest(
                        new RTIDDSImpl<TestData_t>(new DataTypeHelper()),
                        argv);
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
            }
            if ("-dynamicData".toLowerCase().startsWith(argv[i].toLowerCase())) {
                // We can return here since we were just looking for this.
                _isDynamicData = true;
            }
        }
        return true;
    }

    private static boolean _isKeyed = false;
    private static boolean _isDynamicData = false;

}

// ===========================================================================
