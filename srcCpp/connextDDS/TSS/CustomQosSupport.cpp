#include "CustomQosSupport.hpp"

std::string stringValueQoS(DDS_Long resourceLimitValue) {
    if (resourceLimitValue == -1) {
        return "Unlimited";
    } else if (resourceLimitValue == -2) {
        return "Auto";
    } else {
        return perftest::to_string(resourceLimitValue);
    }
}

/* function for customizing domain participant qos */
DDS_Boolean
RTI_TSS_participant_qos(struct DDS_DomainParticipantQos *dp_qos, void *data)
{
    struct QoSBundle *ctx = (struct QoSBundle*) data;

#ifdef RTI_PERF_TSS_PRO
    // Set initial peers and not use multicast
    const std::vector<std::string> peerList =
            ctx->pm->get_vector<std::string>("peer");
    if (!peerList.empty()) {
        DDS_StringSeq_set_maximum(&dp_qos->discovery.initial_peers, 0);

        std::vector<char*> cstrings;
        cstrings.reserve(peerList.size());
        for(unsigned int i = 0; i < peerList.size(); ++i) {
            cstrings.push_back(const_cast<char*>(peerList[i].c_str()));
        }
        DDS_StringSeq_from_array(&dp_qos->discovery.initial_peers,
                (const char **)&cstrings[0],
                (long)peerList.size());
        DDS_StringSeq_set_length(
                &dp_qos->discovery.multicast_receive_addresses, 0);
    }
#elif defined(RTI_PERF_TSS_MICRO)
    DDS_StringSeq_set_maximum(&dp_qos->discovery.initial_peers, 0);

    const std::vector<std::string> peerList =
        ctx->pm->get_vector<std::string>("peer");
    if (!peerList.empty()) {
        DDS_StringSeq_set_maximum(
                &dp_qos->discovery.initial_peers, (int) peerList.size());
        DDS_StringSeq_set_length(
                &dp_qos->discovery.initial_peers, (int) peerList.size());
        for(unsigned int i = 0; i < peerList.size(); ++i) {
            *DDS_StringSeq_get_reference(&dp_qos->discovery.initial_peers, i)
                    = DDS_String_dup(peerList[i].c_str());
        }
    } else { /* Default discovery peers (unicast and multicast) */
        DDS_StringSeq_set_maximum(
                &dp_qos->discovery.initial_peers, 2);
        DDS_StringSeq_set_length(
                &dp_qos->discovery.initial_peers, 2);
        *DDS_StringSeq_get_reference(&dp_qos->discovery.initial_peers, 0)
                    = DDS_String_dup("239.255.0.1");
        *DDS_StringSeq_get_reference(&dp_qos->discovery.initial_peers, 1)
                    = DDS_String_dup("127.0.0.1");
    }
    dp_qos->discovery.accept_unknown_peers = DDS_BOOLEAN_TRUE;
#endif

    if (!PerftestConfigureTransport(*ctx->transport, *dp_qos, ctx->pm)) {
        return false;
    }

#ifdef RTI_PERF_TSS_MICRO
    RT_Registry_T *registry = NULL;
    struct DPDE_DiscoveryPluginProperty *discovery_plugin_properties =
            (DPDE_DiscoveryPluginProperty*)malloc(
                sizeof(DPDE_DiscoveryPluginProperty));

    DPDE_DiscoveryPluginProperty_initialize(discovery_plugin_properties);

    registry = DDS_DomainParticipantFactory_get_registry(
        DDS_DomainParticipantFactory_get_instance());

    if ((RT_Registry_lookup(registry,
    DDSHST_WRITER_DEFAULT_HISTORY_NAME) == NULL) &&
    !RT_Registry_register(registry, DDSHST_WRITER_DEFAULT_HISTORY_NAME,
    WHSM_HistoryFactory_get_interface(), NULL, NULL))
    {
        fprintf(stderr, "failed to register writer history\n");
        return DDS_BOOLEAN_FALSE;
    }

    if ((RT_Registry_lookup(registry,
    DDSHST_READER_DEFAULT_HISTORY_NAME) == NULL) &&
    !RT_Registry_register(registry, DDSHST_READER_DEFAULT_HISTORY_NAME,
    RHSM_HistoryFactory_get_interface(), NULL, NULL))
    {
        fprintf(stderr, "failed to register reader history\n");
        return DDS_BOOLEAN_FALSE;
    }

    #if FACE_COMPLIANCE_LEVEL_SAFETY_BASE_OR_STRICTER
    /* Safety Base or stricter must use non-auto DDS participant ID
    * ATTENTION: the participant_id below _must_ be modified to be unique
    * between all applications. Otherwise, new participants with the same ID
    * with an existing participant will fail to be created.
    */
    dp_qos->protocol.participant_id = 1;
    #endif /* FACE_COMPLIANCE_LEVEL_SAFETY_BASE_OR_STRICTER */
#endif

    /*
     * At this point, and not before is when we know the transport message size.
     */
    *ctx->maxUnfragmentedRTPSPayloadSize = ctx->transport->minimumMessageSizeMax - (MESSAGE_OVERHEAD_BYTES);

    if (!data_size_related_calculations(ctx)) {
        fprintf(stderr, "Failed to configure the data size settings\n");
        return DDS_BOOLEAN_FALSE;
    }

    return DDS_BOOLEAN_TRUE;
}

