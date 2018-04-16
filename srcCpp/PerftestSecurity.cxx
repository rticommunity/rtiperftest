/*
 * (c) 2005-2017  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */

#ifdef RTI_SECURE_PERFTEST

#include "PerftestSecurity.h"
#include "Infrastructure_common.h"

/******************************************************************************/

// Default location of the security related files
const std::string SECURE_PRIVATEKEY_FILE_PUB =
        "./resource/secure/pubkey.pem";
const std::string SECURE_PRIVATEKEY_FILE_SUB =
        "./resource/secure/subkey.pem";
const std::string SECURE_CERTIFICATE_FILE_PUB =
        "./resource/secure/pub.pem";
const std::string SECURE_CERTIFICATE_FILE_SUB =
        "./resource/secure/sub.pem";
const std::string SECURE_CERTAUTHORITY_FILE =
        "./resource/secure/cacert.pem";
const std::string SECURE_PERMISION_FILE_PUB =
        "./resource/secure/signed_PerftestPermissionsPub.xml";
const std::string SECURE_PERMISION_FILE_SUB =
        "./resource/secure/signed_PerftestPermissionsSub.xml";
const std::string SECURE_LIBRARY_NAME = "nddssecurity";

/******************************************************************************/
/* CLASS CONSTRUCTOR AND DESTRUCTOR */

PerftestSecurity::PerftestSecurity() :
    useSecurity(false),
    signPackages(false),
    dataEncrypted(false),
    subMessageEncrypted(false),
    discoveryEncrypted(false),
    debugLevel(-1)
{
}

/******************************************************************************/
/* PUBLIC METHODS */

std::map<std::string, unsigned int> PerftestSecurity::getSecurityCmdLineArgs()
{

    std::map<std::string, unsigned int> cmdLineArgsMap;

    cmdLineArgsMap["-secureEncryptDiscovery"] = 1;
    cmdLineArgsMap["-secureSign"] = 0;
    cmdLineArgsMap["-secureEncryptData"] = 0;
    cmdLineArgsMap["-secureEncryptSM"] = 0;
    cmdLineArgsMap["-secureGovernanceFile"] = 1;
    cmdLineArgsMap["-securePermissionsFile"] = 1;
    cmdLineArgsMap["-secureCertAuthority"] = 1;
    cmdLineArgsMap["-secureCertFile"] = 1;
    cmdLineArgsMap["-securePrivateKey"] = 1;
    cmdLineArgsMap["-secureLibrary"] = 1;
    cmdLineArgsMap["-secureDebug"] = 1;

    return cmdLineArgsMap;
}

std::string PerftestSecurity::helpMessageString()
{
    std::ostringstream oss;
    oss
    << "\t===================== Security Specific Options ======================\n"
    << "\n"
    << "\t-secureEncryptDiscovery       - Encrypt discovery traffic\n"
    << "\t-secureSign                   - Sign (HMAC) discovery and user data\n"
    << "\t-secureEncryptData            - Encrypt topic (user) data\n"
    << "\t-secureEncryptSM              - Encrypt RTPS submessages\n"
    << "\t-secureGovernanceFile <file>  - Governance file. If specified, the authentication,\n"
    << "\t                                signing, and encryption arguments are ignored. The\n"
    << "\t                                Governance document configuration will be used instead\n"
    << "\t                                Default: built using the secure options.\n"
    << "\t-securePermissionsFile <file> - Permissions file <optional>\n"
    << "\t                                Default: \"./resource/secure/signed_PerftestPermissionsSub.xml\"\n"
    << "\t-secureCertAuthority <file>   - Certificate authority file <optional>\n"
    << "\t                                Default: \"./resource/secure/cacert.pem\"\n"
    << "\t-secureCertFile <file>        - Certificate file <optional>\n"
    << "\t                                Default: \"./resource/secure/sub.pem\"\n"
    << "\t-securePrivateKey <file>      - Private key file <optional>\n"
    << "\t                                Default: \"./resource/secure/subkey.pem\"\n";
    return oss.str();
}

