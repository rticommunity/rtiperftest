/* $Id: PerfTestLauncher.java,v 1.7 2015/05/09 12:59:22 jmorales Exp $

(c) 2005-2012  Copyright, Real-Time Innovations, Inc.  All rights reserved.    	
Permission to modify and use for internal purposes granted.   	
This software is provided "as is", without warranty, express or implied.

modification history:
--------------------
5.2.0,09may14,jm PERFTEST-86 Removing support for .ini file config.
5.1.0,26aug14,jm Added support for read the iskeyed option in perftest.ini.
5.1.0,11aug14,jm PERFTEST-69 Changes in the implementation.
08oct08,rbw Created
=========================================================================== */

package com.rti.perftest.ddsimpl;

import java.io.File;
import java.io.IOException;

import com.rti.perftest.gen.TestDataKeyed_t;
import com.rti.perftest.gen.TestData_t;
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
            PerfTest.runTest(new RTIDDSImpl<TestDataKeyed_t>(
                    new DataTypeKeyedHelper()), argv);
        } else {
            System.err.println("Using unkeyed Data.");
            PerfTest.runTest(new RTIDDSImpl<TestData_t>(new DataTypeHelper()),
                    argv);
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
        }
        return true;
    }

    private static boolean _isKeyed = false;

}

// ===========================================================================
// End of $Id: PerfTestLauncher.java,v 1.7 2015/05/09 12:59:22 jmorales Exp $
