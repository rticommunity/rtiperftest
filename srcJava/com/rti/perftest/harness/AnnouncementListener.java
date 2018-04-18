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
    public ArrayList<Integer> subscriber_list;

    // -----------------------------------------------------------------------
    // Public Methods
    // -----------------------------------------------------------------------

    // --- Constructors: -----------------------------------------------------

    public AnnouncementListener() {
        subscriber_list = new ArrayList<Integer>();
    }

    // --- From IMessagingCB: ------------------------------------------------

    public void processMessage(TestMessage message) {
        /*
         * The subscriber_list vector contains the list of discovered subscribers.
         *
         * - If the message.size is INITIALIZE or LENGTH_CHANGED and the
         *   subscriber is not in the list, it will be added.
         * - If the message.size is FINISHED_SIZE and the
         *   subscriber is in the list, it will be removed.
         *
         * The publisher access to this list to verify:
         * - If all the subscribers are discovered or notified about the length
         *   being changed.
         * - If all the subscribers are notified that the test has finished.
         */
        if ((message.size == PerfTest.INITIALIZE_SIZE
                || message.size == PerfTest.LENGTH_CHANGED_SIZE)
                && !subscriber_list.contains(message.entity_id)) {
            subscriber_list.add(message.entity_id);
        } else if (message.size == PerfTest.FINISHED_SIZE) {
            subscriber_list.remove(new Integer(message.entity_id));
        }
    }

}

// ===========================================================================