bool data_size_related_calculations(QoSBundle* ctx)
{
    /*
     * Check that the overhead is not bigger than the -dataLen, since we can not
     * send a lower size that the overhead of the test_type.
     */
    if (ctx->pm->get<unsigned long long>("dataLen")
            < perftest_cpp::OVERHEAD_BYTES) {
        fprintf(stderr,
                "The minimum dataLen allowed for this configuration is %d "
                "Bytes.\n",
                perftest_cpp::OVERHEAD_BYTES);
        /*
         * T::TypeSupport::get_type_name() can not be used since we do need
         * refractor RTIDDSImpl_FlatData class to properly inherit from a
         * templated class instead from a final class.
         */
        return false;
    }

    // If the message size max is lower than the datalen
    *ctx->isLargeData = (ctx->pm->get<unsigned long long>("dataLen") > *ctx->maxUnfragmentedRTPSPayloadSize);

    // Manage parameter -batchSize
    if (ctx->pm->get<long>("batchSize") > 0) {

        // Check if using asynchronous
        if (ctx->pm->get<bool>("asynchronous")) {
            if (ctx->pm->is_set("batchSize") && ctx->pm->get<long>("batchSize") != 0) {
                fprintf(stderr,
                        "Batching cannot be used with asynchronous writing.\n");
                return false;
            } else {
                ctx->pm->set<long>("batchSize", 0); // Disable Batching
            }
        }

        /*
         * Large Data + batching cannot be set. But batching is enabled by default,
         * so in that case, we just disabled batching, else, the customer set it up,
         * so we explitly fail
         */
        if (*ctx->isLargeData) {
            if (ctx->pm->is_set("batchSize") && ctx->pm->get<long>("batchSize") != 0) {
                fprintf(stderr, "Batching cannot be used with Large Data.\n");
                return false;
            } else {
                ctx->pm->set<long>("batchSize", -2);
            }
        } else if (((unsigned long)ctx->pm->get<long>("batchSize")
                        < ctx->pm->get<unsigned long long>("dataLen") * 2)) {
            /*
            * We don't want to use batching if the batch size is not large
            * enough to contain at least two samples (in this case we avoid the
            * checking at the middleware level).
            */
            if (ctx->pm->is_set("batchSize")) {
                /*
                * Batchsize disabled. A message will be print if batchSize < 0
                * in perftest_cpp::print_configuration()
                */
                ctx->pm->set<long>("batchSize", -1);
            } else {
                ctx->pm->set<long>("batchSize", 0); // Disable Batching
            }
        }
    }

    // Manage parameter -enableTurboMode
    if (ctx->pm->get<bool>("enableTurboMode")) {
        if (ctx->pm->get<bool>("asynchronous")) {
            fprintf(stderr, "Turbo Mode cannot be used with asynchronous writing.\n");
            return false;
        } if (ctx->isLargeData) {
            fprintf(stderr, "Turbo Mode disabled, using large data.\n");
            ctx->pm->set<bool>("enableTurboMode", false);
        }
    }

    return true;
}

