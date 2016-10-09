/*
 * (c) 2005-2016  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */

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

