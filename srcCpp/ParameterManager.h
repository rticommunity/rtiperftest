#ifndef __PARAMETERMANAGER_H__
#define __PARAMETERMANAGER_H__


#include "Parameter.h"
#include <iostream>
#include <sstream>
#include <map>

class ParameterManager
{
    private:
        std::map<std::string, Parameter_base*> listParameter;

    public:
        ParameterManager()
        {

        }
        bool initialize(){
            // check for error
            try {
                Parameter<int> *batching = new Parameter<int>();
                batching->setCommandLineArgument(std::make_pair("-batchsize","<bytes>"));
                batching->setDescription("Size in bytes of batched message, default 8kB.\n(Disabled for LatencyTest mode or if dataLen > 4kB)");
                batching->setType(t_int);
                batching->setValue(0);
                batching->setNumArguments(1);
                batching->setRange(0, 63000);
                listParameter["batching"] = batching;
            } catch(std::bad_alloc &ex) {
                fprintf(stderr, "Exception in ParameterManager::initialize(): %s.\n", ex.what());
                return false;
            }

            try {
                Parameter<std::string> *flowController = new Parameter<std::string>();
                flowController->setCommandLineArgument(std::make_pair("-flowController", "<flow>"));
                flowController->setDescription("In the case asynchronous writer use a specific flow controller.\nThere are several flow controller predefined:\n\t{'default', '1Gbps', '10Gbps'}\nDefault: \"default\" (If using asynchronous).");
                flowController->setType(t_string);
                flowController->setValue("default");
                flowController->setNumArguments(1);
                flowController->addValidStrValues("default");
                flowController->addValidStrValues("1Gbps");
                flowController->addValidStrValues("10Gbps");
                listParameter["flowController"] = flowController;
            } catch(std::bad_alloc &ex) {
                fprintf(stderr, "Exception in ParameterManager::initialize(): %s.\n", ex.what());
                return false;
            }

            try {
                Parameter<std::string> *nic = new Parameter<std::string>();
                nic->setCommandLineArgument(std::make_pair("-nic","<ipaddr>"));
                nic->setDescription("Use only the nic specified by <ipaddr>.\nIf not specified, use all available interfaces");
                nic->setType(t_string);
                nic->setValue("");
                nic->setNumArguments(1);
                listParameter["nic"] = nic;
            } catch(std::bad_alloc &ex) {
                fprintf(stderr, "Exception in ParameterManager::initialize(): %s.\n", ex.what());
                return false;
            }

            try {
                Parameter<bool> * pub = new Parameter<bool>();
                pub->setCommandLineArgument(std::make_pair("-pub",""));
                pub->setDescription("Set test to be a publisher");
                pub->setType(t_bool);
                pub->setNumArguments(0);
                pub->setValue(false);
                listParameter["pub"] = pub;
            } catch(std::bad_alloc &ex) {
                fprintf(stderr, "Exception in ParameterManager::initialize(): %s.\n", ex.what());
                return false;
            }
        }


        // Get the value of a parameter
        template <typename T>
        T query(std::string parameterKey)
        {
            std::map<std::string, Parameter_base*>::iterator it;
            it = listParameter.find(parameterKey);
            if (it != listParameter.end()) {
                Parameter_base* p = listParameter[parameterKey];
                return ((Parameter<T>*)p)->getValue();
            } else {
                return T(); // Return the default
                // TODO throw exception
            }
        }

