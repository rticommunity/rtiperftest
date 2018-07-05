#ifndef __PARAMETERMANAGER_H__
#define __PARAMETERMANAGER_H__

/*
 * (c) 2005-2018  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */
#include "PerftestTransport.h"
#include "RTIDDSImpl.h"
#include "Parameter.h"
#include <map>

#if defined(RTI_WIN32)
  #pragma warning(push)
  #pragma warning(disable : 4996)
  #define STRNCASECMP _strnicmp
#elif defined(RTI_VXWORKS)
  #define STRNCASECMP strncmp
#else
  #define STRNCASECMP strncasecmp
#endif
#define IS_OPTION(str, option) (STRNCASECMP(str, option, strlen(str)) == 0)

class ParameterManager
{
    private:
        std::map<std::string, AnyParameter> parameterList;

    public:
        ParameterManager();
        void initialize();
        static ParameterManager &GetInstance();
        ~ParameterManager();

        // Get the value of a parameter
        template <typename T>
        T query(std::string parameterKey)
        {
            std::map<std::string, AnyParameter>::iterator it;
            it = parameterList.find(parameterKey);
            if (it != parameterList.end()) {
                return (static_cast<Parameter<T>*>(parameterList[parameterKey].get<T>()))->getValue();
            } else {
                return T(); // Return the default
                // TODO throw exception
            }
        }

        // Get a vector with the values of a parameter
        template <typename T>
        std::vector<T> queryVector(std::string parameterKey)
        {
          std::map<std::string, AnyParameter>::iterator it;
            it = parameterList.find(parameterKey);
            if (it != parameterList.end()) {
                return (static_cast<ParameterVector<T>*>(parameterList[parameterKey].getVector<T>()))->getValue();
            } else {
                return std::vector<T>(); // Return the default
                // TODO throw exception
            }
        }

        // Get a vector with the values of a parameter
        template <typename K, typename V>
        std::pair<K,V> queryPair(std::string parameterKey)
        {
          std::map<std::string, AnyParameter>::iterator it;
            it = parameterList.find(parameterKey);
            if (it != parameterList.end()) {
                return (static_cast<ParameterPair<K,V>*>(parameterList[parameterKey].getPair<K,V>()))->getValue();
            } else {
                return std::pair<K,V>(); // Return the default
                // TODO throw exception
            }
        }

        // Parse the command line parameters and set the value
        bool parse(int argc, char *argv[]);

        // Get the help message
        std::string displayHelp();

        // check -help option
        bool checkHelp(int argc, char *argv[]);

        // check if a variable has been set
        bool isSet(std::string parameterKey);


    private:
        // Get the help message
        std::string printCommandLineParameter(ParameterBase *parameterValue);
        std::vector<std::string> split(std::string str, char delimiter = ':');

};

#endif // __PARAMETERMANAGER_H__
