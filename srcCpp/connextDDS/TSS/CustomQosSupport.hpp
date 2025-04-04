/* include RTI TSS header file for QoS configuration */
#include "util/rti_tss_common.h"

#ifdef RTI_PERF_TSS_PRO
  #include "Infrastructure_pro.h"
#else
  #include "Infrastructure_micro.h"
#endif

#include "log/ext_log.h"

#include "Infrastructure_common.h"
#include "ParameterManager.h"
#include "perftest.h"
#include "PerftestTransport.h"

#include "perftest_cpp.h"

typedef struct QoSBundle {
    ParameterManager* pm;
    PerftestTransport* transport;
    std::string topic_name;
    long instanceMaxCountReader;
    bool* isLargeData;
    unsigned long long* maxUnfragmentedRTPSPayloadSize;
} QoSBundle;

std::string stringValueQoS(DDS_Long resourceLimitValue);

/* function for customizing domain participant qos */
DDS_Boolean
RTI_TSS_participant_qos(struct DDS_DomainParticipantQos *dp_qos, void *data);

bool data_size_related_calculations(QoSBundle* ctx);

/* function for customizing data writer qos */
DDS_Boolean
RTI_TSS_datawriter_qos(struct DDS_DataWriterQos *writer_qos, void *data);

/* function for customizing data reader qos */
DDS_Boolean
RTI_TSS_datareader_qos(struct DDS_DataReaderQos *reader_qos, void *data);