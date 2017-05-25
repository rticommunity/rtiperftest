/*
 * (c) 2005-2017  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
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

/*
 * Logs callbacks that come to a participant.
 */

final class DomainListener extends DomainParticipantAdapter {

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
