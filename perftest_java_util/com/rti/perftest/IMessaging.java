/* $Id: IMessaging.java,v 1.1.2.1 2014/04/01 11:56:54 juanjo Exp $

(c) 2005-2012  Copyright, Real-Time Innovations, Inc.  All rights reserved.    	
Permission to modify and use for internal purposes granted.   	
This software is provided "as is", without warranty, express or implied.

modification history:
--------------------
29may09,jsr Added detection of wrong command line parameter
01apr08,rbw Follow Java naming conventions
01apr08,rbw Created
=========================================================================== */


package com.rti.perftest;


// ===========================================================================

/**
 * Runs the performance test.
 */
public interface IMessaging {
    public boolean initialize(int argc, String[] argv);

    public void printCmdLineHelp();

    public void shutdown();

    /**
     * If the implementation supports batching and the test scenario is
     * using batching, this function should return the size of the batch
     * in bytes.
     */
    public int getBatchSize();

    /* Used only for scan mode.
     * The maximum size of a message's binary payload. If the size
     * exceeds this during a scan, the test will stop.
     */
    public int getMaxBinDataSize();

    public IMessagingWriter createWriter(String topicName);
    
    /**
     * Pass null for callback if using IMessagingReader.ReceiveMessage()
     * to get data.
     */
    public IMessagingReader createReader(
            String topicName, IMessagingCB callback);
    
    public void dispose();

}

// ===========================================================================
// End of $Id: IMessaging.java,v 1.1.2.1 2014/04/01 11:56:54 juanjo Exp $
