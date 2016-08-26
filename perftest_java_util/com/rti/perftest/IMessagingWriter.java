/* $Id: IMessagingWriter.java,v 1.2 2014/01/10 11:19:08 juanjo Exp $

(c) 2005-2012  Copyright, Real-Time Innovations, Inc.  All rights reserved.    	
Permission to modify and use for internal purposes granted.   	
This software is provided "as is", without warranty, express or implied.

modification history:
--------------------
09jul10,jsr Added new waitForPingResponse method
03may10,jsr Added waitForPingResponse and notifyPingResponse
01apr08,rbw Follow Java naming conventions
01apr08,rbw Created
=========================================================================== */


package com.rti.perftest;

import java.util.concurrent.TimeUnit;


// ===========================================================================

/**
 * Send messages.
 */
public interface IMessagingWriter {
    public void waitForReaders(int numReaders);

    public boolean send(TestMessage message);
    public boolean waitForPingResponse();
    public boolean waitForPingResponse(long timeout, TimeUnit unit);
    public boolean notifyPingResponse();

    public void flush();

}

// ===========================================================================
// End of $Id: IMessagingWriter.java,v 1.2 2014/01/10 11:19:08 juanjo Exp $
