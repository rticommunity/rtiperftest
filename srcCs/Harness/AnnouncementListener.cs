/*
 * (c) 2005-2021 Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */

using System.Collections.Generic;

namespace PerformanceTest
{
    public class AnnouncementListener : IMessagingCallback
    {
        public int announcedSubscriberReplies;
        private readonly List<int> finishedSubscribers = new List<int>();

        public void ProcessMessage(TestMessage message)
        {
            /*
             * If the entity_id is not in the list of subscribers
             * that finished the test, add it.
             *
             * Independently, decrease announced_subscriber_replies if a known
             * writer responds to a message using this channel. We use
             * this as a way to check that all the readers have received
             * a message written by the Throughput writer.
             */
            if (!finishedSubscribers.Contains(message.entityId))
            {
                finishedSubscribers.Add(message.entityId);
                announcedSubscriberReplies++;
            }
            else
            {
                announcedSubscriberReplies--;
            }
        }
    }
} // PerformanceTest Namespace
