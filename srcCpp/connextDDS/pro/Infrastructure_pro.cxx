/*
 * (c) 2005-2019  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */

#include "Infrastructure_pro.h"
#include "perftest_cpp.h"
#include "osapi/osapi_sharedMemorySegment.h"

#include <time.h>
using namespace std;

#ifndef RTI_USE_CPP_11_INFRASTRUCTURE

/* Perftest Clock class */

PerftestClock::PerftestClock()
{
  #ifndef RTI_PERFTEST_NANO_CLOCK
    clock = RTIHighResolutionClock_new();
    if (clock == NULL) {
        throw std::bad_alloc();
    }
    RTINtpTime_setZero(&clockTimeAux);

    clockSec = 0;
    clockUsec = 0;
  #endif
}

PerftestClock::~PerftestClock()
{
  #ifndef RTI_PERFTEST_NANO_CLOCK
    RTIHighResolutionClock_delete(clock);
  #endif
}

PerftestClock &PerftestClock::getInstance()
{
    static PerftestClock instance;
    return instance;
}

unsigned long long PerftestClock::getTime()
{
  #ifndef RTI_PERFTEST_NANO_CLOCK
    clock->getTime(clock, &clockTimeAux);
    RTINtpTime_unpackToMicrosec(
            clockSec,
            clockUsec,
            clockTimeAux);
    return clockUsec + 1000000 * clockSec;
  #else
    clock_gettime(CLOCK_MONOTONIC, &timeStruct);
    return (timeStruct.tv_sec * ONE_BILLION) + timeStruct.tv_nsec;
  #endif
}

void PerftestClock::milliSleep(unsigned int millisec)
{
    NDDSUtility::sleep(DDS_Duration_t::from_millis(millisec));
}

void PerftestClock::sleep(const struct DDS_Duration_t& sleep_period)
{
    NDDSUtility::sleep(sleep_period);
}

/********************************************************************/
/* THREADS */

struct PerftestThread* PerftestThread_new(
        const char *name,
        int threadPriority,
        int threadOptions,
        RTIOsapiThreadOnSpawnedMethod method,
        void *threadParam)
{
    struct RTIOsapiThread *thread = NULL;
    thread = RTIOsapiThread_new(
            name,
            threadPriority,
            threadOptions,
            RTI_OSAPI_THREAD_STACK_SIZE_DEFAULT,
            NULL,
            method,
            threadParam);
    return thread;
}

#endif //#ifndef RTI_USE_CPP_11_INFRASTRUCTURE

/********************************************************************/
/* Transport Related functions */

bool addPropertyToParticipantQos(
        DDS_DomainParticipantQos &qos,
        std::string propertyName,
        std::string propertyValue)
{

    DDS_ReturnCode_t retcode = DDSPropertyQosPolicyHelper::add_property(
            qos.property,
            propertyName.c_str(),
            propertyValue.c_str(),
            false);
    if (retcode != DDS_RETCODE_OK) {
        fprintf(
                stderr,
                "%s Failed to add property \"%s\"\n",
                classLoggingString.c_str(),
                propertyName.c_str());
        return false;
    }

    return true;
}

bool assertPropertyToParticipantQos(
        DDS_DomainParticipantQos &qos,
        std::string propertyName,
        std::string propertyValue)
{
    DDS_ReturnCode_t retcode = DDSPropertyQosPolicyHelper::assert_property(
            qos.property,
            propertyName.c_str(),
            propertyValue.c_str(),
            false);
    if (retcode != DDS_RETCODE_OK) {
        fprintf(
                stderr,
                "%s Failed to add property \"%s\"\n",
                classLoggingString.c_str(),
                propertyName.c_str());
        return false;
    }

    return true;
}

bool setAllowInterfacesList(
        PerftestTransport &transport,
        DDS_DomainParticipantQos &qos,
        ParameterManager *parameterManager)
{
    if (!parameterManager->get<std::string>("allowInterfaces").empty()) {

        if (transport.transportConfig.kind == TRANSPORT_NOT_SET) {
            fprintf(stderr,
                    "%s Ignoring -nic/-allowInterfaces option\n",
                    classLoggingString.c_str());
            return true;
        }

        /*
         * In these 2 specific cases, we are forced to add 2 properties, one
         * for UDPv4 and another for UDPv6
         */
        if (transport.transportConfig.kind == TRANSPORT_UDPv4_UDPv6_SHMEM
                || transport.transportConfig.kind == TRANSPORT_UDPv4_UDPv6) {

            std::string propertyName =
                    "dds.transport.UDPv4.builtin.parent.allow_interfaces";

            if (!addPropertyToParticipantQos(
                    qos,
                    propertyName,
                    parameterManager->get<std::string>("allowInterfaces"))) {
                return false;
            }

            propertyName =
                    "dds.transport.UDPv6.builtin.parent.allow_interfaces";

            if (!addPropertyToParticipantQos(
                    qos,
                    propertyName,
                    parameterManager->get<std::string>("allowInterfaces"))) {
                return false;
            }

        } else {

            std::string propertyName = transport.transportConfig.prefixString;
            if (transport.transportConfig.kind == TRANSPORT_WANv4) {
                propertyName += ".parent";
            }

            propertyName += ".parent.allow_interfaces";

            return addPropertyToParticipantQos(
                    qos,
                    propertyName,
                    parameterManager->get<std::string>("allowInterfaces"));
        }
    }

    return true;
}

