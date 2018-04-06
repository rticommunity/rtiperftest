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
    public int announced_subscriber_replies;

    // -----------------------------------------------------------------------
    // Public Methods
    // -----------------------------------------------------------------------

    // --- Constructors: -----------------------------------------------------

    public AnnouncementListener() {
        announced_subscriber_replies = 0;
        _finished_subscribers = new ArrayList<Integer>();
    }

    // --- From IMessagingCB: ------------------------------------------------

    public void processMessage(TestMessage message) {
        /*
         * If the entity_id is not in the list of subscribers
         * that finished the test, add it.
         *
         * Independently, decrease announced_subscriber_replies if a known
         * writer responds to a message using this channel. We use
         * this as a way to check that all the readers have received
         * a message written by the Throughput writer.
         */
        if (!_finished_subscribers.contains(message.entity_id)) {
            _finished_subscribers.add(message.entity_id);
            announced_subscriber_replies++;
        } else {
            announced_subscriber_replies--;
        }
    }

}

// ===========================================================================