/* function for customizing data writer qos */
DDS_Boolean
RTI_TSS_datawriter_qos(struct DDS_DataWriterQos *dw_qos, void *data)
{
    struct QoSBundle *ctx = (struct QoSBundle*) data;

#ifdef RTI_PERF_TSS_PRO
    if (ctx->pm->get<bool>("noPositiveAcks")
            && (strcmp(ctx->topic_name.c_str(), THROUGHPUT_TOPIC_NAME) == 0 || strcmp(ctx->topic_name.c_str(), LATENCY_TOPIC_NAME) == 0)) {
        dw_qos->protocol.disable_positive_acks = true;
        if (ctx->pm->is_set("keepDurationUsec")) {
            DDS_Duration_t keep_duration;
            keep_duration.sec = 0;
            keep_duration.nanosec =
                    ctx->pm->get<unsigned long long>("keepDurationUsec") * 1000;
            dw_qos->protocol.rtps_reliable_writer.disable_positive_acks_min_sample_keep_duration = keep_duration;
        }
    }

    if (*ctx->isLargeData || ctx->pm->get<bool>("asynchronous")) {
        dw_qos->publish_mode.kind = DDS_ASYNCHRONOUS_PUBLISH_MODE_QOS;
        if (ctx->pm->get<std::string>("flowController") != "default") {
            dw_qos->publish_mode.flow_controller_name =
                    DDS_String_dup(("dds.flow_controller.token_bucket." +
                    ctx->pm->get<std::string>("flowController")).c_str());
        }
    }

    dw_qos->resource_limits.initial_samples = ctx->pm->get<int>("sendQueueSize");
#endif

    if (strcmp(ctx->topic_name.c_str(), ANNOUNCEMENT_TOPIC_NAME) != 0)
    {
        if (ctx->pm->get<bool>("bestEffort"))
        {
            dw_qos->reliability.kind = DDS_BEST_EFFORT_RELIABILITY_QOS;
        }
        else
        {
            // default: use the setting specified in the qos profile
#ifdef RTI_PERF_TSS_MICRO
            dw_qos->reliability.kind = DDS_RELIABLE_RELIABILITY_QOS;
#endif
        }
    }
    else
    {
#ifdef RTI_PERF_TSS_MICRO
        dw_qos->reliability.kind = DDS_RELIABLE_RELIABILITY_QOS;
        dw_qos->durability.kind = DDS_TRANSIENT_LOCAL_DURABILITY_QOS;
#endif
    }

    if (strcmp(ctx->topic_name.c_str(), ANNOUNCEMENT_TOPIC_NAME) == 0
        || strcmp(ctx->topic_name.c_str(), THROUGHPUT_TOPIC_NAME) == 0) {
#ifdef RTI_PERF_TSS_PRO
        dw_qos->protocol.rtps_reliable_writer.low_watermark =
                ctx->pm->get<int>("sendQueueSize") * 1 / 10;
        dw_qos->protocol.rtps_reliable_writer.high_watermark =
                ctx->pm->get<int>("sendQueueSize") * 9 / 10;

        /*
         * If _SendQueueSize is 1 low watermark and high watermark would both be
         * 0, which would cause the middleware to fail. So instead we set the
         * high watermark to the low watermark + 1 in such case.
         */
        if (dw_qos->protocol.rtps_reliable_writer.high_watermark
                == dw_qos->protocol.rtps_reliable_writer.low_watermark) {
            dw_qos->protocol.rtps_reliable_writer.high_watermark =
                    dw_qos->protocol.rtps_reliable_writer.low_watermark + 1;
        }
#endif
    }

    if (strcmp(ctx->topic_name.c_str(), THROUGHPUT_TOPIC_NAME) == 0) {
#ifdef RTI_PERF_TSS_PRO
        if (ctx->pm->get<bool>("multicast")) {
            dw_qos->protocol.rtps_reliable_writer.enable_multicast_periodic_heartbeat =
                    RTI_TRUE;
        }

        if (ctx->pm->get<long>("batchSize") > 0) {
            dw_qos->batch.enable = true;
            dw_qos->batch.max_data_bytes = ctx->pm->get<long>("batchSize");
            dw_qos->resource_limits.max_samples = DDS_LENGTH_UNLIMITED;
            dw_qos->writer_resource_limits.max_batches =
                    ctx->pm->get<int>("sendQueueSize");
        } else {
            dw_qos->resource_limits.max_samples = ctx->pm->get<int>("sendQueueSize");
        }
#else
        dw_qos->resource_limits.max_samples = ctx->pm->get<int>("sendQueueSize");
#endif
        dw_qos->resource_limits.max_samples_per_instance =
                dw_qos->resource_limits.max_samples;

        dw_qos->durability.kind =
                (DDS_DurabilityQosPolicyKind)ctx->pm->get<int>("durability");

#ifdef RTI_PERF_TSS_PRO
        dw_qos->resource_limits.initial_samples =
                ctx->pm->get<int>("sendQueueSize");
        dw_qos->durability.direct_communication =
                !ctx->pm->get<bool>("noDirectCommunication");
#endif // RTI_PERF_TSS_PRO

        if (ctx->pm->get<unsigned long long>("dataLen") > DEFAULT_MESSAGE_SIZE_MAX) {
            dw_qos->protocol.rtps_reliable_writer.heartbeats_per_max_samples =
                    ctx->pm->get<int>("sendQueueSize");
        } else {
            dw_qos->protocol.rtps_reliable_writer.heartbeats_per_max_samples =
                    ctx->pm->get<int>("sendQueueSize") / 10;
        }

#ifdef RTI_PERF_TSS_MICRO
#ifdef PERFTEST_RTI_MICRO_24x_COMPATIBILITY
        dw_qos->history.kind = DDS_KEEP_LAST_HISTORY_QOS;
#else
        dw_qos->history.kind = DDS_KEEP_ALL_HISTORY_QOS;
#endif // PERFTEST_RTI_MICRO_24x_COMPATIBILITY

        dw_qos->history.depth = ctx->pm->get<int>("sendQueueSize");

        // Same values we use for Pro (See perftest_qos_profiles.xml).
        dw_qos->protocol.rtps_reliable_writer.heartbeat_period.sec = 0;
        dw_qos->protocol.rtps_reliable_writer.heartbeat_period.nanosec = 10000000;
#endif // RTI_PERF_TSS_PRO
    }

    if (strcmp(ctx->topic_name.c_str(), LATENCY_TOPIC_NAME)) {
        if (ctx->pm->get<bool>("noDirectCommunication")
            && (ctx->pm->get<int>("durability") == DDS_TRANSIENT_DURABILITY_QOS
            || ctx->pm->get<int>("durability") == DDS_PERSISTENT_DURABILITY_QOS))
        {
            dw_qos->durability.kind =
                    (DDS_DurabilityQosPolicyKind) ctx->pm->get<int>("durability");
#ifdef RTI_PERF_TSS_PRO
            dw_qos->durability.direct_communication =
                    !ctx->pm->get<bool>("noDirectCommunication");
#endif // RTI_PERF_TSS_PRO
        }

#ifdef RTI_PERF_TSS_PRO
        if (ctx->pm->get<unsigned long long>("dataLen") > DEFAULT_MESSAGE_SIZE_MAX) {
            dw_qos->protocol.rtps_reliable_writer.heartbeats_per_max_samples =
                ctx->pm->get<int>("sendQueueSize");
        } else {
            dw_qos->protocol.rtps_reliable_writer.heartbeats_per_max_samples =
                ctx->pm->get<int>("sendQueueSize") / 10;
        }

        if (ctx->pm->is_set("sendQueueSize")) {
            dw_qos->protocol.rtps_reliable_writer.max_send_window_size =
                    ctx->pm->get<int>("sendQueueSize");
            dw_qos->protocol.rtps_reliable_writer.min_send_window_size =
                    ctx->pm->get<int>("sendQueueSize");
        }
#endif // RTI_PERF_TSS_PRO
    }

    //One extra for MAX_CFT_VALUE
    dw_qos->resource_limits.max_instances =
            (DDS_Long)ctx->pm->get<long>("instances") + 1;

#ifdef RTI_PERF_TSS_PRO
    dw_qos->resource_limits.initial_instances =
            dw_qos->resource_limits.max_instances;

    // If is LargeData
    if (ctx->pm->get<int>("unbounded") != 0) {
        char buf[10];
        snprintf(buf, 10, "%d", ctx->pm->get<int>("unbounded"));
        DDSPropertyQosPolicyHelper::add_property(dw_qos->property,
               "dds.data_writer.history.memory_manager.fast_pool.pool_buffer_max_size",
               buf,
               false);
    } else {
        DDSPropertyQosPolicyHelper::add_property(dw_qos.property,
               "dds.data_writer.history.memory_manager.pluggable_allocator.underlying_allocator",
               "fast_buffer_pool",
               false);
    }

    if (ctx->pm->get<long>("instances") > 1) {
        if (ctx->pm->is_set("instanceHashBuckets")) {
            dw_qos->resource_limits.instance_hash_buckets =
                    ctx->pm->get<long>("instanceHashBuckets");
        } else {
            dw_qos->resource_limits.instance_hash_buckets =
                    ctx->pm->get<long>("instances");
        }
    }
#endif

    if (ctx->pm->get<bool>("showResourceLimits")
            && ctx->topic_name.c_str() != ANNOUNCEMENT_TOPIC_NAME) {
        std::ostringstream stringStream;

        stringStream << "Resource Limits DW ("
                    << ctx->topic_name.c_str()
                    << " topic):\n"
        // Samples
                    << "\tSamples ("
                #ifdef RTI_PERF_TSS_PRO
                    << "Initial/"
                #endif
                    << "Max): "
                #ifdef RTI_PERF_TSS_PRO
                    << stringValueQoS(dw_qos->resource_limits.initial_samples)
                    << "/"
                #endif
                    << stringValueQoS(dw_qos->resource_limits.max_samples)
                    << "\n";

        if (ctx->pm->get<bool>("keyed")) {
            // Instances
            stringStream << "\tInstances ("
                    #ifdef RTI_PERF_TSS_PRO
                        << "Initial/"
                    #endif
                        << "Max): "
                    #ifdef RTI_PERF_TSS_PRO
                        << stringValueQoS(dw_qos->resource_limits.initial_instances)
                        << "/"
                    #endif
                        << stringValueQoS(dw_qos->resource_limits.max_instances)
                        << "\n";

            // Samples per Instance
            stringStream << "\tMax Samples per Instance: "
                        << stringValueQoS(dw_qos->resource_limits.max_samples_per_instance)
                        << "\n";
        }

    #ifdef RTI_PERF_TSS_PRO
        // Batches
        if (dw_qos->batch.enable) {
            stringStream << "\tBatching Max Bytes: "
                        << stringValueQoS(dw_qos->batch.max_data_bytes)
                        << "\n"
                        << "\tBatching Max Batches: "
                        << stringValueQoS(dw_qos->writer_resource_limits.max_batches)
                        << "\n";
        }

        // Send Queue
        stringStream << "\tSend Queue (Min/Max): "
                    << stringValueQoS(
                        dw_qos->protocol.rtps_reliable_writer.min_send_window_size)
                    << "/"
                    << stringValueQoS(
                        dw_qos->protocol.rtps_reliable_writer.max_send_window_size)
                    << "\n";

      #ifdef RTI_FLATDATA_AVAILABLE
        // writer_loaned_sample_allocation
        if (_isFlatData) {
            stringStream << "\twriter_loaned_sample_allocation (initial_count/max_count): "
                        << stringValueQoS(
                            dw_qos->writer_resource_limits.writer_loaned_sample_allocation.initial_count)
                        << "/"
                        << stringValueQoS(
                            dw_qos->writer_resource_limits.writer_loaned_sample_allocation.max_count)
                        << "\n";
            // Property: pool_buffer_max_size
            stringStream << "\tfast_pool.pool_buffer_max_size: "
                        << stringValueQoS(
                            _isFlatData ? DDS_LENGTH_UNLIMITED : ctx->pm->get<int>("unbounded"))
                        << "\n";
        }
      #endif

        // Heartbeats per max samples
        stringStream << "\tHeartbeats per max samples: "
                    << stringValueQoS(
                        dw_qos->protocol.rtps_reliable_writer.heartbeats_per_max_samples)
                    << "\n";

    #endif

        // Heartbeats per max samples
        stringStream << "\tHeartbeat period (s/ns): "
                     << dw_qos->protocol.rtps_reliable_writer.heartbeat_period.sec 
                     << ", "
                     << dw_qos->protocol.rtps_reliable_writer.heartbeat_period.nanosec
                     << "\n";

        fprintf(stderr, "%s\n", stringStream.str().c_str());

    }

    return DDS_BOOLEAN_TRUE;
}