bool setTransportVerbosity(
        PerftestTransport &transport,
        DDS_DomainParticipantQos &qos,
        ParameterManager *parameterManager)
{
    if (!parameterManager->get<std::string>("transportVerbosity").empty()) {
        if (transport.transportConfig.kind == TRANSPORT_NOT_SET) {
            fprintf(stderr,
                    "%s Ignoring -transportVerbosity option\n",
                    classLoggingString.c_str());
            return true;
        }

        std::string propertyName = transport.transportConfig.prefixString
                + ".verbosity";

        // The name of the property in TCPv4 is different
        if (transport.transportConfig.kind == TRANSPORT_TCPv4) {
            propertyName = transport.transportConfig.prefixString
                    + ".logging_verbosity_bitmap";
        } else if (transport.transportConfig.kind == TRANSPORT_UDPv4
                || transport.transportConfig.kind == TRANSPORT_UDPv6
                || transport.transportConfig.kind == TRANSPORT_SHMEM
                || transport.transportConfig.kind == TRANSPORT_UDPv4_SHMEM
                || transport.transportConfig.kind == TRANSPORT_UDPv6_SHMEM
                || transport.transportConfig.kind == TRANSPORT_UDPv4_UDPv6
                || transport.transportConfig.kind == TRANSPORT_UDPv4_UDPv6_SHMEM) {
            // We do not change logging for the builtin transports.
            return true;
        }

        return addPropertyToParticipantQos(
                qos,
                propertyName,
                parameterManager->get<std::string>("transportVerbosity"));
    }
    return true;
}

bool configureSecurityFiles(
        PerftestTransport &transport,
        DDS_DomainParticipantQos& qos,
        ParameterManager *parameterManager)
{

    if (!parameterManager->get<std::string>("transportCertAuthority").empty()) {
        if (!addPropertyToParticipantQos(
                qos,
                transport.transportConfig.prefixString + ".tls.verify.ca_file",
                parameterManager->get<std::string>("transportCertAuthority"))) {
            return false;
        }
    }

    if (!parameterManager->get<std::string>("transportCertFile").empty()) {
        if (!addPropertyToParticipantQos(
                qos,
                transport.transportConfig.prefixString
                        + ".tls.identity.certificate_chain_file",
                parameterManager->get<std::string>("transportCertFile"))) {
            return false;
        }
    }

    if (!parameterManager->get<std::string>("transportPrivateKey").empty()) {
        if (!addPropertyToParticipantQos(
                qos,
                transport.transportConfig.prefixString
                        + ".tls.identity.private_key_file",
                parameterManager->get<std::string>("transportPrivateKey"))) {
            return false;
        }
    }

    return true;
}

bool configureTcpTransport(
        PerftestTransport &transport,
        DDS_DomainParticipantQos& qos,
        ParameterManager *parameterManager)
{
    qos.transport_builtin.mask = DDS_TRANSPORTBUILTIN_MASK_NONE;

    if (!addPropertyToParticipantQos(
            qos,
            std::string("dds.transport.load_plugins"),
            transport.transportConfig.prefixString)) {
        return false;
    }

    if (!parameterManager->get<std::string>("transportServerBindPort").empty()) {
        if (!addPropertyToParticipantQos(
                qos,
                transport.transportConfig.prefixString + ".server_bind_port",
                parameterManager->get<std::string>("transportServerBindPort"))) {
            return false;
        }
    }

    if (parameterManager->get<bool>("transportWan")) {
        if (!assertPropertyToParticipantQos(
                qos,
                transport.transportConfig.prefixString
                        + ".parent.classid",
                "NDDS_TRANSPORT_CLASSID_TCPV4_WAN")) {
            return false;
        }

        if (parameterManager->get<std::string>("transportServerBindPort") != "0") {
            if (!parameterManager->get<std::string>("transportPublicAddress").empty()) {
                if (!addPropertyToParticipantQos(
                        qos,
                        transport.transportConfig.prefixString
                                + ".public_address",
                        parameterManager->get<std::string>("transportPublicAddress"))) {
                    return false;
                }
            } else {
                fprintf(stderr,
                        "%s Public Address is required if "
                        "Server Bind Port != 0\n",
                        classLoggingString.c_str());
                return false;
            }
        }
    }

    if (transport.transportConfig.kind == TRANSPORT_TLSv4) {
        if (parameterManager->get<bool>("transportWan")) {
            if (!assertPropertyToParticipantQos(
                    qos,
                    transport.transportConfig.prefixString
                            + ".parent.classid",
                    "NDDS_TRANSPORT_CLASSID_TLSV4_WAN")) {
                return false;
            }
        } else {
            if (!assertPropertyToParticipantQos(
                    qos,
                    transport.transportConfig.prefixString
                            + ".parent.classid",
                    "NDDS_TRANSPORT_CLASSID_TLSV4_LAN")) {
                return false;
            }
        }

        if (!configureSecurityFiles(transport, qos, parameterManager)) {
            return false;
        }
    }

    return true;
}

bool configureDtlsTransport(
        PerftestTransport &transport,
        DDS_DomainParticipantQos& qos,
        ParameterManager *parameterManager)
{

    qos.transport_builtin.mask = DDS_TRANSPORTBUILTIN_MASK_NONE;

    if (!addPropertyToParticipantQos(
            qos,
            std::string("dds.transport.load_plugins"),
            transport.transportConfig.prefixString)) {
        return false;
    }

    if (!addPropertyToParticipantQos(
            qos,
            transport.transportConfig.prefixString + ".library",
            "nddstransporttls")) {
        return false;
    }

    if (!addPropertyToParticipantQos(
            qos,
            transport.transportConfig.prefixString + ".create_function",
            "NDDS_Transport_DTLS_create")) {
        return false;
    }

    if (!configureSecurityFiles(transport, qos, parameterManager)) {
        return false;
    }

    return true;
}

