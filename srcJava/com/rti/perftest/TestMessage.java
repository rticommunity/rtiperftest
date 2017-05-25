/*
 * (c) 2005-2017  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */

package com.rti.perftest;


// ===========================================================================

/**
 * Message class.
 */
public class TestMessage {
    
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

