
/*******************************************************************************
 * SECURITY PLUGIN
 */
#ifdef RTI_SECURE_PERFTEST

template<typename T>
bool RTIDDSImpl<T>::configureSecurePlugin(DDS_DomainParticipantQos& dpQos) {
    // configure use of security plugins, based on provided arguments

    DDS_ReturnCode_t retcode;
    std::string governanceFilePath;

    // load plugin
    retcode = DDSPropertyQosPolicyHelper::add_property(
            dpQos.property,
            "com.rti.serv.load_plugin",
            "com.rti.serv.secure",
            false);
    if (retcode != DDS_RETCODE_OK) {
        printf("Failed to add property com.rti.serv.load_plugin\n");
        return false;
    }

  #ifdef RTI_PERFTEST_DYNAMIC_LINKING

    retcode = DDSPropertyQosPolicyHelper::assert_property(
            dpQos.property,
            "com.rti.serv.secure.create_function",
            "RTI_Security_PluginSuite_create",
            false);
    if (retcode != DDS_RETCODE_OK) {
        printf("Failed to add property com.rti.serv.secure.create_function\n");
        return false;
    }


    retcode = DDSPropertyQosPolicyHelper::add_property(
            dpQos.property,
            "com.rti.serv.secure.library",
            _PM->get<std::string>("secureLibrary").c_str(),
            false);
    if (retcode != DDS_RETCODE_OK) {
        printf("Failed to add property com.rti.serv.secure.library\n");
        return false;
    }

  #else // Static library linking

    retcode = DDSPropertyQosPolicyHelper::assert_pointer_property(
            dpQos.property,
            "com.rti.serv.secure.create_function_ptr",
            (void *) RTI_Security_PluginSuite_create);
    if (retcode != DDS_RETCODE_OK) {
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

        retcode = DDSPropertyQosPolicyHelper::add_property(
                dpQos.property,
                "com.rti.serv.secure.access_control.governance_file",
                governanceFilePath.c_str(),
                false);
    } else {
        retcode = DDSPropertyQosPolicyHelper::add_property(
                dpQos.property,
                "com.rti.serv.secure.access_control.governance_file",
                governanceFilePath.c_str(),
                false);
    }
    if (retcode != DDS_RETCODE_OK) {
        printf("Failed to add property "
                "com.rti.serv.secure.access_control.governance_file\n");
        return false;
    }

    /*
     * Save the local variable governanceFilePath into
     * the parameter "secureGovernanceFile"
     */
    _PM->set("secureGovernanceFile", governanceFilePath);

    // Permissions file
    retcode = DDSPropertyQosPolicyHelper::add_property(
            dpQos.property,
            "com.rti.serv.secure.access_control.permissions_file",
            _PM->get<std::string>("securePermissionsFile").c_str(),
            false);
    if (retcode != DDS_RETCODE_OK) {
        printf("Failed to add property "
                "com.rti.serv.secure.access_control.permissions_file\n");
        return false;
    }

    // permissions authority file
    retcode = DDSPropertyQosPolicyHelper::add_property(
            dpQos.property,
            "com.rti.serv.secure.access_control.permissions_authority_file",
            _PM->get<std::string>("secureCertAuthority").c_str(),
            false);
    if (retcode != DDS_RETCODE_OK) {
        printf("Failed to add property "
                "com.rti.serv.secure.access_control.permissions_authority_file\n");
        return false;
    }

    // certificate authority
    retcode = DDSPropertyQosPolicyHelper::add_property(
            dpQos.property,
            "com.rti.serv.secure.authentication.ca_file",
            _PM->get<std::string>("secureCertAuthority").c_str(),
            false);
    if (retcode != DDS_RETCODE_OK) {
        printf("Failed to add property "
                "com.rti.serv.secure.authentication.ca_file\n");
        return false;
    }

    // public key
    retcode = DDSPropertyQosPolicyHelper::add_property(
            dpQos.property,
            "com.rti.serv.secure.authentication.certificate_file",
            _PM->get<std::string>("secureCertFile").c_str(),
            false);
    if (retcode != DDS_RETCODE_OK) {
        printf("Failed to add property "
                "com.rti.serv.secure.authentication.certificate_file\n");
        return false;
    }

    retcode = DDSPropertyQosPolicyHelper::add_property(
            dpQos.property,
            "com.rti.serv.secure.cryptography.max_receiver_specific_macs",
            "4",
            false);
    if (retcode != DDS_RETCODE_OK) {
        printf("Failed to add property com.rti.serv.secure.library\n");
        return false;
    }

    // private key
    retcode = DDSPropertyQosPolicyHelper::add_property(
            dpQos.property,
            "com.rti.serv.secure.authentication.private_key_file",
            _PM->get<std::string>("securePrivateKey").c_str(),
            false);
    if (retcode != DDS_RETCODE_OK) {
        printf("Failed to add property "
                "com.rti.serv.secure.authentication.private_key_file\n");
        return false;
    }

    if (_PM->is_set("secureDebug")) {
        char buf[16];
        sprintf(buf, "%d", _PM->get<int>("secureDebug"));
        retcode = DDSPropertyQosPolicyHelper::add_property(
                dpQos.property,
                "com.rti.serv.secure.logging.log_level",
                buf,
                false);
        if (retcode != DDS_RETCODE_OK) {
            printf("Failed to add property "
                    "com.rti.serv.secure.logging.log_level\n");
            return false;
        }
    }

    return true;
}