/* function for customizing data reader qos */
DDS_Boolean
RTI_TSS_datareader_qos(struct DDS_DataReaderQos *dr_qos, void *data)
{
    struct QoSBundle *ctx = (struct QoSBundle*) data;

#ifdef RTI_PERF_TSS_PRO
    dr_qos->resource_limits.initial_instances = 1;
#endif // RTI_PERF_TSS_PRO

    // Only force reliability on throughput/latency topics
    if (strcmp(ctx->topic_name.c_str(), ANNOUNCEMENT_TOPIC_NAME) != 0) {
        if (ctx->pm->get<bool>("bestEffort")) {
            dr_qos->reliability.kind = DDS_BEST_EFFORT_RELIABILITY_QOS;
        } else {
            dr_qos->reliability.kind = DDS_RELIABLE_RELIABILITY_QOS;
        }
    }

#ifdef RTI_PERF_TSS_PRO
    if (ctx->pm->is_set("receiveQueueSize")) {
        dr_qos->resource_limits.initial_samples =
                ctx->pm->get<int>("receiveQueueSize");
    }

    dr_qos->resource_limits.initial_instances =
            ctx->pm->get<long>("instances") + 1;
    if (ctx->instanceMaxCountReader != DDS_LENGTH_UNLIMITED) {
        ctx->instanceMaxCountReader++;
    }
#else
    /*
     * In micro we cannot have UNLIMITED instances, this means that we need to
     * increase the InstanceMaxCountReader (max instances for the dr) in all
     * cases
     */
    ctx->instanceMaxCountReader++;
#endif // RTI_PERF_TSS_PRO
    dr_qos->resource_limits.max_instances =
            (DDS_Long)ctx->instanceMaxCountReader;

#ifdef RTI_PERF_TSS_PRO
    if (ctx->pm->get<long>("instances") > 1) {
        if (ctx->pm->is_set("instanceHashBuckets")) {
            dr_qos->resource_limits.instance_hash_buckets =
                    ctx->pm->get<long>("instanceHashBuckets");
        } else {
            dr_qos->resource_limits.instance_hash_buckets =
                    ctx->pm->get<long>("instances");
        }
    }

    if (ctx->pm->get<bool>("noPositiveAcks")
            && (ctx->topic_name.c_str() == THROUGHPUT_TOPIC_NAME
            || ctx->topic_name.c_str() == LATENCY_TOPIC_NAME)) {
        dr_qos->protocol.disable_positive_acks = true;
    }
#endif

    if (ctx->topic_name.c_str() == THROUGHPUT_TOPIC_NAME
            || (ctx->topic_name.c_str() == LATENCY_TOPIC_NAME
            && ctx->pm->get<bool>("noDirectCommunication")
            && (ctx->pm->get<int>("durability") == DDS_TRANSIENT_DURABILITY_QOS
            || ctx->pm->get<int>("durability") == DDS_PERSISTENT_DURABILITY_QOS))) {
        dr_qos->durability.kind =
                (DDS_DurabilityQosPolicyKind) ctx->pm->get<int>("durability");
#ifndef RTI_PERF_TSS_MICRO
        dr_qos->durability.direct_communication =
                !ctx->pm->get<bool>("noDirectCommunication");
#endif

    }

#ifdef RTI_PERF_TSS_PRO
    if (ctx->pm->get<bool>("multicast") && ctx->transport->allowsMulticast()) {
        DDS_TransportMulticastSettingsSeq_ensure_length(
                &dr_qos->multicast.value, 1, 1);
        DDS_TransportMulticastSettingsSeq_get_reference(
                &dr_qos->multicast.value, 0)->receive_address = DDS_String_dup(
                    ctx->transport->getMulticastAddr(ctx->topic_name.c_str()).c_str());

        if (DDS_TransportMulticastSettingsSeq_get_reference(
                &dr_qos->multicast.value, 0)->receive_address == NULL) {
            fprintf(stderr,
                    "topic name must either be %s or %s or %s.\n",
                    THROUGHPUT_TOPIC_NAME,
                    LATENCY_TOPIC_NAME,
                    ANNOUNCEMENT_TOPIC_NAME);
            return false;
        }

        DDS_TransportMulticastSettingsSeq_get_reference(
                &dr_qos->multicast.value, 0)->receive_port = 0;
        DDS_StringSeq_set_length(
            &DDS_TransportMulticastSettingsSeq_get_reference(
                    &dr_qos->multicast.value, 0)->transports, 0);
    }
#endif

#ifdef RTI_PERF_TSS_MICRO
    if (strcmp(ctx->topic_name.c_str(), THROUGHPUT_TOPIC_NAME) == 0) {
        /*
         * For Connext DDS Pro settings are set so initial samples are set to
         * a lower value than max_samples, so we can grow if needed. For micro
         * however we do not have the initial_samples parameter, therefore we
         * must choose a value for max_samples since the beginning. We chose to
         * use 10000. This value should be large enough to handle most of the
         * communications.
         *
         * We could potentially modify this with a new command line parameter
         */
        if (ctx->pm->get<unsigned long long>("dataLen") > MAX_BOUNDED_SEQ_SIZE) {
            dr_qos->resource_limits.max_samples = 50;
            dr_qos->resource_limits.max_samples_per_instance = 50;
            dr_qos->history.depth = 50;
        }
        else {
            dr_qos->resource_limits.max_samples = 5000;
            dr_qos->resource_limits.max_samples_per_instance = 5000;
            dr_qos->history.depth = 5000;
        }
        /*
         * In micro 2.4.x we don't have keep all, this means we need to set the
         * history to keep last and chose a history depth. For the depth value
         * we can we same value as max_samples
         */
        #if PERFTEST_RTI_MICRO_24x_COMPATIBILITY
          // Keep all not supported in Micro 2.4.x
          dr_qos->history.kind = DDS_KEEP_LAST_HISTORY_QOS;
        #else
          dr_qos->history.kind = DDS_KEEP_ALL_HISTORY_QOS;
        #endif

    } else { // "LatencyQos" or "AnnouncementQos"

        /*
         * By default Micro will use a max_samples = 1. This is too low for the
         * initial burst of data. In pro we set this value via QoS to
         * LENGTH_UNLIMITED. In Micro we will use a lower number due to
         * memory restrictions.
         */
        if (ctx->pm->get<unsigned long long>("dataLen") > MAX_BOUNDED_SEQ_SIZE) {
            dr_qos->resource_limits.max_samples = 50;
        }
        else {
            dr_qos->resource_limits.max_samples = 5000;
        }
    }

    /*
     * We could potentially use here the number of subscriber, right now this
     * class does not have access to the number of subscriber though.
     */
    dr_qos->reader_resource_limits.max_remote_writers = 50;
    dr_qos->reader_resource_limits.max_remote_writers_per_instance = 50;

    if (ctx->pm->get<bool>("multicast")) {

        if (ctx->transport->getMulticastAddr(ctx->topic_name.c_str()).empty()) {
            fprintf(stderr,
                    "topic name must either be %s or %s or %s.\n",
                    THROUGHPUT_TOPIC_NAME,
                    LATENCY_TOPIC_NAME,
                    ANNOUNCEMENT_TOPIC_NAME);
            return false;
        }

        std::string receive_address = "_udp://" + ctx->transport->getMulticastAddr(ctx->topic_name.c_str());
        DDS_StringSeq_set_maximum(&dr_qos->transport.enabled_transports, 1);
        DDS_StringSeq_set_length(&dr_qos->transport.enabled_transports, 1);
        *DDS_StringSeq_get_reference(&dr_qos->transport.enabled_transports, 0) =
                DDS_String_dup(receive_address.c_str());

    }

    if (ctx->pm->get<bool>("showResourceLimits")
            && ctx->topic_name.c_str() != ANNOUNCEMENT_TOPIC_NAME)
    {
        std::ostringstream stringStream;

        stringStream << "Resource Limits DR ("
                    << ctx->topic_name.c_str()
                    << " topic):\n"
        // Samples
                    << "\tSamples ("
                #ifdef RTI_PERF_TSS_PRO
                    << "Initial/"
                #endif
                    << "Max): "
                #ifdef RTI_PERF_TSS_PRO
                    << stringValueQoS(dr_qos->resource_limits.initial_samples)
                    << "/"
                #endif
                    << stringValueQoS(dr_qos->resource_limits.max_samples)
                    << "\n";

        if (ctx->pm->get<bool>("keyed")){
            // Instances
            stringStream << "\tInstances ("
                    #ifdef RTI_PERF_TSS_PRO
                        << "Initial/"
                    #endif
                        << "Max): "
                    #ifdef RTI_PERF_TSS_PRO
                        << stringValueQoS(dr_qos->resource_limits.initial_instances)
                        << "/"
                    #endif
                        << stringValueQoS(dr_qos->resource_limits.max_instances)
                        << "\n";

            // Samples per Instance
            stringStream << "\tMax Samples per Instance: "
                        << stringValueQoS(dr_qos->resource_limits.max_samples_per_instance)
                        << "\n";
        }

        fprintf(stderr, "%s\n", stringStream.str().c_str());
    }
#endif //RTI_PERF_TSS_MICRO

    if (ctx->pm->get<int>("unbounded") != 0)
    {
      #ifdef RTI_PERF_TSS_PRO
        char buf[10];
        snprintf(buf, 10, "%d", ctx->pm->get<int>("unbounded"));
        DDSPropertyQosPolicyHelper::add_property(dr_qos->property,
                "dds.data_reader.history.memory_manager.fast_pool.pool_buffer_max_size",
                buf, false);
      #else
        /* This is only needed for Micro 2.4.x. False unbounded sequences are
         * available in Micro 3.0 */
        #if PERFTEST_RTI_MICRO_24x_COMPATIBILITY
          fprintf(stderr,
                  "Unbounded sequences not supported on Micro.\n");
          return false;
        #endif
      #endif
    }

    return DDS_BOOLEAN_TRUE;
}