/*
 * (c) 2005-2017  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */

package com.rti.perftest;


// ===========================================================================

/**
 * Callback for processing messages.
 */
public interface IMessagingCB {
    public void processMessage(TestMessage message);
}

// ===========================================================================
