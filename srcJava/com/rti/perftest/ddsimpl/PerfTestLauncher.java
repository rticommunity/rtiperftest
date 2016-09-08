/*
 * Copyright 2016
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

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