bool configureWanTransport(
        PerftestTransport &transport,
        DDS_DomainParticipantQos& qos,
        ParameterManager *parameterManager)
{

    qos.transport_builtin.mask = DDS_TRANSPORTBUILTIN_MASK_NONE;

    if (!addPropertyToParticipantQos(
            qos,
            std::string("dds.transport.load_plugins"),
            transport.transportConfig.prefixString)) {
        return false;
    }

    if (!addPropertyToParticipantQos(
            qos,
            transport.transportConfig.prefixString + ".library",
            "nddstransportwan")) {
        return false;
    }

    if (!addPropertyToParticipantQos(
            qos,
            transport.transportConfig.prefixString + ".create_function",
            "NDDS_Transport_WAN_create")) {
        return false;
    }

    if (!parameterManager->get<std::string>("transportWanServerAddress").empty()) {
        if (!addPropertyToParticipantQos(
                qos,
                transport.transportConfig.prefixString + ".server",
                parameterManager->get<std::string>("transportWanServerAddress"))) {
            return false;
        }
    } else {
        fprintf(stderr,
                "%s Wan Server Address is required\n",
                classLoggingString.c_str());
        return false;
    }

    if (!parameterManager->get<std::string>("transportWanServerPort").empty()) {
        if (!addPropertyToParticipantQos(
                qos,
                transport.transportConfig.prefixString + ".server_port",
                parameterManager->get<std::string>("transportWanServerPort"))) {
            return false;
        }
    }

    if (!parameterManager->get<std::string>("transportWanId").empty()) {
        if (!addPropertyToParticipantQos(
                qos,
                transport.transportConfig.prefixString
                        + ".transport_instance_id",
                parameterManager->get<std::string>("transportWanId"))) {
            return false;
        }
    } else {
        fprintf(stderr, "%s Wan ID is required\n", classLoggingString.c_str());
        return false;
    }

    if (parameterManager->get<bool>("transportSecureWan")) {
        if (!addPropertyToParticipantQos(
                qos,
                transport.transportConfig.prefixString + ".enable_security",
                "1")) {
            return false;
        }

        if (!configureSecurityFiles(transport, qos, parameterManager)) {
            return false;
        }
    }

    return true;
}
#ifdef PERFTEST_CONNEXT_PRO_610
bool configureUdpv4WanTransport(
        PerftestTransport &transport,
        DDS_DomainParticipantQos& qos,
        ParameterManager *parameterManager)
{

    qos.transport_builtin.mask = DDS_TRANSPORTBUILTIN_UDPv4_WAN;

    if (!parameterManager->get<std::string>("transportPublicAddress").empty()) {


        std::string publicAddress
                = parameterManager->get<std::string>("transportPublicAddress");
        std::string delimiter = ":";

        size_t pos = publicAddress.find(delimiter);
        if (pos == std::string::npos) {
            fprintf(stderr,
                    "%s Wan Public Address format invalid. Use "
                    "<public_address>:<public_port>\n",
                    classLoggingString.c_str());
            return false;
        }

        std::string publicAddressIp
                = publicAddress.substr(0, pos);
        std::string publicAddressPort
                = publicAddress.substr(pos+1, std::string::npos);

        // If transportHostPort is defined use that port for the transport
        std::string publicAddressHostPort;
        if (!parameterManager->get<std::string>("transportHostPort").empty()) {
            std::string transportHostPort = parameterManager->get<std::string>("transportHostPort");
            publicAddressHostPort = transportHostPort;
        } else {  // Use Internal Port as external port
            publicAddressHostPort = publicAddressPort;
        }

        if (!addPropertyToParticipantQos(
                qos,
                transport.transportConfig.prefixString + ".public_address",
                publicAddressIp)) {
            return false;
        }

        std::string json_string_comm_ports(
                "{ \"default\": { \"host\": "
                + publicAddressHostPort
                + ",  \"public\": "
                + publicAddressPort
                + " } }");

        if (!addPropertyToParticipantQos(
                qos,
                transport.transportConfig.prefixString + ".comm_ports",
                json_string_comm_ports)) {
            return false;
        }
    }

    return true;
}
#endif // PERFTEST_CONNEXT_PRO_610


