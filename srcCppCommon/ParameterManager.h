/*
 * (c) 2005-2018  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */

#ifndef __PARAMETERMANAGER_H__
#define __PARAMETERMANAGER_H__

#ifdef RTI_LANGUAGE_CPP_TRADITIONAL
  #include "perftest.h"
#elif defined(RTI_LANGUAGE_CPP_MODERN)
  #include "perftest.hpp"
#endif
#include "Parameter.h"
#include <map>

#if defined(RTI_WIN32) || defined(RTI_INTIME)
  #pragma warning(push)
  #pragma warning(disable : 4996)
  #define STRNCASECMP _strnicmp
#elif defined(RTI_VXWORKS)
  #define STRNCASECMP strncmp
#else
  #define STRNCASECMP strncasecmp
#endif
#define IS_OPTION(str, option) (STRNCASECMP(str, option, strlen(str)) == 0)



/******************************************************************************/
// Default location of the security related files
const std::string TRANSPORT_PRIVATEKEY_FILE_PUB = "./resource/secure/pubkey.pem";
const std::string TRANSPORT_PRIVATEKEY_FILE_SUB = "./resource/secure/subkey.pem";
const std::string TRANSPORT_CERTIFICATE_FILE_PUB = "./resource/secure/pub.pem";
const std::string TRANSPORT_CERTIFICATE_FILE_SUB = "./resource/secure/sub.pem";
const std::string TRANSPORT_CERTAUTHORITY_FILE = "./resource/secure/cacert.pem";

// Default Multicast IP per topic
const std::string TRANSPORT_MULTICAST_ADDR_LATENCY = "239.255.1.2";
const std::string TRANSPORT_MULTICAST_ADDR_ANNOUNCEMENT = "239.255.1.100";
const std::string TRANSPORT_MULTICAST_ADDR_THROUGHPUT = "239.255.1.1";


class ParameterManager
{
    private:
        std::map<std::string, AnyParameter> _parameterList;

    public:
        ParameterManager();
        ParameterManager(bool perftestForMicro);
        void initialize();
        ~ParameterManager();

        // Set the value of a parameter
        template <typename T>
        void set(std::string parameterKey, T var)
        {
            std::map<std::string, AnyParameter>::iterator it =
                    _parameterList.find(parameterKey);
            if (it != _parameterList.end()) {
                (static_cast<Parameter<T>*>(it->second.get<T>()))->set_value(var);
            }
        }

        // Get the value of a parameter
        template <typename T>
        T get(std::string parameterKey)
        {
            std::map<std::string, AnyParameter>::iterator it =
                    _parameterList.find(parameterKey);
            if (it != _parameterList.end()) {
                return (static_cast<Parameter<T>*>(
                        it->second.get<T>()))->get_value();
            } else {
                return T(); // Return the default
            }
        }

        // Get a vector with the values of a parameter
        template <typename T>
        std::vector<T> get_vector(std::string parameterKey)
        {
            std::map<std::string, AnyParameter>::iterator it =
                    _parameterList.find(parameterKey);
            if (it != _parameterList.end()) {
                return (static_cast<ParameterVector<T>*>(
                        it->second.get_vector<T>()))->get_value();
            } else {
                return std::vector<T>(); // Return the default
            }
        }

        // Get a vector with the values of a parameter
        template <typename K, typename V>
        std::pair<K,V> get_pair(std::string parameterKey)
        {
            std::map<std::string, AnyParameter>::iterator it =
                    _parameterList.find(parameterKey);
            if (it != _parameterList.end()) {
                return (static_cast<ParameterPair<K,V>*>(
                    it->second.get_pair<K,V>()))->get_value();
            } else {
                return std::pair<K,V>(); // Return the default
            }
        }

        // Parse the command line parameters and set the values
        bool parse(int argc, char *argv[]);

        // Get the help message
        std::string display_help();

        // Check -help option
        bool check_help(int argc, char *argv[]);

        // Check if a variable has been set
        bool is_set(std::string parameterKey);

        /*
         * Validate:
         *     That if -pub not use parameter of SUB group
         *     That if -sub not use parameter of PUB group
         */
        bool check_incompatible_parameters();

        // Verify if there is a parameter of the group set
        bool group_is_used(Group group);


    private:
        bool perftestForMicro;
        std::vector<std::string> split(std::string str, std::string delimiter = ":");
        std::string get_center_header_help_line(std::string name);

        /*
         * Add a parameter to the _parameterList.
         *      Create a local AnyParameter and add to the map using the method:
         *           AnyParameter& operator=(AnyParameter &other)
         */
        template <typename T>
        void create(std::string key, T *param) {
            AnyParameter var(param);
            _parameterList[key] = var;
        }

};

#endif // __PARAMETERMANAGER_H__
