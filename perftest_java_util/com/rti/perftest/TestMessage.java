/* $Id: TestMessage.java,v 1.4 2014/09/22 16:28:50 jmorales Exp $

(c) 2005-2012  Copyright, Real-Time Innovations, Inc.  All rights reserved.    	
Permission to modify and use for internal purposes granted.   	
This software is provided "as is", without warranty, express or implied.

modification history:
--------------------
5.1.0,22sep14,jm  PERFTEST-75 Fixed LargeData + Turbo-Mode. Changing max size to
                  131072.
5.1.0,16sep14,jm  PERFTEST-60 PERFTEST-66 Large data support 
                  added for perftest.
04may08,hhw Added entity_id.
01apr08,rbw Created
=========================================================================== */


package com.rti.perftest;


// ===========================================================================

/**
 * Message class.
 */
public class TestMessage {
    public static final int MAX_SYNCHRONOUS_SIZE = 63000;
    public static final int MAX_DATA_SIZE = 131072;
    
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
// End of $Id: TestMessage.java,v 1.4 2014/09/22 16:28:50 jmorales Exp $