bool configureShmemTransport(
        PerftestTransport &transport,
        DDS_DomainParticipantQos& qos,
        ParameterManager *parameterManager)
{
  /**
   * OSAPI do not support SHMEM for Android yet
   */
  #if !defined(RTI_ANDROID)
    DDS_Property_t *parentProp =
            DDSPropertyQosPolicyHelper::lookup_property(qos.property,
                    "dds.transport.shmem.builtin.parent.message_size_max");

    unsigned long long datalen = parameterManager->get<unsigned long long>("dataLen");

    long parentMsgSizeMax = transport.minimumMessageSizeMax;
    std::ostringstream ss;
    bool messageSizeMaxSet = false;

    /*
     * If the property defining the message_size_max for shared memory is not
     * set and we are using exclusively SHMEM, then we will calculate
     * automatically the message_size_max to accommodate the sample in a single
     * packet and avoid fragmentation.
     *
     * Note that if we have set message_size_max via the command line argument
     * we will not change it and we will use that value, because we call this 
     * function after setting the message_size_max for shmem.
     */
    if (parentProp != NULL && parentProp->value != NULL) {
        parentMsgSizeMax = atoi(parentProp->value);
        messageSizeMaxSet = true;
    } else {
        if (qos.transport_builtin.mask == DDS_TRANSPORTBUILTIN_SHMEM) {
            if ((datalen + MESSAGE_OVERHEAD_BYTES)
                    > (unsigned long long) parentMsgSizeMax) {
                parentMsgSizeMax = datalen + MESSAGE_OVERHEAD_BYTES;

                if (parameterManager->get<bool>("flatdata")) {
                    /*
                     * This 17 is due to the incorrecty setting OVERHEAD_BYTES in
                     * perftest_cpp: In the case of Flat Data is not right.
                     */
                    parentMsgSizeMax += 17;
                }

                transport.minimumMessageSizeMax = parentMsgSizeMax;
            }
        }

        ss.str("");
        ss.clear();
        ss << parentMsgSizeMax;
        if (!assertPropertyToParticipantQos(
                qos,
                "dds.transport.shmem.builtin.parent.message_size_max",
                ss.str())) {
            return false;
        }
        transport.loggingString +=
                ("\tSHMEM message_size_max: "
                + ss.str()
                + "\n");
    }

    /*
     * The maximum size of a SHMEM segment highly depends on the platform.
     * So that, we need to find out the maximum allocable space to avoid
     * runtime errors.
     *
     * minSize is the packet size of the builtin transport for SHMEM.
     * We need it as the minimum size for a SHMEM segment since otherwise,
     * DDS would not even be able to send a sample.
     *
     * The maxBufferSize is present in order to have a limit on the buffer size
     * for SHMEM so Perftest does not require unlimited resources.
     *
     * Since we are doing a decremental search, we use a step of 1MB to search
     * down a pseudo-optimal buffer size in the range
     * [minBufferSize, maxBufferSize].
     */
    RTIOsapiSharedMemorySegmentHandle handle;
    RTI_UINT64 pid = RTIOsapiProcess_getId();
    RTIBool success = RTI_FALSE;
    int retcode;
    int key = rand();
    long minBufferSize = 1048576;
    int step = 1048576; // 1MB
    long maxBufferSize = (std::max)((long) 121634816 /* 116MB */, parentMsgSizeMax);

    do {
        // Reset handles to known state
        RTIOsapiMemory_zero(&handle,
                sizeof(struct RTIOsapiSharedMemorySegmentHandle));

        success = RTIOsapiSharedMemorySegment_create(
                &handle, &retcode, key, maxBufferSize, pid);

        RTIOsapiSharedMemorySegment_delete(&handle);

        if (success) {
            break;
        }

        maxBufferSize -= step;
    } while (maxBufferSize > minBufferSize);

    if (maxBufferSize < minBufferSize) {
        fprintf(stderr,
                "%s Failed to allocate SHMEM segment of 1MB.\n"
                "Change OS settings to test SHMEM.\n",
                classLoggingString.c_str());
        return false;
    }

    if (!messageSizeMaxSet &&
            parentMsgSizeMax > maxBufferSize) {
        parentMsgSizeMax = maxBufferSize;

        transport.minimumMessageSizeMax = parentMsgSizeMax;
        ss.str("");
        ss.clear();
        ss << maxBufferSize;
        if (!assertPropertyToParticipantQos(
                qos,
                "dds.transport.shmem.builtin.parent.message_size_max",
                ss.str())) {
            return false;
        }
        transport.loggingString +=
                ("\tSHMEM message_size_max: "
                + ss.str()
                + "\n");
    }

    /*
     * From user manual "Properties for Builtin Shared-Memory Transport":
     * To optimize memory usage, specify a receive queue size less than that
     * required to hold the maximum number of messages which are all of the
     * maximum size.
     *
     * In most situations, the average message size may be far less than the
     * maximum message size. So for example, if the maximum message size is 64K
     * bytes, and you configure the plugin to buffer at least 10 messages, then
     * 640K bytes of memory would be needed if all messages were 64K bytes.
     * Should this be desired, then receive_buffer_size should be set to 640K
     * bytes.
     *
     * However, if the average message size is only 10K bytes, then you could
     * set the receive_buffer_size to 100K bytes. This allows you to optimize
     * the memory usage of the plugin for the average case and yet allow the
     * plugin to handle the extreme case.
     *
     * The receivedMessageCountMax should be set to a value that can hold
     * more than “-sendQueueSize" samples in perftest in order block in the
     * send window before starting to lose messages on the Shared Memory
     * transport
     */
    /*
     * This is the flow Controller default token size. Change this if you modify
     * the qos file to add a different "bytes_per_token" property
     */
    long flowControllerTokenSize = transport.minimumMessageSizeMax;

  #ifdef RTI_FLATDATA_AVAILABLE
    // Zero Copy sends 16-byte references
    if (parameterManager->get<bool>("zerocopy")) {
        datalen = 16;
    }
  #endif

    /*
     * We choose the minimum between the flow Controller max fragment size and
     * the message_size_max - RTPS headers.
     */
    int fragmentSize = (std::min)(
            parentMsgSizeMax - TMP_COMMEND_WRITER_MAX_RTPS_OVERHEAD,
            flowControllerTokenSize - TMP_COMMEND_WRITER_MAX_RTPS_OVERHEAD);

    unsigned long long rtpsMessagesPerSample = (std::max)(
            1ull, (datalen / fragmentSize) + 1);

    unsigned long long receivedMessageCountMax =
            2 * (parameterManager->get<int>("sendQueueSize") + 1) * rtpsMessagesPerSample;

    unsigned long long receiveBufferSize = (std::min)(
        (unsigned long long) maxBufferSize,
        receivedMessageCountMax *
                (TMP_COMMEND_WRITER_MAX_RTPS_OVERHEAD + fragmentSize));


    if (DDSPropertyQosPolicyHelper::lookup_property(
                qos.property,
                "dds.transport.shmem.builtin.received_message_count_max")
            == NULL) {
        ss.str("");
        ss.clear();
        ss << receivedMessageCountMax;

        if (!assertPropertyToParticipantQos(
                qos,
                "dds.transport.shmem.builtin.received_message_count_max",
                ss.str())) {
            return false;
        }
        transport.loggingString +=
                ("\tSHMEM received_message_count_max: "
                + ss.str()
                + "\n");
    }

    if (DDSPropertyQosPolicyHelper::lookup_property(
                qos.property,
                "dds.transport.shmem.builtin.receive_buffer_size")
            == NULL) {
        ss.str("");
        ss.clear();
        ss << receiveBufferSize;
        if (!assertPropertyToParticipantQos(
                qos,
                "dds.transport.shmem.builtin.receive_buffer_size",
                ss.str())) {
            return false;
        }
        transport.loggingString +=
                ("\tSHMEM receive_buffer_size: "
                + ss.str()
                + "\n");
    }

    return true;
  #else
    // Not supported yet.
    return false;
  #endif
}

