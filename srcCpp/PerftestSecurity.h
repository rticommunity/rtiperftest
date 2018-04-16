/*
 * (c) 2005-2018  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */
#ifdef RTI_SECURE_PERFTEST

#ifndef PERFTEST_SECURITY_H
#define PERFTEST_SECURITY_H

#include <string>
#include <map>
#include <sstream>

/******************************************************************************/

class PerftestSecurity {

public:

    /**************************************************************************/
    /* PUBLIC CLASS MEMBERS */

    bool useSecurity;
    bool signPackages;
    bool dataEncrypted; // user data
    bool subMessageEncrypted;   // submessage
    bool discoveryEncrypted;
    std::string certAuthorityFile;
    std::string certificateFile;
    std::string privateKeyFile;
    std::string governanceFile;
    std::string permissionsFile;
    std::string library;
    int  debugLevel;

    /**************************************************************************/
    /* CLASS CONSTRUCTOR AND DESTRUCTOR */

    PerftestSecurity();

    /**************************************************************************/
    /* PUBLIC METHODS */

    static std::map<std::string, unsigned int> getSecurityCmdLineArgs();

    std::string helpMessageString();

    bool parseSecurityOptions(int argc, char *argv[]);
    bool validateSecureArgs(bool _isPublisher);
    void printSecurityConfigurationSummary();

};

#endif /* PERFTEST_SECURITY_H */
#endif /* RTI_SECURE_PERFTEST */
