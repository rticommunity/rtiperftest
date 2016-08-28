/*
 * (c) 2005-2016  Copyright, Real-Time Innovations, Inc.  All rights reserved.
 * Permission to modify and use for internal purposes granted.
 * This software is provided "as is", without warranty, express or implied.
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
// End of $Id$
