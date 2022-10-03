#ifndef FAST_MEMORY_H
#define FAST_MEMORY_H

#include "log/log_makeheader.h"
#include "log/log_common.h"
#include "osapi/osapi_heap.h"
#include "writer_history/writer_history_interface.h"

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
    int _batchInfoBufferLength;
    char *_batchInfoBuffer;
    struct MIGCoherentSetInfo _coherentSetInfo;
    struct NDDS_WriterHistory_BatchStatus _batchStatus;
    RTI_UINT16 _batchEncapsulationKind;
};

RTI_INT32 WriterHistoryFastMemoryPlugin_pruneLifespanExpiredSamples(
    struct NDDS_WriterHistory_Plugin * self,
    RTIBool * samplesReusable_out,
    NDDS_WriterHistory_Handle history_in,
    const struct RTINtpTime *now_in,
    RTIBool singleSample_in);

RTI_INT32 WriterHistoryFastMemoryPlugin_registerInstance(
    struct NDDS_WriterHistory_Plugin * self,
    NDDS_WriterHistory_Handle history_in,
    const struct MIGRtpsKeyHash * instanceKeyHash_in,
    const void * instanceData_in,
    const struct RTINtpTime * timestamp_in,
    const struct RTINtpTime * now_in);

RTI_INT32 WriterHistoryFastMemoryPlugin_setDurableSubscriptions(
        struct NDDS_WriterHistory_Plugin *self,
        RTIBool *sampleReusable_out,
        NDDS_WriterHistory_Handle history_in,
        struct NDDS_WriterHistory_EndpointGroup *groups,
        int length);

RTI_INT32 WriterHistoryFastMemoryPlugin_getNextSn(
        struct NDDS_WriterHistory_Plugin * self,
        struct REDASequenceNumber sn_out[],
        NDDS_WriterHistory_Handle history_in,
        RTI_UINT32 sessionCount_in,
        RTI_INT32 sessionId_in[]);

RTI_INT32 WriterHistoryFastMemoryPlugin_getFirstNonReclaimableSn(
    struct NDDS_WriterHistory_Plugin * self,
    struct REDASequenceNumber sn_out[],
    struct RTINtpTime timestamp_out[],
    NDDS_WriterHistory_Handle history_in,
    RTI_UINT32 sessionCount_in,
    const RTI_INT32 sessionId_in[]);

RTI_INT32 WriterHistoryFastMemoryPlugin_getFirstAvailableSn(
    struct NDDS_WriterHistory_Plugin * self,
    struct REDASequenceNumber sn_out[],
    struct REDASequenceNumber virtualSn_out[],
    NDDS_WriterHistory_Handle history_in,
    RTI_UINT32 sessionCount_in,
    const RTI_INT32 sessionId_in[]);

RTI_INT32 WriterHistoryFastMemoryPlugin_getLastAvailableSn(
    struct NDDS_WriterHistory_Plugin * self,
    struct REDASequenceNumber sn_out[],
    NDDS_WriterHistory_Handle history_in,
    RTI_UINT32 sessionCount_in,
    const RTI_INT32 sessionId_in[]);

RTI_INT32 WriterHistoryFastMemoryPlugin_returnSampleLoan(
    struct NDDS_WriterHistory_Plugin * self,
    RTIBool * sampleReusable_out,
    NDDS_WriterHistory_Handle history_in,
    struct NDDS_WriterHistory_Sample * sample_in,
    const struct RTINtpTime * now_in);

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
    const struct NDDS_WriterHistory_WriteParams * write_params);

RTI_INT32 WriterHistoryFastMemoryPlugin_getBatchInProgress(
        struct NDDS_WriterHistory_Plugin *self,
        struct NDDS_WriterHistory_Sample *batch_out[],
        NDDS_WriterHistory_Handle history_in,
        RTI_INT32 sessionCount_in,
        RTI_INT32 sessionId_in[],
        MIGGeneratorEndian endian_in,
        const struct MIGRtpsGuid *readerGuid_in,
        RTIBool inCoherentSet_in,
        RTIBool endCoherentSet_in,
        const struct RTINtpTime *timestamp_in, /* Source timestamp */
        const struct RTINtpTime *now_in);

RTI_INT32 WriterHistoryFastMemoryPlugin_addBatchSampleGroup(
        struct NDDS_WriterHistory_Plugin *self,
        struct NDDS_WriterHistory_BatchSampleGroup *group_out,
        NDDS_WriterHistory_Handle history_in,
        const struct RTINtpTime *timestamp_in,
        const struct RTINtpTime *now_in);

RTI_INT32 WriterHistoryFastMemoryPlugin_flushBatch(
        struct NDDS_WriterHistory_Plugin *self,
        RTI_UINT32 *sampleCount_out,
        struct NDDS_WriterHistory_Sample *sample_out[],
        RTI_INT32 sessionId_out[],
        struct REDASequenceNumber firstAvailableSn_out[],
        struct REDASequenceNumber firstAvailableVirtualSn_out[],
        NDDS_WriterHistory_Handle history_in,
        RTI_INT32 sessionCount_in,
        const RTI_INT32 sessionId_in[]);

RTI_INT32 WriterHistoryMemoryPlugin_pruneExpiredSamples(
        struct NDDS_WriterHistory_Plugin *self,
        RTIBool *samplesReusable_out,
        NDDS_WriterHistory_Handle history_in,
        const struct RTINtpTime *now_in,
        RTIBool singleSample_in);
        
RTI_INT32 WriterHistoryFastMemoryPlugin_destroyHistory(
    struct NDDS_WriterHistory_Plugin * self,
    NDDS_WriterHistory_Handle history_in);

RTI_INT32 WriterHistoryFastMemoryPlugin_createHistory (
        struct NDDS_WriterHistory_Plugin * self,
        NDDS_WriterHistory_Handle * history_out,
        const struct NDDS_WriterHistory_Property * property_in,
        const struct NDDS_WriterHistory_Listener * historyListener_in,
        const struct MIGRtpsGuid * dwGuid_in,
        const struct MIGRtpsGuid * dwVirtualGuid_in,
        const struct NDDS_WriterHistory_AttributeSeq * dwProperty,
        struct RTIClock * clock_in,
        struct RTIClock * timestamp_clock_in,
        void * reserved);

RTI_INT32 WriterHistoryFastMemoryPlugin_initialize(
        struct NDDS_WriterHistory_Plugin * self,
        const struct NDDS_WriterHistory_AttributeSeq * dpProperties_in,
        void * reserved);

RTI_INT32 WriterHistoryFastMemoryPlugin_destroy(
        struct NDDS_WriterHistory_Plugin * self);

RTI_INT32 NDDS_WriterHistory_FastMemoryPlugin_create(
        struct NDDS_WriterHistory_Plugin ** plugin);

#endif /* FAST_MEMORY_H */