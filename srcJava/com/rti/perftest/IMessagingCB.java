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
// End of $Id: IMessagingCB.java,v 1.2 2014/01/10 11:19:08 juanjo Exp $
