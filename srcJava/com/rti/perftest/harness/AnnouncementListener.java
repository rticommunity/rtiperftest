/*
 * Copyright 2016
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

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
// End of $Id: AnnouncementListener.java,v 1.2 2014/01/10 11:19:08 juanjo Exp $
