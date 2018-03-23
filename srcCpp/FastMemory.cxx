#include "FastMemory.h"

#define NDDS_WRITERHISTORY_FAST_MEMORY_PLUGIN_CLASSID (3)

struct WriterHistoryFastMemorySample {
    struct NDDS_WriterHistory_Sample parent;
};

struct WriterHistoryFastMemory {
    struct NDDS_WriterHistory_Listener _listener;
    struct NDDS_WriterHistory_Property _property;
    RTIBool _storeFilterStatus;
    struct REDASequenceNumber _nextSn;
    struct REDAFastBufferPool *_sampleInlineQosPool;
    struct RTIEncapsulationInfo _encapsulationInfo;

    /* With BEST-EFFORT volatile there is only one outstanding sample. No
     * need to create a FB
     */
    struct WriterHistoryFastMemorySample _sample;
    struct MIGSerializedData _serData;
    int _serBufferLength;
    char *_serBuffer;
};

RTI_INT32 WriterHistoryFastMemoryPlugin_pruneLifespanExpiredSamples(
    struct NDDS_WriterHistory_Plugin * self,
    RTIBool * samplesReusable_out,
    NDDS_WriterHistory_Handle history_in,
    const struct RTINtpTime *now_in,
    RTIBool singleSample_in)
{
    if (samplesReusable_out != NULL) {
        *samplesReusable_out = RTI_FALSE;
    }
    return NDDS_WRITERHISTORY_RETCODE_OK;
}

RTI_INT32 WriterHistoryFastMemoryPlugin_registerInstance(
    struct NDDS_WriterHistory_Plugin * self,
    NDDS_WriterHistory_Handle history_in,
    const struct MIGRtpsKeyHash * instanceKeyHash_in,
    const void * instanceData_in,    
    const struct RTINtpTime * timestamp_in,
    const struct RTINtpTime * now_in)
{
    return NDDS_WRITERHISTORY_RETCODE_OK;
}

RTI_INT32 WriterHistoryFastMemoryPlugin_setDurableSubscriptions(
        struct NDDS_WriterHistory_Plugin *self,
        RTIBool *sampleReusable_out,
        NDDS_WriterHistory_Handle history_in,
        struct NDDS_WriterHistory_EndpointGroup *groups,
        int length)
{
    if (sampleReusable_out != NULL) {
        *sampleReusable_out = RTI_FALSE;
    }
    
    return NDDS_WRITERHISTORY_RETCODE_OK;
}

RTI_INT32 WriterHistoryFastMemoryPlugin_getNextSn(
        struct NDDS_WriterHistory_Plugin * self,
        struct REDASequenceNumber sn_out[],
        NDDS_WriterHistory_Handle history_in,
        RTI_UINT32 sessionCount_in,
        RTI_INT32 sessionId_in[])
{
    struct WriterHistoryFastMemory * wh = (struct WriterHistoryFastMemory *)history_in;
    sn_out[0] = wh->_nextSn;
    return NDDS_WRITERHISTORY_RETCODE_OK;
}

RTI_INT32 WriterHistoryFastMemoryPlugin_getFirstNonReclaimableSn(
    struct NDDS_WriterHistory_Plugin * self,
    struct REDASequenceNumber sn_out[],
    struct RTINtpTime timestamp_out[],
    NDDS_WriterHistory_Handle history_in,
    RTI_UINT32 sessionCount_in,
    const RTI_INT32 sessionId_in[])
{
    struct WriterHistoryFastMemory *wh = (struct WriterHistoryFastMemory *)history_in;
    sn_out[0] = wh->_nextSn;
    return NDDS_WRITERHISTORY_RETCODE_OK;
}

