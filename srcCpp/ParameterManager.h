#ifndef __PARAMETERMANAGER_H__
#define __PARAMETERMANAGER_H__

/*
 * (c) 2005-2018  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */

#include "Parameter.h"
#include <map>

class ParameterManager
{
    private:
        std::map<std::string, Parameter_base*> parameterList;

    public:
        ParameterManager();
        bool initialize();
        ~ParameterManager();

        // Get the value of a parameter
        template <typename T>
        T query(std::string parameterKey)
        {
            std::map<std::string, Parameter_base*>::iterator it;
            it = parameterList.find(parameterKey);
            if (it != parameterList.end()) {
                Parameter_base* p = parameterList[parameterKey];
                    return ((Parameter<T>*)p)->getValue();
            } else {
                return T(); // Return the default
                // TODO throw exception
            }
        }

        // Get a vector with the values of a parameter
        template <typename T>
        std::vector<T> queryVector(std::string parameterKey)
        {
            std::map<std::string, Parameter_base*>::iterator it;
            it = parameterList.find(parameterKey);
            if (it != parameterList.end()) {
                Parameter_base* p = parameterList[parameterKey];
                return ((ParameterVector<T>*)p)->getValue();

            } else {
                return std::vector<T>(); // Return the default
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
        std::string printCommandLineParameter(Parameter_base *parameterValue);
        std::vector<std::string> split(std::string str, char delimiter = ':');

};

#endif // __PARAMETERMANAGER_H__