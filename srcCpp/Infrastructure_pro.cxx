/*
 * (c) 2005-2019  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */

#include "Infrastructure_pro.h"
#include "perftest_cpp.h"
#include "osapi/osapi_sharedMemorySegment.h"

/*
 * Since std::to_string is not defined until c++11
 * we will define it here.
 */
namespace std {
    template<typename T>
    std::string to_string(const T &n) {
        std::ostringstream s;
        s << n;
        return s.str();
    }
}

/* Perftest Clock class */

PerftestClock::PerftestClock()
{
    clock = RTIHighResolutionClock_new();
    if (clock == NULL) {
        throw std::bad_alloc();
    }
    RTINtpTime_setZero(&clockTimeAux);

    clockSec = 0;
    clockUsec = 0;
}

PerftestClock::~PerftestClock()
{
    RTIHighResolutionClock_delete(clock);
}

PerftestClock &PerftestClock::getInstance()
{
    static PerftestClock instance;
    return instance;
}

unsigned long long PerftestClock::getTimeUsec()
{
    clock->getTime(clock, &clockTimeAux);
    RTINtpTime_unpackToMicrosec(
            clockSec,
            clockUsec,
            clockTimeAux);
    return clockUsec + 1000000 * clockSec;
}

void PerftestClock::milliSleep(unsigned int millisec)
{
    NDDSUtility::sleep(DDS_Duration_t::from_millis(millisec));
}

void PerftestConfigureVerbosity(int verbosityLevel)
{

    NDDS_Config_LogVerbosity verbosity = NDDS_CONFIG_LOG_VERBOSITY_ERROR;
    switch (verbosityLevel) {
        case 0: verbosity = NDDS_CONFIG_LOG_VERBOSITY_SILENT;
                fprintf(stderr, "Setting verbosity to SILENT\n");
                break;
        case 1: verbosity = NDDS_CONFIG_LOG_VERBOSITY_ERROR;
                fprintf(stderr, "Setting verbosity to ERROR\n");
                break;
        case 2: verbosity = NDDS_CONFIG_LOG_VERBOSITY_WARNING;
                fprintf(stderr, "Setting verbosity to WARNING\n");
                break;
        case 3: verbosity = NDDS_CONFIG_LOG_VERBOSITY_STATUS_ALL;
                fprintf(stderr, "Setting verbosity to STATUS_ALL\n");
                break;
        default: fprintf(stderr,
                    "Invalid value for the verbosity parameter. Setting verbosity to ERROR (1)\n");
                break;
    }
    NDDSConfigLogger::get_instance()->set_verbosity(verbosity);
}

const std::string GetDDSVersionString() {
    DDS_ProductVersion_t productVersion =
            NDDSConfigVersion::get_instance().get_product_version();

    return "RTI Connext DDS "
            + std::to_string((int) productVersion.major) + "."
            + std::to_string((int) productVersion.minor) + "."
            + std::to_string((int) productVersion.release);
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
        ParameterManager *_PM)
{
    if (!_PM->get<std::string>("allowInterfaces").empty()) {

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
                    _PM->get<std::string>("allowInterfaces"))) {
                return false;
            }

            propertyName =
                    "dds.transport.UDPv6.builtin.parent.allow_interfaces";

            if (!addPropertyToParticipantQos(
                    qos,
                    propertyName,
                    _PM->get<std::string>("allowInterfaces"))) {
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
                    _PM->get<std::string>("allowInterfaces"));
        }
    }

    return true;
}

bool setTransportVerbosity(
        PerftestTransport &transport,
        DDS_DomainParticipantQos &qos,
        ParameterManager *_PM)
{
    if (!_PM->get<std::string>("transportVerbosity").empty()) {
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
                _PM->get<std::string>("transportVerbosity"));
    }
    return true;
}

bool configureSecurityFiles(
        PerftestTransport &transport,
        DDS_DomainParticipantQos& qos,
        ParameterManager *_PM)
{

    if (!_PM->get<std::string>("transportCertAuthority").empty()) {
        if (!addPropertyToParticipantQos(
                qos,
                transport.transportConfig.prefixString + ".tls.verify.ca_file",
                _PM->get<std::string>("transportCertAuthority"))) {
            return false;
        }
    }

    if (!_PM->get<std::string>("transportCertFile").empty()) {
        if (!addPropertyToParticipantQos(
                qos,
                transport.transportConfig.prefixString
                        + ".tls.identity.certificate_chain_file",
                _PM->get<std::string>("transportCertFile"))) {
            return false;
        }
    }

    if (!_PM->get<std::string>("transportPrivateKey").empty()) {
        if (!addPropertyToParticipantQos(
                qos,
                transport.transportConfig.prefixString
                        + ".tls.identity.private_key_file",
                _PM->get<std::string>("transportPrivateKey"))) {
            return false;
        }
    }

    return true;
}

