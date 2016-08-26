/* $Id: TestMessage.java,v 1.1.2.1 2014/04/01 11:56:54 juanjo Exp $

(c) 2005-2012  Copyright, Real-Time Innovations, Inc.  All rights reserved.    	
Permission to modify and use for internal purposes granted.   	
This software is provided "as is", without warranty, express or implied.

modification history:
--------------------
04may08,hhw Added entity_id.
01apr08,rbw Created
=========================================================================== */


package com.rti.perftest;


// ===========================================================================

/**
 * Message class.
 */
public class TestMessage {
    public static final int MAX_DATA_SIZE = 63000;
    
    public byte[] data;
    public int    size;
    public byte[] key = new byte[4];
    public int    entity_id;
    public int    seq_num;
    public int    timestamp_sec;
    public int    timestamp_usec;
    public int    latency_ping;
}

// ===========================================================================
// End of $Id: TestMessage.java,v 1.1.2.1 2014/04/01 11:56:54 juanjo Exp $