        // Parse the command line parameters and set the value
        bool parse(int argc, char *argv[])
        {
            bool success = true;
            // Copy all arguments into a container of strings
            std::vector<std::string> allArgs(argv, argv + argc);
            std::map<std::string, Parameter_base *>::iterator it;
            for(unsigned int i = 1; i < allArgs.size(); i++) {
                for (it = listParameter.begin(); it != listParameter.end(); it++) {
                    if (allArgs[i] == it->second->getCommandLineArgument().first) { // check for small string compare
                        // NumArguments == 0
                        if (it->second->getnumArguments() == 0){
                            // Type is t_bool
                            if (it->second->getType() == t_bool) {
                                ((Parameter<bool>*)it->second)->setValue(true);
                            }
                        // NumArguments == 1
                        } else if (it->second->getnumArguments() == 1) {
                            // Check for error in num of arguments
                            if (++i >= allArgs.size() || allArgs[i].find("-") == 0) {
                                fprintf(stderr, "Missing '%s' after '%s'\n",
                                        it->second->getCommandLineArgument().second.c_str(),
                                        it->second->getCommandLineArgument().first.c_str());
                                return false;
                            }
                            // Type is t_string
                            if (it->second->getType() == t_string) {
                                if (!it->second->validateStrRange(allArgs[i])){
                                    success = false;
                                }
                                ((Parameter<std::string>*)it->second)->setValue(allArgs[i]);
                            }
                            // Type is t_string
                            else if (it->second->getType() == t_int) {
                                int var;
                                if (sscanf(allArgs[i].c_str(), "%d", &var) != 1) {
                                    fprintf(stderr, "%s\n", it->second->getErrorMessage().c_str());
                                    success = false;
                                }
                                if (!it->second->validateNumericRange(var)){
                                    success = false;
                                }
                                ((Parameter<int>*)it->second)->setValue(var);
                            }
                        }
                        it->second->setIsSet(true);
                        break;
                    }
                }
            }
            return success;
        }

        // Get the help message
        std::string displayHelp()
        {
            std::map<std::string, Parameter_base *>::iterator it;
            std::ostringstream oss;
            oss << "/**********************************************************************************************/\n"
                << "Usage:\t perftest_cpp [options]\n"
                << "Where [options] are:\n\n";
            for (it = listParameter.begin(); it != listParameter.end(); it++){
                if (!it->second->getInternal()) {
                    oss << printCommandLineParameter(it->second);
                }
            }
            oss << "/**********************************************************************************************/\n";
            return oss.str();
        }

        // check -help option
        bool checkHelp(int argc, char *argv[])
        {
            std::vector<std::string> allArgs(argv, argv + argc);
            for(unsigned int i = 1; i < allArgs.size(); i++) {
                if (allArgs[i] == "-help" || allArgs[i] == "-h") {
                    std::cout <<  displayHelp() <<'\n';
                    return true;
                }
            }
            return false;
        }

        ~ParameterManager()
        {
            std::map<std::string, Parameter_base *>::iterator it;
            for (it = listParameter.begin(); it != listParameter.end(); it++) {
                delete it->second;
            }
            listParameter.clear();
        }

        // check if a variable has been set
        bool isSet(std::string parameterKey)
        {
            std::map<std::string, Parameter_base*>::iterator it;
            it = listParameter.find(parameterKey);
            if (it != listParameter.end()) {
                return listParameter[parameterKey]->getIsSet();
            } else {
                return false;
            }
        }

        // // validate the listParameter
        // //bool validate(map<string,description> listParameiterDescriptipon) {
        // //}


    private:
        // Get the help message
        std::string printCommandLineParameter(Parameter_base *parameterValue)
        {
            const std::string spaces (42, ' ');
            std::string description = parameterValue->getDescription();
            std::size_t foundPosition = description.find("\n");
            while (foundPosition != std::string::npos) {
                description.insert(foundPosition + 1, spaces);
                foundPosition = description.find("\n", foundPosition + 1);
            }
            std::ostringstream oss;
            oss << "\t"
                <<  parameterValue->getCommandLineArgument().first
                << " "
                << parameterValue->getCommandLineArgument().second
                << std::string(32 - (parameterValue->getCommandLineArgument().first.size() + parameterValue->getCommandLineArgument().second.size()), ' ' )
                << "- "
                << description
                << "\n";
            return oss.str();
        }

};

#endif // __PARAMETERMANAGER_H__