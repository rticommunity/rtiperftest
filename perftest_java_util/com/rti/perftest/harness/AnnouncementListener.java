/* $Id: AnnouncementListener.java,v 1.1.2.1 2014/04/01 11:56:54 juanjo Exp $

(c) 2005-2012  Copyright, Real-Time Innovations, Inc.  All rights reserved.    	
Permission to modify and use for internal purposes granted.   	
This software is provided "as is", without warranty, express or implied.

 modification history:
 --------------------
 04may08,hhw Created.
=========================================================================== */

package com.rti.perftest.harness;

import com.rti.perftest.IMessagingCB;
import com.rti.perftest.TestMessage;


//===========================================================================

/**
 * Data listener for the Announcement
 *
 * Receives an announcement message from a Subscriber once
 * the subscriber has discovered every Publisher.
 */
/*package*/ final class AnnouncementListener implements IMessagingCB {
    // -----------------------------------------------------------------------
    // Public Fields
    // -----------------------------------------------------------------------

    public int announced_subscribers;

    // -----------------------------------------------------------------------
    // Public Methods
    // -----------------------------------------------------------------------

    // --- Constructors: -----------------------------------------------------

    public AnnouncementListener() {
        announced_subscribers = 0;
    }

    // --- From IMessagingCB: ------------------------------------------------

    public void processMessage(TestMessage message) {
        announced_subscribers++;
    }

}

// ===========================================================================
// End of $Id: AnnouncementListener.java,v 1.1.2.1 2014/04/01 11:56:54 juanjo Exp $