template <typename T>
bool RTIDDSImpl<T>::validateSecureArgs()
{
    if (_PM->group_is_used(SECURE)) {
        if (_PM->get<std::string>("securePrivateKey").empty()) {
            if (_PM->get<bool>("pub")) {
                _PM->set("securePrivateKey", SECURE_PRIVATEKEY_FILE_PUB);
            } else {
                _PM->set("securePrivateKey", SECURE_PRIVATEKEY_FILE_SUB);
            }
        }

        if (_PM->get<std::string>("secureCertFile").empty()) {
            if (_PM->get<bool>("pub")) {
                _PM->set("secureCertFile", SECURE_CERTIFICATE_FILE_PUB);
            } else {
                _PM->set("secureCertFile", SECURE_CERTIFICATE_FILE_SUB);
            }
        }

        if (_PM->get<std::string>("secureCertAuthority").empty()) {
            _PM->set("secureCertAuthority", SECURE_CERTAUTHORITY_FILE);
        }

        if (_PM->get<std::string>("securePermissionsFile").empty()) {
            if (_PM->get<bool>("pub")) {
                _PM->set("securePermissionsFile", SECURE_PERMISION_FILE_PUB);
            } else {
                _PM->set("securePermissionsFile", SECURE_PERMISION_FILE_SUB);
            }
        }

      #ifdef RTI_PERFTEST_DYNAMIC_LINKING
        if (_PM->get<std::string>("secureLibrary").empty()) {
            _PM->set("secureLibrary", SECURE_LIBRARY_NAME);
        }
      #endif

    }

    return true;
}

template <typename T>
std::string RTIDDSImpl<T>::printSecureArgs()
{
    std::ostringstream stringStream;
    stringStream << "Secure Configuration:\n";

    stringStream << "\tEncrypt discovery: ";
    if (_PM->get<bool>("secureEncryptDiscovery")) {
        stringStream << "True\n";
    } else {
        stringStream << "False\n";
    }

    stringStream << "\tEncrypt topic (user) data: ";
    if (_PM->get<bool>("secureEncryptData")) {
        stringStream << "True\n";
    } else {
        stringStream << "False\n";
    }

    stringStream << "\tEncrypt submessage: ";
    if (_PM->get<bool>("secureEncryptSM")) {
        stringStream << "True\n";
    } else {
        stringStream << "False\n";
    }

    stringStream << "\tSign data: ";
    if (_PM->get<bool>("secureSign")) {
        stringStream << "True\n";
    } else {
        stringStream << "False\n";
    }

    stringStream << "\tGovernance file: ";
    if (_PM->get<std::string>("secureGovernanceFile").empty()) {
        stringStream << "Not Specified\n";
    } else {
        stringStream << _PM->get<std::string>("secureGovernanceFile")
                     << "\n";
    }

    stringStream << "\tPermissions file: ";
    if (_PM->get<std::string>("securePermissionsFile").empty()) {
        stringStream << "Not Specified\n";
    } else {
        stringStream << _PM->get<std::string>("securePermissionsFile")
                     << "\n";
    }

    stringStream << "\tPrivate key file: ";
    if (_PM->get<std::string>("securePrivateKey").empty()) {
        stringStream << "Not Specified\n";
    } else {
        stringStream << _PM->get<std::string>("securePrivateKey")
                     << "\n";
    }

    stringStream << "\tCertificate file: ";
    if (_PM->get<std::string>("secureCertFile").empty()) {
        stringStream << "Not Specified\n";
    } else {
        stringStream << _PM->get<std::string>("secureCertFile")
                     << "\n";
    }

    stringStream << "\tCertificate authority file: ";
    if (_PM->get<std::string>("secureCertAuthority").empty()) {
        stringStream << "Not Specified\n";
    } else {
        stringStream << _PM->get<std::string>("secureCertAuthority")
                     << "\n";
    }

    stringStream << "\tPlugin library: ";
    if (_PM->get<std::string>("secureLibrary").empty()) {
        stringStream << "Not Specified\n";
    } else {
        stringStream << _PM->get<std::string>("secureLibrary")
                     << "\n";
    }

    if (_PM->is_set("secureDebug")) {
        stringStream << "\tDebug level: "
                     << _PM->get<int>("secureDebug")
                     << "\n";
    }

    return stringStream.str();
}

#endif