/*
 * Gets the MessageSizeMax given the name (String) of a transport from a
 * DDS_DomainParticipantQos object. If the value is not present, returns
 * DEFAULT_MESSAGE_SIZE_MAX.
 */
long getTransportMessageSizeMax(
        std::string targetTransportName,
        PerftestTransport &transport,
        DDS_DomainParticipantQos& qos)
{
    std::string propertyName =
            transport.transportConfigMap[targetTransportName].prefixString
            + ".parent.message_size_max";

    DDS_Property_t *parentProp = DDSPropertyQosPolicyHelper::lookup_property(
            qos.property,
            propertyName.c_str());
    if (parentProp != NULL && parentProp->value != NULL) {
        // printf("Value for %s is %s\n", propertyName.c_str(), parentProp->value);
        return atoi(parentProp->value);
    } else {
        // printf("Value for %s not found, returning default\n",propertyName.c_str());
        return DEFAULT_MESSAGE_SIZE_MAX;
    }
}

/*
 * Sets the MessageSizeMax property given the name (String) of a transport in a
 * DDS_DomainParticipantQos object.
 */
bool setTransportMessageSizeMax(
        std::string targetTransportName,
        PerftestTransport &transport,
        DDS_DomainParticipantQos& qos,
        long messageSizeMax)
{
    std::string propertyName =
            transport.transportConfigMap[targetTransportName].prefixString
            + ".parent.message_size_max";

    return assertPropertyToParticipantQos(
            qos,
            propertyName,
            std::to_string(messageSizeMax).c_str());
}


/*
 * Setup the transport minimumMessageSize max. If the message_size_max is set via
 * command line, then we will use that and set the properties for every transport.
 * Otherwise, what we will do is get the minimum value for all the enabled transports
 * in the XML configuration.
 */
bool setupTransportMinimumMessageSizeMax(
        ParameterManager *parameterManager,
        PerftestTransport &transport,
        DDS_DomainParticipantQos& qos)
{
    long qosConfigurationMessageSizeMax = MESSAGE_SIZE_MAX_NOT_SET;
    long transportMessageSizeMax = MESSAGE_SIZE_MAX_NOT_SET;

    if (parameterManager->is_set("messageSizeMax")) {

        transportMessageSizeMax = parameterManager->get<int>("messageSizeMax");
        qosConfigurationMessageSizeMax = transportMessageSizeMax;
        transport.loggingString += ("\tmessage_size_max: "
                + std::to_string(transportMessageSizeMax)
                + " (set via command line)\n");

        if ((qos.transport_builtin.mask & DDS_TRANSPORTBUILTIN_SHMEM) != 0) {
            if (!setTransportMessageSizeMax("SHMEM", transport, qos, transportMessageSizeMax)){
                printf("[Error]: error in setupTransportMinimumMessageSizeMax setting message_size_max to SHMEM\n");
                return false;
            }
        }
        if ((qos.transport_builtin.mask & DDS_TRANSPORTBUILTIN_UDPv4) != 0) {
            if (!setTransportMessageSizeMax("UDPv4", transport, qos, transportMessageSizeMax)){
                printf("[Error]: error in setupTransportMinimumMessageSizeMax setting message_size_max to UDPv4\n");
                return false;
            }
        }
        if ((qos.transport_builtin.mask & DDS_TRANSPORTBUILTIN_UDPv6) != 0) {
            if (!setTransportMessageSizeMax("UDPv6", transport, qos, transportMessageSizeMax)){
                printf("[Error]: error in setupTransportMinimumMessageSizeMax setting message_size_max to UDPv6\n");
                return false;
            }
        }
        if (transport.transportConfig.kind == TRANSPORT_TCPv4
                || transport.transportConfig.kind == TRANSPORT_TLSv4) {
            if (!setTransportMessageSizeMax("TCP", transport, qos, transportMessageSizeMax)){
                printf("[Error]: error in setupTransportMinimumMessageSizeMax setting message_size_max to TCP\n");
                return false;
            }
        }
        if (transport.transportConfig.kind == TRANSPORT_DTLSv4) {
            if (!setTransportMessageSizeMax("DTLS", transport, qos, transportMessageSizeMax)){
                printf("[Error]: error in setupTransportMinimumMessageSizeMax setting message_size_max to DTLS\n");
                return false;
            }
        }
        if (transport.transportConfig.kind == TRANSPORT_WANv4) {
            if (!setTransportMessageSizeMax("WAN", transport, qos, transportMessageSizeMax)){
                printf("[Error]: error in setupTransportMinimumMessageSizeMax setting message_size_max to WAN\n");
                return false;
            }
        }
      #ifdef PERFTEST_CONNEXT_PRO_610
        if (transport.transportConfig.kind == TRANSPORT_UDPv4_WAN) {
            if (!setTransportMessageSizeMax("UDPv4_WAN", transport, qos, transportMessageSizeMax)){
                printf("[Error]: error in setupTransportMinimumMessageSizeMax setting message_size_max to UDPv4_WAN\n");
                return false;
            }
        }
      #endif

    } else {
        if ((qos.transport_builtin.mask & DDS_TRANSPORTBUILTIN_SHMEM) != 0) {
            transportMessageSizeMax = getTransportMessageSizeMax("SHMEM", transport, qos);
            if (transportMessageSizeMax < qosConfigurationMessageSizeMax) {
                qosConfigurationMessageSizeMax = transportMessageSizeMax;
            }
        }
        if ((qos.transport_builtin.mask & DDS_TRANSPORTBUILTIN_UDPv4) != 0) {
            transportMessageSizeMax = getTransportMessageSizeMax("UDPv4", transport, qos);
            if (transportMessageSizeMax < qosConfigurationMessageSizeMax) {
                qosConfigurationMessageSizeMax = transportMessageSizeMax;
            }
        }
        if ((qos.transport_builtin.mask & DDS_TRANSPORTBUILTIN_UDPv6) != 0) {
            transportMessageSizeMax = getTransportMessageSizeMax("UDPv6", transport, qos);
            if (transportMessageSizeMax < qosConfigurationMessageSizeMax) {
                qosConfigurationMessageSizeMax = transportMessageSizeMax;
            }
        }
        if (transport.transportConfig.kind == TRANSPORT_TCPv4
                || transport.transportConfig.kind == TRANSPORT_TLSv4) {
            transportMessageSizeMax = getTransportMessageSizeMax("TCP", transport, qos);
            if (transportMessageSizeMax < qosConfigurationMessageSizeMax) {
                qosConfigurationMessageSizeMax = transportMessageSizeMax;
            }
        }
        if (transport.transportConfig.kind == TRANSPORT_DTLSv4) {
            transportMessageSizeMax = getTransportMessageSizeMax("DTLS", transport, qos);
            if (transportMessageSizeMax < qosConfigurationMessageSizeMax) {
                qosConfigurationMessageSizeMax = transportMessageSizeMax;
            }
        }
        if (transport.transportConfig.kind == TRANSPORT_WANv4) {
            transportMessageSizeMax = getTransportMessageSizeMax("WAN", transport, qos);
            if (transportMessageSizeMax < qosConfigurationMessageSizeMax) {
                qosConfigurationMessageSizeMax = transportMessageSizeMax;
            }
        }
      #ifdef PERFTEST_CONNEXT_PRO_610
        if (transport.transportConfig.kind == TRANSPORT_UDPv4_WAN) {
            transportMessageSizeMax = getTransportMessageSizeMax("UDPv4_WAN", transport, qos);
            if (transportMessageSizeMax < qosConfigurationMessageSizeMax) {
                qosConfigurationMessageSizeMax = transportMessageSizeMax;
            }
        }
      #endif
        transport.loggingString += ("\tmessage_size_max: "
                + std::to_string(qosConfigurationMessageSizeMax)
                + " (minimum across all transports)\n");
    }

    transport.minimumMessageSizeMax = qosConfigurationMessageSizeMax;
    return true;
}