RTI_INT32 WriterHistoryFastMemoryPlugin_getFirstAvailableSn(
    struct NDDS_WriterHistory_Plugin * self,
    struct REDASequenceNumber sn_out[],
    struct REDASequenceNumber virtualSn_out[],
    NDDS_WriterHistory_Handle history_in,
    RTI_UINT32 sessionCount_in,
    const RTI_INT32 sessionId_in[])
{
    const char *METHOD_NAME = RTI_FUNCTION_NAME;
    struct WriterHistoryFastMemory *wh = (struct WriterHistoryFastMemory *)history_in;

    sn_out[0] = wh->_nextSn;
    virtualSn_out[0] = wh->_nextSn;

    return NDDS_WRITERHISTORY_RETCODE_OK;
}

RTI_INT32 WriterHistoryFastMemoryPlugin_getLastAvailableSn(
    struct NDDS_WriterHistory_Plugin * self,
    struct REDASequenceNumber sn_out[],
    NDDS_WriterHistory_Handle history_in,
    RTI_UINT32 sessionCount_in,
    const RTI_INT32 sessionId_in[])
{
    struct WriterHistoryFastMemory *wh = (struct WriterHistoryFastMemory *)history_in;

    sn_out[0] = wh->_nextSn;
    REDASequenceNumber_minusminus(&sn_out[0]);
    return NDDS_WRITERHISTORY_RETCODE_OK;
}

RTI_INT32 WriterHistoryFastMemoryPlugin_returnSampleLoan(
    struct NDDS_WriterHistory_Plugin * self,
    RTIBool * sampleReusable_out,
    NDDS_WriterHistory_Handle history_in,
    struct NDDS_WriterHistory_Sample * sample_in,
    const struct RTINtpTime * now_in)
{
    RTI_INT32 failReason = NDDS_WRITERHISTORY_RETCODE_FAILURE_INTERNAL;
    RTI_INT32 localFailReason = NDDS_WRITERHISTORY_RETCODE_FAILURE_INTERNAL;
    const char *METHOD_NAME = RTI_FUNCTION_NAME;
    struct WriterHistoryFastMemory *wh = (struct WriterHistoryFastMemory *)history_in;
    
    if (wh->_listener.finalize_sample != NULL) {
        localFailReason = wh->_listener.finalize_sample(
                            &wh->_listener,
                            sample_in);
    
        if (failReason != NDDS_WRITERHISTORY_RETCODE_OK) {
            goto done;
        }
    }

    *sampleReusable_out = RTI_TRUE;
    failReason = NDDS_WRITERHISTORY_RETCODE_OK;
  done:
    return failReason;
}

