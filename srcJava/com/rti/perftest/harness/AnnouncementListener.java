/*
 * (c) 2005-2017  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */

package com.rti.perftest.harness;

import java.util.ArrayList;
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
    private ArrayList<Integer> _finished_subscribers;
    public int announced_subscribers;

    // -----------------------------------------------------------------------
    // Public Methods
    // -----------------------------------------------------------------------

    // --- Constructors: -----------------------------------------------------

    public AnnouncementListener() {
        announced_subscribers = 0;
        _finished_subscribers = new ArrayList<Integer>();
    }

    // --- From IMessagingCB: ------------------------------------------------

    public void processMessage(TestMessage message) {
        if (!_finished_subscribers.contains(message.entity_id)) {
            _finished_subscribers.add(message.entity_id);
            announced_subscribers++;
        } else {
            announced_subscribers--;
        }
    }

}

// ===========================================================================
