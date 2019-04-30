/*
 * (c) 2005-2019  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */
#ifdef RTI_SECURE_PERFTEST

#ifndef PERFTEST_SECURITY_H
#define PERFTEST_SECURITY_H

#include <string>
#include <map>
#include <sstream>
#include "ParameterManager.h"

/******************************************************************************/

class PerftestSecurity {

public:

    /**************************************************************************/
    /* PUBLIC CLASS MEMBERS */

    bool useSecurity;
    bool signPackages;
    bool dataEncrypted; // user data
    bool subMessageEncrypted; // submessage
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

    ~PerftestSecurity();
    void initialize(ParameterManager *PM);

    /**************************************************************************/
    /* PUBLIC METHODS */

    bool validateSecureArgs();
    std::string printSecurityConfigurationSummary();

private:

    /**************************************************************************/
    /* PRIVATE CLASS MEMBERS */

    ParameterManager *_PM;

};

#endif /* PERFTEST_SECURITY_H */
#endif /* RTI_SECURE_PERFTEST */