RTI_INT32 WriterHistoryFastMemoryPlugin_addSample(
    struct NDDS_WriterHistory_Plugin * self,
    RTI_UINT32 * sampleCount, /*out*/
    struct NDDS_WriterHistory_Sample * sample_out[],
    RTI_INT32 sessionId_out[],
    struct REDASequenceNumber firstAvailableSn_out[],
    struct REDASequenceNumber firstAvailableVirtualSn_out[],
    NDDS_WriterHistory_Handle history_in,
    NDDS_WriterHistory_SampleKind kind_in,
    const struct MIGRtpsKeyHash * instanceKeyHash_in,
    const struct RTINtpTime * timestamp_in,
    MIGGeneratorEndian endian_in,
    const void * userData_in,
    struct REDABuffer *cookie_in,
    const struct COMMENDFilterStatus * filterStatus_in,
    const struct MIGRtpsGuid * readerGuid_in,
    const struct NDDS_WriterHistory_OriginalWriterInfo * originalWriterInfo_in,
    const struct NDDS_WriterHistory_OriginalWriterInfo * relatedOriginalWriterInfo_in,
    const struct RTINtpTime * now_in,
    RTI_INT32 publicationPriority,
    RTIBool createInstance,
    RTI_INT32 sample_flags,
    const struct NDDS_WriterHistory_WriteParams * write_params)
{
    RTI_INT32 failReason = NDDS_WRITERHISTORY_RETCODE_FAILURE_INTERNAL;
    RTI_INT32 localFailReason = NDDS_WRITERHISTORY_RETCODE_FAILURE_INTERNAL;
    const char *METHOD_NAME = RTI_FUNCTION_NAME;
    struct WriterHistoryFastMemory *wh = (struct WriterHistoryFastMemory *)history_in;
    struct WriterHistoryFastMemorySample *sample;
    
    sample = &wh->_sample;
    
    sample->parent.sample.vSn = wh->_nextSn;
    sample->parent.sample.sn = wh->_nextSn;
    REDASequenceNumber_plusplus(&wh->_nextSn);
    sample->parent.kind = kind_in;
    sample->parent.sample.protocolParameters.pointer = NULL;
    sample->parent.sample.protocolParameters.length = 0;
    
    sample->parent.sample.serializedData->serializedData.length =
            wh->_serBufferLength;
    sample->parent.sample.publicationPriority = publicationPriority;
    sample->parent.sample.timestamp = *timestamp_in;

    if (wh->_storeFilterStatus) {
        COMMENDFilterStatus_copy(&sample->parent.filterStatus, filterStatus_in);
    }

    if (readerGuid_in == NULL) {
        struct MIGRtpsGuid unknownGuid = MIG_RTPS_GUID_UNKNOWN;
        sample->parent.sample.readerGuid = unknownGuid;
    } else {
        sample->parent.sample.readerGuid = *readerGuid_in;
    }

    localFailReason = wh->_listener.initialize_sample(
        &wh->_listener,
        (struct NDDS_WriterHistory_Sample *)sample,
        NULL,
        RTI_TRUE,
        endian_in,
        originalWriterInfo_in,
        relatedOriginalWriterInfo_in,
        sample_flags,
        write_params,
        userData_in);

    if (localFailReason != NDDS_WRITERHISTORY_RETCODE_OK) {
        goto done;
    }
    
    sample_out[0] = (struct NDDS_WriterHistory_Sample *)sample;
    *sampleCount = 1;
    *sessionId_out = 0;
    firstAvailableSn_out[0] = sample->parent.sample.sn;
    firstAvailableVirtualSn_out[0] = sample->parent.sample.sn;    
    
    failReason = NDDS_WRITERHISTORY_RETCODE_OK;
  done:
    return failReason;
}

RTI_INT32 WriterHistoryFastMemoryPlugin_destroyHistory(
    struct NDDS_WriterHistory_Plugin * self,
    NDDS_WriterHistory_Handle history_in)
{
    const char *METHOD_NAME = RTI_FUNCTION_NAME;
    struct WriterHistoryFastMemory *wh = (struct WriterHistoryFastMemory *)history_in;

    COMMENDFilterStatus_finalize(&wh->_sample.parent.filterStatus);

    if (wh->_sampleInlineQosPool != NULL) {
        REDAFastBufferPool_delete(wh->_sampleInlineQosPool);
    }
    
    if (wh->_serBuffer != NULL) {
        RTIOsapiHeap_freeBufferAligned(wh->_serBuffer);
    }
    
    NDDS_WriterHistory_Property_finalize(&wh->_property);
    RTIOsapiMemory_zero(wh, sizeof(struct WriterHistoryFastMemory));

    RTIOsapiHeap_freeStructure(wh);
    return NDDS_WRITERHISTORY_RETCODE_OK;    
}

