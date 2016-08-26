/* $Id: IMessagingReader.java,v 1.1.2.1 2014/04/01 11:56:54 juanjo Exp $

(c) 2005-2012  Copyright, Real-Time Innovations, Inc.  All rights reserved.    	
Permission to modify and use for internal purposes granted.   	
This software is provided "as is", without warranty, express or implied.

modification history:
--------------------
04may08,hhw Added waitForWriters();
01apr08,rbw Follow Java naming conventions
01apr08,rbw Created
=========================================================================== */


package com.rti.perftest;


// ===========================================================================

/**
 * Processes received messages.
 */
public interface IMessagingReader {
    /**
     * Only used for non-callback test.
     */
    public TestMessage receiveMessage();

    public void waitForWriters(int numWriters);

    /**
     * Only used for non-callback test to cleanup the thread.
     */
    public void shutdown();

}

// ===========================================================================
// End of $Id: IMessagingReader.java,v 1.1.2.1 2014/04/01 11:56:54 juanjo Exp $