bool PerftestSecurity::parseSecurityOptions(int argc, char *argv[])
{
    // We will only parse the properties related with transports here.
    for (int i = 0; i < argc; ++i) {

        if (IS_OPTION(argv[i], "-secureSign")) {
            signPackages = true;
            useSecurity = true;
        }
        else if (IS_OPTION(argv[i], "-secureEncryptBoth")) {
            dataEncrypted = true;
            subMessageEncrypted = true;
            useSecurity = true;
        }
        else if (IS_OPTION(argv[i], "-secureEncryptData")) {
            dataEncrypted = true;
            useSecurity = true;
        }
        else if (IS_OPTION(argv[i], "-secureEncryptSM")) {
            subMessageEncrypted = true;
            useSecurity = true;
        }
        else if (IS_OPTION(argv[i], "-secureEncryptDiscovery")) {
            discoveryEncrypted = true;
            useSecurity = true;
        }
        else if (IS_OPTION(argv[i], "-secureGovernanceFile")) {
            if ((i == (argc-1)) || *argv[++i] == '-') {
               fprintf(stderr, "Missing <file> after -secureGovernanceFile\n");
               return false;
            }
            governanceFile = argv[i];
            fprintf(stdout, "Warning -- authentication, encryption, signing arguments "
                    "will be ignored, and the values specified by the Governance file will "
                    "be used instead\n");
            useSecurity = true;
        }
        else if (IS_OPTION(argv[i], "-securePermissionsFile")) {
            if ((i == (argc-1)) || *argv[++i] == '-') {
                fprintf(stderr, "Missing <file> after -securePermissionsFile\n");
                return false;
            }
            permissionsFile = argv[i];
            useSecurity = true;
        }
        else if (IS_OPTION(argv[i], "-secureCertAuthority")) {
            if ((i == (argc-1)) || *argv[++i] == '-') {
                fprintf(stderr, "Missing <file> after -secureCertAuthority\n");
                return false;
            }
            certAuthorityFile = argv[i];
            useSecurity = true;
        }
        else if (IS_OPTION(argv[i], "-secureCertFile")) {
            if ((i == (argc-1)) || *argv[++i] == '-') {
                fprintf(stderr, "Missing <file> after -secureCertFile\n");
                return false;
            }
            certificateFile = argv[i];
            useSecurity = true;
        }
        else if (IS_OPTION(argv[i], "-securePrivateKey")) {
            if ((i == (argc-1)) || *argv[++i] == '-') {
                fprintf(stderr, "Missing <file> after -securePrivateKey\n");
                return false;
            }
            privateKeyFile = argv[i];
            useSecurity = true;
        }
        else if (IS_OPTION(argv[i], "-secureLibrary")) {
            if ((i == (argc-1)) || *argv[++i] == '-') {
                fprintf(stderr, "Missing <file> after -secureLibrary\n");
                return false;
            }
            library = argv[i];
        }
        else if (IS_OPTION(argv[i], "-secureDebug")) {
            if ((i == (argc-1)) || *argv[++i] == '-') {
                fprintf(stderr, "Missing <level> after -secureDebug\n");
                 return false;
            }
            debugLevel = strtol(argv[i], NULL, 10);
        }
    }

    return true;
}

bool PerftestSecurity::validateSecureArgs(bool _isPublisher)
{
    if (useSecurity) {
        if (privateKeyFile.empty()) {
            if (_isPublisher) {
                privateKeyFile = SECURE_PRIVATEKEY_FILE_PUB;
            } else {
                privateKeyFile = SECURE_PRIVATEKEY_FILE_SUB;
            }
        }

        if (certificateFile.empty()) {
            if (_isPublisher) {
                certificateFile = SECURE_CERTIFICATE_FILE_PUB;
            } else {
                certificateFile = SECURE_CERTIFICATE_FILE_SUB;
            }
        }

        if (certAuthorityFile.empty()) {
            certAuthorityFile = SECURE_CERTAUTHORITY_FILE;
        }

        if (permissionsFile.empty()) {
            if (_isPublisher) {
                permissionsFile = SECURE_PERMISION_FILE_PUB;
            } else {
                permissionsFile = SECURE_PERMISION_FILE_SUB;
            }
        }

      #ifdef RTI_PERFTEST_DYNAMIC_LINKING
        if (library.empty()) {
            library = SECURE_LIBRARY_NAME;
        }
      #endif

    }

    return true;
}

void PerftestSecurity::printSecurityConfigurationSummary()
{
    std::ostringstream stringStream;
    stringStream << "Security Plugins Information:\n";

    stringStream << "\tEncrypt discovery: "
                 << (discoveryEncrypted ? "True" : "False")
                 << "\n";

    stringStream << "\tEncrypt topic (user) data: "
                 << (dataEncrypted ? "True" : "False")
                 << "\n";

    stringStream << "\tEncrypt submessage: "
                 << (subMessageEncrypted ? "True" : "False")
                 << "\n";

    stringStream << "\tSign data: "
                 << (signPackages ? "True" : "False")
                 << "\n";

    stringStream << "\tGovernance file: "
                 << (governanceFile.empty() ? "Not Specified" : governanceFile)
                 << "\n";

    stringStream << "\tPermissions file: "
                 << (permissionsFile.empty() ? "Not Specified" : permissionsFile)
                 << "\n";

    stringStream << "\tPrivate key file: "
                 << (privateKeyFile.empty() ? "Not Specified" : privateKeyFile)
                 << "\n";

    stringStream << "\tCertificate file: "
                 << (certificateFile.empty() ? "Not Specified" : certificateFile)
                 << "\n";

    stringStream << "\tCertificate authority file: "
                 << (certAuthorityFile.empty() ? "Not Specified" : certAuthorityFile)
                 << "\n";

    stringStream << "\tPlugin library: "
                 << (library.empty() ? "Not Specified" : library)
                 << "\n";

    if( debugLevel != -1 ){
        stringStream << "\tDebug level: " << debugLevel << "\n";
    }

    fprintf(stderr, "%s", stringStream.str().c_str());
}

/******************************************************************************/

#endif /* RTI_SECURE_PERFTEST */