bool configureTcpTransport(
        PerftestTransport &transport,
        DDS_DomainParticipantQos& qos,
        ParameterManager *_PM)
{
    qos.transport_builtin.mask = DDS_TRANSPORTBUILTIN_MASK_NONE;

    if (!addPropertyToParticipantQos(
            qos,
            std::string("dds.transport.load_plugins"),
            transport.transportConfig.prefixString)) {
        return false;
    }

    if (!_PM->get<std::string>("transportServerBindPort").empty()) {
        if (!addPropertyToParticipantQos(
                qos,
                transport.transportConfig.prefixString + ".server_bind_port",
                _PM->get<std::string>("transportServerBindPort"))) {
            return false;
        }
    }

    if (_PM->get<bool>("transportWan")) {
        if (!assertPropertyToParticipantQos(
                qos,
                transport.transportConfig.prefixString
                        + ".parent.classid",
                "NDDS_TRANSPORT_CLASSID_TCPV4_WAN")) {
            return false;
        }

        if (_PM->get<std::string>("transportServerBindPort") != "0") {
            if (!_PM->get<std::string>("transportPublicAddress").empty()) {
                if (!addPropertyToParticipantQos(
                        qos,
                        transport.transportConfig.prefixString
                                + ".public_address",
                        _PM->get<std::string>("transportPublicAddress"))) {
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
        if (_PM->get<bool>("transportWan")) {
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

        if (!configureSecurityFiles(transport, qos, _PM)) {
            return false;
        }
    }

    return true;
}

bool configureDtlsTransport(
        PerftestTransport &transport,
        DDS_DomainParticipantQos& qos,
        ParameterManager *_PM)
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

    if (!configureSecurityFiles(transport, qos, _PM)) {
        return false;
    }

    return true;
}

bool configureWanTransport(
        PerftestTransport &transport,
        DDS_DomainParticipantQos& qos,
        ParameterManager *_PM)
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

    if (!_PM->get<std::string>("transportWanServerAddress").empty()) {
        if (!addPropertyToParticipantQos(
                qos,
                transport.transportConfig.prefixString + ".server",
                _PM->get<std::string>("transportWanServerAddress"))) {
            return false;
        }
    } else {
        fprintf(stderr,
                "%s Wan Server Address is required\n",
                classLoggingString.c_str());
        return false;
    }

    if (!_PM->get<std::string>("transportWanServerPort").empty()) {
        if (!addPropertyToParticipantQos(
                qos,
                transport.transportConfig.prefixString + ".server_port",
                _PM->get<std::string>("transportWanServerPort"))) {
            return false;
        }
    }

    if (!_PM->get<std::string>("transportWanId").empty()) {
        if (!addPropertyToParticipantQos(
                qos,
                transport.transportConfig.prefixString
                        + ".transport_instance_id",
                _PM->get<std::string>("transportWanId"))) {
            return false;
        }
    } else {
        fprintf(stderr, "%s Wan ID is required\n", classLoggingString.c_str());
        return false;
    }

    if (_PM->get<bool>("transportSecureWan")) {
        if (!addPropertyToParticipantQos(
                qos,
                transport.transportConfig.prefixString + ".enable_security",
                "1")) {
            return false;
        }

        if (!configureSecurityFiles(transport, qos, _PM)) {
            return false;
        }
    }

    return true;
}

bool configureShmemTransport(
        PerftestTransport &transport,
        DDS_DomainParticipantQos& qos,
        ParameterManager *_PM)
{
  /**
   * OSPAI do not support SHMEM for Android yet
   */
  #if !defined(RTI_ANDROID)
    DDS_Property_t *parentProp =
            DDSPropertyQosPolicyHelper::lookup_property(qos.property,
                    "dds.transport.shmem.builtin.parent.message_size_max");
    int parentMsgSizeMax = atoi(parentProp->value);

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
    int minBufferSize = parentMsgSizeMax;
    int step = 1048576 + parentMsgSizeMax; // 1MB + parentMsgSizeMax
    int maxBufferSize = 60817408; // 58MB

    do {
        // Reset handles to known state
        RTIOsapiMemory_zero(&handle,
                sizeof(struct RTIOsapiSharedMemorySegmentHandle));

        success = RTIOsapiSharedMemorySegment_create(
                &handle, &retcode, key, maxBufferSize, pid);

        RTIOsapiSharedMemorySegment_delete(&handle);
        maxBufferSize -= step;
    } while (maxBufferSize > minBufferSize && success == RTI_FALSE);

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
    std::ostringstream ss;
    /*
     * This is the flow Controller default token size. Change this if you modify
     * the qos file to add a different "bytes_per_token" property
     */
    int flowControllerTokenSize = 65536;
    unsigned long long datalen = _PM->get<unsigned long long>("dataLen");

  #ifdef RTI_FLATDATA_AVAILABLE
    // Zero Copy sends 16-byte references
    if (_PM->get<bool>("zerocopy")) {
        datalen = 16;
    }
  #endif

    /*
     * We choose the minimum between the flow Controller max fragment size and
     * the message_size_max - RTPS headers.
     */
    int fragmentSize = (std::min)(
            parentMsgSizeMax - COMMEND_WRITER_MAX_RTPS_OVERHEAD,
            flowControllerTokenSize);

    // max(1, (sample_serialized_size / fragmentSize))
    unsigned long long int rtpsMessagesPerSample = (std::max)(
            1ull, (perftest_cpp::OVERHEAD_BYTES + datalen) / fragmentSize);

    unsigned long long int receivedMessageCountMax =
            2 * (_PM->get<int>("sendQueueSize") + 1) * rtpsMessagesPerSample;

    // min(maxBufferSize, receivedMessageCountMax * rtps_message_size)
    unsigned long long int receiveBufferSize = (std::min)(
        (unsigned long long) maxBufferSize,
        receivedMessageCountMax *
                (COMMEND_WRITER_MAX_RTPS_OVERHEAD + fragmentSize));

    ss << receivedMessageCountMax;
    if (!assertPropertyToParticipantQos(
            qos,
            "dds.transport.shmem.builtin.received_message_count_max",
            ss.str())) {
        return false;
    }

    ss.str("");
    ss.clear();
    ss << receiveBufferSize;
    if (!assertPropertyToParticipantQos(
            qos,
            "dds.transport.shmem.builtin.receive_buffer_size",
            ss.str())) {
        return false;
    }

    return true;
  #else
    // Not supported yet.
    return false;
  #endif
}

bool PerftestConfigureTransport(
        PerftestTransport &transport,
        DDS_DomainParticipantQos &qos,
        ParameterManager *_PM)
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
            if (!configureShmemTransport(transport, qos, _PM)) {
                fprintf(stderr,
                        "%s Failed to configure SHMEM plugin\n",
                        classLoggingString.c_str());
                return false;
            }
            break;

        case TRANSPORT_TCPv4:
            if (!configureTcpTransport(transport, qos, _PM)) {
                fprintf(stderr,
                        "%s Failed to configure TCP plugin\n",
                        classLoggingString.c_str());
                return false;
            }
            break;

        case TRANSPORT_TLSv4:
            if (!configureTcpTransport(transport, qos, _PM)) {
                fprintf(stderr,
                        "%s Failed to configure TCP + TLS plugin\n",
                        classLoggingString.c_str());
                return false;
            }
            break;

        case TRANSPORT_DTLSv4:
            if (!configureDtlsTransport(transport, qos, _PM)) {
                fprintf(stderr,
                        "%s Failed to configure DTLS plugin\n",
                        classLoggingString.c_str());
                return false;
            }
            break;

        case TRANSPORT_WANv4:
            if (!configureWanTransport(transport, qos, _PM)) {
                fprintf(stderr,
                        "%s Failed to configure WAN plugin\n",
                        classLoggingString.c_str());
                return false;
            }
            break;
        default:

            /*
             * If shared memory is enabled we want to set up its
             * specific configuration
             */
            if ((qos.transport_builtin.mask & DDS_TRANSPORTBUILTIN_SHMEM)
                    != 0) {
                if (!configureShmemTransport(transport, qos, _PM)) {
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
        if (!setAllowInterfacesList(transport, qos, _PM)) {
            return false;
        }
    } else {
        // We are not using the allow interface string, so it is clean
        _PM->set<std::string>("allowInterfaces", std::string(""));
    }

    if (!setTransportVerbosity(transport, qos, _PM)) {
        return false;
    }

    return true;
}

#ifdef RTI_SECURE_PERFTEST
bool PerftestConfigureSecurity(
        PerftestSecurity &security,
        DDS_DomainParticipantQos &qos,
        ParameterManager *_PM)
{
    // configure use of security plugins, based on provided arguments
    std::string governanceFilePath;

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
        "RTI_Security_PluginSuite_create")) {
        return false;
    }

    if (!addPropertyToParticipantQos(
            qos,
            "com.rti.serv.secure.library",
            _PM->get<std::string>("secureLibrary"))) {
        return false;
    }

  #else // Static library linking

    if(DDSPropertyQosPolicyHelper::assert_pointer_property(
            qos.property,
            "com.rti.serv.secure.create_function_ptr",
            (void *) RTI_Security_PluginSuite_create)
                != DDS_RETCODE_OK) {
        printf("Failed to add pointer_property "
                "com.rti.serv.secure.create_function_ptr\n");
        return false;
    }

  #endif

    /*
     * Below, we are using com.rti.serv.secure properties in order to be
     * backward compatible with RTI Connext DDS 5.3.0 and below. Later versions
     * use the properties that are specified in the DDS Security specification
     * (see also the RTI Security Plugins Getting Started Guide). However,
     * later versions still support the legacy properties as an alternative.
     */

    // check if governance file provided
    if (_PM->get<std::string>("secureGovernanceFile").empty()) {
        // choose a pre-built governance file
        governanceFilePath = "./resource/secure/signed_PerftestGovernance_";
        if (_PM->get<bool>("secureEncryptDiscovery")) {
            governanceFilePath += "Discovery";
        }
        if (_PM->get<bool>("secureSign")) {
            governanceFilePath += "Sign";
        }
        if (_PM->get<bool>("secureEncryptData")
                && _PM->get<bool>("secureEncryptSM")) {
            governanceFilePath += "EncryptBoth";
        } else if (_PM->get<bool>("secureEncryptData")) {
            governanceFilePath += "EncryptData";
        } else if (_PM->get<bool>("secureEncryptSM")) {
            governanceFilePath += "EncryptSubmessage";
        }

        governanceFilePath += ".xml";

        if (!addPropertyToParticipantQos(
                qos,
                "com.rti.serv.secure.access_control.governance_file",
                governanceFilePath)) {
            return false;
        }
    } else {
        governanceFilePath = _PM->get<std::string>("secureGovernanceFile");
        if (!addPropertyToParticipantQos(
                qos,
                "com.rti.serv.secure.access_control.governance_file",
                governanceFilePath)) {
            return false;
        }
    }

    /*
     * Save the local variable governanceFilePath into
     * the parameter "secureGovernanceFile"
     */
    _PM->set("secureGovernanceFile", governanceFilePath);

    // Permissions file
    if (!addPropertyToParticipantQos(
            qos,
            "com.rti.serv.secure.access_control.permissions_file",
            _PM->get<std::string>("securePermissionsFile"))) {
        return false;
    }

    // permissions authority file
    if (!addPropertyToParticipantQos(
            qos,
            "com.rti.serv.secure.access_control.permissions_authority_file",
            _PM->get<std::string>("secureCertAuthority"))) {
        return false;
    }

    // certificate authority
    if (!addPropertyToParticipantQos(
            qos,
            "com.rti.serv.secure.authentication.ca_file",
            _PM->get<std::string>("secureCertAuthority"))) {
        return false;
    }

    // public key
    if (!addPropertyToParticipantQos(
            qos,
            "com.rti.serv.secure.authentication.certificate_file",
            _PM->get<std::string>("secureCertFile"))) {
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
            _PM->get<std::string>("securePrivateKey"))) {
        return false;
    }

    if (_PM->is_set("secureDebug")) {
        // private key
        if (!addPropertyToParticipantQos(
                qos,
                "com.rti.serv.secure.logging.log_level",
                std::to_string(_PM->get<int>("secureDebug")))) {
            return false;
        }
    }

    return true;
}
#endif