RTI_INT32 WriterHistoryFastMemoryPlugin_createHistory (
        struct NDDS_WriterHistory_Plugin * self,
        NDDS_WriterHistory_Handle * history_out,
        const struct NDDS_WriterHistory_Property * property_in,
        const struct NDDS_WriterHistory_Listener * historyListener_in,
        const struct MIGRtpsGuid * dwGuid_in,
        const struct MIGRtpsGuid * dwVirtualGuid_in,
        const struct NDDS_WriterHistory_AttributeSeq * dwProperty,
        struct RTIClock * clock_in,
        void * reserved)
{
    RTI_INT32 failReason = NDDS_WRITERHISTORY_RETCODE_FAILURE_INTERNAL;
    const char *METHOD_NAME = RTI_FUNCTION_NAME;
    struct WriterHistoryFastMemory *wh = NULL;
    struct REDAFastBufferPoolProperty poolP =
        REDA_FAST_BUFFER_POOL_PROPERTY_DEFAULT;
    struct REDASequenceNumber unknownSn = REDA_SEQUENCE_NUMBER_UNKNOWN;
    struct COMMENDFilterStatus defaultFilterStatus = COMMEND_FILTER_STATUS_NEW;

    *history_out = NULL;
    
    RTIOsapiHeap_allocateStructure(&wh, struct WriterHistoryFastMemory);
    
    if (wh == NULL) {
        return NDDS_WRITERHISTORY_RETCODE_FAILURE_INTERNAL;
    }

    RTIOsapiMemory_zero(wh,sizeof(struct WriterHistoryFastMemory));
    wh->_sample.parent.filterStatus = defaultFilterStatus;
    REDASequenceNumber_setZero(&wh->_nextSn);
    REDASequenceNumber_plusplus(&wh->_nextSn);
    NDDS_WriterHistory_Property_initialize(&wh->_property);
    NDDS_WriterHistory_Property_copy(&wh->_property, property_in);
    
    poolP.growth.initial = property_in->sampleCount.initial;
    poolP.growth.maximal = property_in->sampleCount.maximal;
    poolP.growth.increment = REDA_FAST_BUFFER_POOL_UNLIMITED;

    wh->_sampleInlineQosPool = 
            REDAFastBufferPool_new(
                    property_in->sampleInlineQosDataSize.size,
                    property_in->sampleInlineQosDataSize.alignment,
                    &poolP);

    if (wh->_sampleInlineQosPool == NULL) {
        goto done;
    }
    
    wh->_encapsulationInfo = property_in->encapsulationInfo[0];

    RTIOsapiHeap_allocateBufferAligned(
            &wh->_serBuffer,
            property_in->sampleUserDataSize[0].size,
            property_in->sampleUserDataSize[0].alignment);
    
    if (wh->_serBuffer == NULL) {
        goto done;
    }
    
    wh->_serBufferLength = property_in->sampleUserDataSize[0].size;
    wh->_serData.serializedData.pointer = wh->_serBuffer;
    wh->_serData.encapsulationId = wh->_encapsulationInfo.encapsulationId;
    wh->_listener = *historyListener_in;
    
    /* Initialize immutable fields in sample */
    wh->_sample.parent.sample.sessionId = 0;
    wh->_sample.parent.sample.protocolPool = wh->_sampleInlineQosPool;
    wh->_sample.parent.sample.serializedData = &wh->_serData;
    MIGRtpsKeyHash_setDefault(&wh->_sample.parent.sample.objectKeyHash);
    wh->_sample.parent.sample.setFirstSn = unknownSn;

    if (!property_in->storeFilterStatus ||
        ((RTI_INT32)property_in->maxRemoteReaderFilters == REDA_FAST_BUFFER_POOL_UNLIMITED)) {
        wh->_storeFilterStatus = RTI_FALSE;
    } else {
        wh->_storeFilterStatus = RTI_TRUE;
    }

    if (wh->_storeFilterStatus) {
        COMMENDFilterStatus_init(
                &wh->_sample.parent.filterStatus,
                property_in->maxRemoteReaderFilters);
    }

    *history_out = wh;
    failReason = NDDS_WRITERHISTORY_RETCODE_OK;
  done: 
    if (failReason != NDDS_WRITERHISTORY_RETCODE_OK 
            && wh != NULL) {
        WriterHistoryFastMemoryPlugin_destroyHistory(self, wh);
    }
    return failReason;
}

RTI_INT32 WriterHistoryFastMemoryPlugin_initialize(
        struct NDDS_WriterHistory_Plugin * self,
        const struct NDDS_WriterHistory_AttributeSeq * dpProperties_in,
        void * reserved)
{
    return NDDS_WRITERHISTORY_RETCODE_OK;
}

RTI_INT32 WriterHistoryFastMemoryPlugin_destroy(
        struct NDDS_WriterHistory_Plugin * self)
{
    RTIOsapiHeap_freeStructure(self);
    return NDDS_WRITERHISTORY_RETCODE_OK;
}

