/* $Id: PerfTestLauncher.java,v 1.1.2.1 2014/04/01 11:56:54 juanjo Exp $

(c) 2005-2012  Copyright, Real-Time Innovations, Inc.  All rights reserved.    	
Permission to modify and use for internal purposes granted.   	
This software is provided "as is", without warranty, express or implied.

modification history:
--------------------
08oct08,rbw Created
=========================================================================== */

package com.rti.perftest.ddsimpl;

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
        PerfTest.runTest(RTIDDSImpl.class, argv);
    }



    // -----------------------------------------------------------------------
    // Private Methods
    // -----------------------------------------------------------------------

    private PerfTestLauncher() {
        // do nothing
    }
    
}

// ===========================================================================
// End of $Id: PerfTestLauncher.java,v 1.1.2.1 2014/04/01 11:56:54 juanjo Exp $