bool PerftestConfigureTransport(
        PerftestTransport &transport,
        DDS_DomainParticipantQos &qos,
        ParameterManager *parameterManager)
{

    /*
     * If transportConfig.kind is not set, then we want to use the value
     * provided by the Participant Qos, so first we read it from there and
     * update the value of transportConfig.kind with whatever was already set.
     */

    if (transport.transportConfig.kind == TRANSPORT_NOT_SET) {

        DDS_TransportBuiltinKindMask mask = qos.transport_builtin.mask;
        switch (mask) {
            case DDS_TRANSPORTBUILTIN_UDPv4:
                transport.transportConfig = TransportConfig(
                    TRANSPORT_UDPv4,
                    "UDPv4",
                    "dds.transport.UDPv4.builtin");
                break;
            case DDS_TRANSPORTBUILTIN_UDPv6:
                transport.transportConfig = TransportConfig(
                    TRANSPORT_UDPv6,
                    "UDPv6",
                    "dds.transport.UDPv6.builtin");
                break;
            case DDS_TRANSPORTBUILTIN_SHMEM:
                transport.transportConfig = TransportConfig(
                    TRANSPORT_SHMEM,
                    "SHMEM",
                    "dds.transport.shmem.builtin");
                break;
            case DDS_TRANSPORTBUILTIN_UDPv4|DDS_TRANSPORTBUILTIN_SHMEM:
                transport.transportConfig = TransportConfig(
                    TRANSPORT_UDPv4_SHMEM,
                    "UDPv4 & SHMEM",
                    "dds.transport.UDPv4.builtin");
                break;
            case DDS_TRANSPORTBUILTIN_UDPv4|DDS_TRANSPORTBUILTIN_UDPv6:
                transport.transportConfig = TransportConfig(
                    TRANSPORT_UDPv4_UDPv6,
                    "UDPv4 & UDPv6",
                    "dds.transport.UDPv4.builtin");
                break;
            case DDS_TRANSPORTBUILTIN_SHMEM|DDS_TRANSPORTBUILTIN_UDPv6:
                transport.transportConfig = TransportConfig(
                    TRANSPORT_UDPv6_SHMEM,
                    "UDPv6 & SHMEM",
                    "dds.transport.UDPv6.builtin");
                break;
            case DDS_TRANSPORTBUILTIN_UDPv4|DDS_TRANSPORTBUILTIN_UDPv6|DDS_TRANSPORTBUILTIN_SHMEM:
                transport.transportConfig = TransportConfig(
                    TRANSPORT_UDPv4_UDPv6_SHMEM,
                    "UDPv4 & UDPv6 & SHMEM",
                    "dds.transport.UDPv4.builtin");
                break;
          #ifdef PERFTEST_CONNEXT_PRO_610
            case DDS_TRANSPORTBUILTIN_UDPv4_WAN:
                transport.transportConfig = TransportConfig(
                    TRANSPORT_UDPv4_WAN,
                    "UDPv4_WAN",
                    "dds.transport.UDPv4_WAN.builtin");
                break;
          #endif
            default:
                /*
                 * This would mean that the mask is either empty or a
                 * different value that we do not support yet. So we keep
                 * the value as "TRANSPORT_NOT_SET"
                 */
                break;
        }
        transport.transportConfig.takenFromQoS = true;
    }

    switch (transport.transportConfig.kind) {
        case TRANSPORT_UDPv4:
            qos.transport_builtin.mask = DDS_TRANSPORTBUILTIN_UDPv4;
            break;

        case TRANSPORT_UDPv6:
            qos.transport_builtin.mask = DDS_TRANSPORTBUILTIN_UDPv6;
            break;

        case TRANSPORT_SHMEM:
            qos.transport_builtin.mask = DDS_TRANSPORTBUILTIN_SHMEM;
            break;

        default:
            break;
    };

    /*
     * Once the configurations have been established, we can get the
     * MessageSizeMax for the Transport, which should be the minimum of
     * all the enabled transports, unless the message size max has been
     * provided, which will be used instead.
     */
    setupTransportMinimumMessageSizeMax(parameterManager, transport, qos);

    switch (transport.transportConfig.kind) {

        case TRANSPORT_UDPv4:
        case TRANSPORT_UDPv6:
            break;

        case TRANSPORT_SHMEM:
            if (!configureShmemTransport(transport, qos, parameterManager)) {
                fprintf(stderr,
                        "%s Failed to configure SHMEM plugin\n",
                        classLoggingString.c_str());
                return false;
            }
            break;

        case TRANSPORT_TCPv4:
            if (!configureTcpTransport(transport, qos, parameterManager)) {
                fprintf(stderr,
                        "%s Failed to configure TCP plugin\n",
                        classLoggingString.c_str());
                return false;
            }
            break;

        case TRANSPORT_TLSv4:
            if (!configureTcpTransport(transport, qos, parameterManager)) {
                fprintf(stderr,
                        "%s Failed to configure TCP + TLS plugin\n",
                        classLoggingString.c_str());
                return false;
            }
            break;

        case TRANSPORT_DTLSv4:
            if (!configureDtlsTransport(transport, qos, parameterManager)) {
                fprintf(stderr,
                        "%s Failed to configure DTLS plugin\n",
                        classLoggingString.c_str());
                return false;
            }
            break;

        case TRANSPORT_WANv4:
            if (!configureWanTransport(transport, qos, parameterManager)) {
                fprintf(stderr,
                        "%s Failed to configure WAN plugin\n",
                        classLoggingString.c_str());
                return false;
            }
            break;
      #ifdef PERFTEST_CONNEXT_PRO_610
        case TRANSPORT_UDPv4_WAN:
            if (!configureUdpv4WanTransport(transport, qos, parameterManager)) {
                fprintf(stderr,
                        "%s Failed to configure UDPv4_WAN plugin\n",
                        classLoggingString.c_str());
                return false;
            }
            break;
      #endif // PERFTEST_CONNEXT_PRO_610
        default:

            /*
             * If shared memory is enabled we want to set up its
             * specific configuration
             */
            if ((qos.transport_builtin.mask & DDS_TRANSPORTBUILTIN_SHMEM)
                    != 0) {
                if (!configureShmemTransport(transport, qos, parameterManager)) {
                    fprintf(stderr,
                            "%s Failed to configure SHMEM plugin\n",
                            classLoggingString.c_str());
                    return false;
                }
            }
            break;
    } // Switch

    /*
     * If the transport is empty or if it is shmem, it does not make sense
     * setting an interface, in those cases, if the allow interfaces is provided
     * we empty it.
     */
    if (transport.transportConfig.kind != TRANSPORT_NOT_SET
            && transport.transportConfig.kind != TRANSPORT_SHMEM) {
        if (!setAllowInterfacesList(transport, qos, parameterManager)) {
            return false;
        }
    } else {
        // We are not using the allow interface string, so it is clean
        parameterManager->set<std::string>("allowInterfaces", std::string(""));
    }

    if (!setTransportVerbosity(transport, qos, parameterManager)) {
        return false;
    }

    return true;
}