RTI_INT32 NDDS_WriterHistory_FastMemoryPlugin_create(
        struct NDDS_WriterHistory_Plugin ** plugin)
{
    const char *METHOD_NAME = RTI_FUNCTION_NAME;
    struct NDDS_WriterHistory_Plugin * me = NULL;

    *plugin = NULL;

    RTIOsapiHeap_allocateStructure(&me,struct NDDS_WriterHistory_Plugin);

    if (me == NULL) {
        return NDDS_WRITERHISTORY_RETCODE_FAILURE_INTERNAL;
    }

    me->classId = NDDS_WRITERHISTORY_FAST_MEMORY_PLUGIN_CLASSID;
    me->create_history = WriterHistoryFastMemoryPlugin_createHistory;
    me->destroy_history = WriterHistoryFastMemoryPlugin_destroyHistory;
    me->add_sample = WriterHistoryFastMemoryPlugin_addSample;
    me->add_batch_sample_group = NULL;
    me->add_historical_sample = NULL;
    me->find_sample = NULL;
    me->get_first_available_sn = WriterHistoryFastMemoryPlugin_getFirstAvailableSn;
    me->get_last_available_sn = WriterHistoryFastMemoryPlugin_getLastAvailableSn;
    me->get_first_non_reclaimable_sn = WriterHistoryFastMemoryPlugin_getFirstNonReclaimableSn;
    me->register_instance = WriterHistoryFastMemoryPlugin_registerInstance;
    me->unregister_instance = NULL;
    me->find_instance = NULL;
    me->set_deadline = NULL;
    me->set_sample_keep_duration = NULL;
    me->scale_sample_keep_duration = NULL;
    me->check_deadline = NULL;
    me->set_lifespan = NULL;
    me->prune_lifespan_expired_samples = 
        WriterHistoryFastMemoryPlugin_pruneLifespanExpiredSamples;
    me->begin_coherent_changes =
        NULL;
    me->begin_instance_iteration = NULL;
    me->end_instance_iteration = NULL;
    me->next_instance = NULL;
    me->begin_sample_iteration = NULL;
    me->end_sample_iteration = NULL;
    me->next_sample = NULL;
    me->return_sample_loan = WriterHistoryFastMemoryPlugin_returnSampleLoan;
    me->return_instance_loan = NULL;
    me->change_first_non_reclaimable_sn = NULL;
    me->make_sample_reclaimable = NULL;
    me->get_non_reclaimable_samples_count = NULL;
    me->get_app_ack_non_reclaimable_samples_count =
        NULL;
    me->flush_batch = NULL;
    me->initialize = WriterHistoryFastMemoryPlugin_initialize;
    me->destroy = WriterHistoryFastMemoryPlugin_destroy;
    me->get_statistics = NULL;
    me->set_statistics = NULL;
    me->get_batch_in_progress = NULL;
    me->get_next_sn = WriterHistoryFastMemoryPlugin_getNextSn;
    me->check_sample_keep_duration = NULL;
    me->set_sample_keep_duration_mode = NULL;
    me->get_serialization_buffer = NULL;
    me->return_serialization_buffer = NULL;
    me->purge_instance = NULL;
    me->get_writer_info = NULL;
    me->get_last_available_virtual_sn = NULL;
    me->set_autopurge_instance_on_unregister_delay = 
        NULL;
    me->set_autopurge_instance_on_dispose_delay =
        NULL;

    me->assert_remote_reader = NULL;
    me->remove_remote_reader = NULL;
    me->assert_app_ack = NULL;

    me->set_durable_subscriptions = WriterHistoryFastMemoryPlugin_setDurableSubscriptions;
    me->get_durable_subscription_info = NULL;
    me->set_durable_subscription_info = NULL;
    
    me->is_sample_app_ack = NULL;
    me->get_session_sample_count = NULL;

    *plugin = me;
    return NDDS_WRITERHISTORY_RETCODE_OK;
}
