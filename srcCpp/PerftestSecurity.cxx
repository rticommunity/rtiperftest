/*
 * (c) 2005-2019  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */

#ifdef RTI_SECURE_PERFTEST

#include "PerftestSecurity.h"
#include "Infrastructure_common.h"

/******************************************************************************/

// Default location of the security related files
#ifdef RTI_MICRO
    const std::string prefix = "file:";
#else
    const std::string prefix = "";
#endif

const std::string SECURE_PRIVATEKEY_FILE_PUB =
        prefix + "./resource/secure/pubkey.pem";
const std::string SECURE_PRIVATEKEY_FILE_SUB =
        prefix + "./resource/secure/subkey.pem";
const std::string SECURE_CERTIFICATE_FILE_PUB =
        prefix + "./resource/secure/pub.pem";
const std::string SECURE_CERTIFICATE_FILE_SUB =
        prefix + "./resource/secure/sub.pem";
const std::string SECURE_CERTAUTHORITY_FILE =
        prefix + "./resource/secure/cacert.pem";
const std::string SECURE_PERMISION_FILE_PUB =
        prefix + "./resource/secure/signed_PerftestPermissionsPub.xml";
const std::string SECURE_PERMISION_FILE_SUB =
        prefix + "./resource/secure/signed_PerftestPermissionsSub.xml";
const std::string SECURE_LIBRARY_NAME = "nddssecurity";

/******************************************************************************/
/* CLASS CONSTRUCTOR AND DESTRUCTOR */

PerftestSecurity::PerftestSecurity()
{
}

void PerftestSecurity::initialize(ParameterManager *PM)
{
    _PM = PM;
}

/******************************************************************************/
/* PUBLIC METHODS */

bool PerftestSecurity::validateSecureArgs()
{
    if (_PM->group_is_used(SECURE)) {

        // Manage parameter -secureGovernanceFile
        if (_PM->is_set("secureGovernanceFile")) {
            fprintf(stdout,
                "Warning -- authentication, encryption, signing arguments "
                "will be ignored, and the values specified by the Governance "
                "file will be used instead\n");
        }

        // Manage parameter -secureEncryptBoth
        if (_PM->is_set("secureEncryptBoth")) {
            _PM->set("secureEncryptData", true);
            _PM->set("secureEncryptSM", true);
        }

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

std::string PerftestSecurity::printSecurityConfigurationSummary()
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

#endif /* RTI_SECURE_PERFTEST */