#ifdef RTI_SECURE_PERFTEST
bool PerftestConfigureSecurity(
        PerftestSecurity &security,
        DDS_DomainParticipantQos &qos,
        ParameterManager *parameterManager)
{

    if (!addPropertyToParticipantQos(
            qos,
            "com.rti.serv.load_plugin",
            "com.rti.serv.secure")) {
        return false;
    }

  #ifdef RTI_PERFTEST_DYNAMIC_LINKING

    if (!assertPropertyToParticipantQos(
        qos,
        "com.rti.serv.secure.create_function",
      #if defined(RTI_LW_SECURE_PERFTEST) && defined(PERFTEST_CONNEXT_PRO_720)
        "RTI_SecurityLightweight_PluginSuite_create"
      #else
        "RTI_Security_PluginSuite_create"
      #endif //defined(RTI_LW_SECURE_PERFTEST) && defined(PERFTEST_CONNEXT_PRO_720)
        )) {
        return false;
    }

    if (!addPropertyToParticipantQos(
            qos,
            "com.rti.serv.secure.library",
            parameterManager->get<std::string>("secureLibrary"))) {
        return false;
    }

  #else // Static library linking

    if(DDSPropertyQosPolicyHelper::assert_pointer_property(
            qos.property,
            "com.rti.serv.secure.create_function_ptr",
          #if defined(RTI_LW_SECURE_PERFTEST) && defined(PERFTEST_CONNEXT_PRO_720)
            (void *) RTI_SecurityLightweight_PluginSuite_create
          #else
            (void *) RTI_Security_PluginSuite_create
          #endif //defined(RTI_LW_SECURE_PERFTEST) && defined(PERFTEST_CONNEXT_PRO_720)
            )
                != DDS_RETCODE_OK) {
        printf("Failed to add pointer_property "
                "com.rti.serv.secure.create_function_ptr\n");
        return false;
    }

  #endif


  // These options only make sense when not using LW security, this will
  // happen in Static if we have not defined RTI_LW_SECURE_PERFTEST, and in
  // dynamic if the command line option -lightWeightSecurity was not passed.
  #ifndef RTI_LW_SECURE_PERFTEST

    /*
     * Below, we are using com.rti.serv.secure properties in order to be
     * backward compatible with RTI Connext DDS 5.3.0 and below. Later versions
     * use the properties that are specified in the DDS Security specification
     * (see also the RTI Security Plugins Getting Started Guide). However,
     * later versions still support the legacy properties as an alternative.
     */

    // In the case where we are dynamic, we should not set these properties if
    // we are using the lightweight security library.
    if (!parameterManager->is_set("lightWeightSecurity")
            && !parameterManager->is_set("secureRtpsHmacOnly")) {
        // check if governance file provided
        if (!parameterManager->get<std::string>("secureGovernanceFile").empty()) {
            if (!addPropertyToParticipantQos(
                    qos,
                    "com.rti.serv.secure.access_control.governance_file",
                    parameterManager->get<std::string>("secureGovernanceFile"))) {
                return false;
            }
        } else {
            fprintf(stderr, "%s SecureGovernanceFile cannot be empty when using regular security.\n",
                    classLoggingString.c_str());
            return false;
        }

        // Permissions file
        if (!addPropertyToParticipantQos(
                qos,
                "com.rti.serv.secure.access_control.permissions_file",
                parameterManager->get<std::string>("securePermissionsFile"))) {
            return false;
        }

        // permissions authority file (legacy property, it should be permissions_file)
        if (!addPropertyToParticipantQos(
                qos,
                "com.rti.serv.secure.access_control.permissions_authority_file",
                parameterManager->get<std::string>("secureCertAuthority"))) {
            return false;
        }

        // certificate authority
        if (!addPropertyToParticipantQos(
                qos,
                "com.rti.serv.secure.authentication.ca_file",
                parameterManager->get<std::string>("secureCertAuthority"))) {
            return false;
        }

        // public key
        if (!addPropertyToParticipantQos(
                qos,
                "com.rti.serv.secure.authentication.certificate_file",
                parameterManager->get<std::string>("secureCertFile"))) {
            return false;
        }

        if (!addPropertyToParticipantQos(
                qos,
                "com.rti.serv.secure.cryptography.max_receiver_specific_macs",
                "4")) {
            return false;
        }

        // private key
        if (!addPropertyToParticipantQos(
                qos,
                "com.rti.serv.secure.authentication.private_key_file",
                parameterManager->get<std::string>("securePrivateKey"))) {
            return false;
        }

        if (parameterManager->is_set("secureEncryptionAlgo")) {
            if (!addPropertyToParticipantQos(
                    qos,
                    "com.rti.serv.secure.cryptography.encryption_algorithm",
                    parameterManager->get<std::string>("secureEncryptionAlgo"))) {
                return false;
            }
        }
    }

  #endif // !defined(RTI_LW_SECURE_PERFTEST)

    if (parameterManager->is_set("securePSK") || parameterManager->is_set("securePSKAlgorithm")) {

        if (!parameterManager->is_set("securePSK")) {
            parameterManager->set("securePSK", "DefaultValue");
        }

        if (!addPropertyToParticipantQos(
                qos,
                "com.rti.serv.secure.dds.sec.crypto.rtps_psk_secret_passphrase",
                parameterManager->get<std::string>("securePSK").c_str())) {
            return false;
        }

        if (parameterManager->get<std::string>("securePSKAlgorithm").find("GMAC") != std::string::npos) {
            if (!addPropertyToParticipantQos(
                    qos,
                    "com.rti.serv.secure.dds.sec.access.rtps_psk_protection_kind",
                    "SIGN")) {
                return false;
            }
        }

        if (!addPropertyToParticipantQos(
                qos,
                "com.rti.serv.secure.cryptography.rtps_psk_symmetric_cipher_algorithm",
                parameterManager->get<std::string>("securePSKAlgorithm").c_str())) {
            return false;
        }

    }

    if (parameterManager->is_set("secureRtpsHmacOnly")) {
        if (!addPropertyToParticipantQos(
                qos,
                "com.rti.serv.secure.hmac_only.enabled",
                "1")) {
            return false;
        }
        if (!addPropertyToParticipantQos(
                qos,
                "com.rti.serv.secure.hmac_only.cryptography.key",
                parameterManager->get<std::string>("secureRtpsHmacOnly").c_str())) {
            return false;
        }
    }

    if (parameterManager->is_set("secureEnableAAD")) {
        if (!addPropertyToParticipantQos(
                qos,
                "com.rti.serv.secure.cryptography.enable_additional_authenticated_data",
                "1")) {
            return false;
        }
    }

    if (parameterManager->is_set("secureDebug")) {
        // private key
        if (!addPropertyToParticipantQos(
                qos,
                "com.rti.serv.secure.logging.log_level",
                perftest::to_string(parameterManager->get<int>("secureDebug")))) {
            return false;
        }
    }

    return true;
}

#endif
