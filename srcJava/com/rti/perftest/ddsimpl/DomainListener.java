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

package com.rti.perftest.ddsimpl;

import com.rti.dds.domain.DomainParticipantAdapter;
import com.rti.dds.publication.DataWriter;
import com.rti.dds.publication.OfferedIncompatibleQosStatus;
import com.rti.dds.subscription.DataReader;
import com.rti.dds.subscription.RequestedIncompatibleQosStatus;
import com.rti.dds.topic.InconsistentTopicStatus;
import com.rti.dds.topic.Topic;


// ===========================================================================

/**
 * Logs callbacks that come to a participant.
 */
/*package*/ final class DomainListener extends DomainParticipantAdapter {
    // --- From DomainParticipantListener: -----------------------------------

    @Override
    public void on_inconsistent_topic(
            Topic topic, InconsistentTopicStatus status) {

        System.out.println(
                "Found inconsistent topic. Expecting " +
                topic.get_name() +
                " of type " +
                topic.get_type_name());
    }


    @Override
    public void on_offered_incompatible_qos(
            DataWriter writer, OfferedIncompatibleQosStatus status) {

        System.out.println(
                "Found incompatible reader for writer " +
                writer.get_topic().get_name() +
                "  QoS is " +
                status.last_policy_id);
    }


    @Override
    public void on_requested_incompatible_qos(
            DataReader reader, RequestedIncompatibleQosStatus status) {

        System.out.println(
                "Found incompatible writer for reader " +
                reader.get_topicdescription().get_name() +
                "  QoS is " +
                status.last_policy_id);
    }

}

// ===========================================================================
// End of $Id: DomainListener.java,v 1.3 2014/09/02 13:53:33 jmorales Exp $
