#ifndef __PARAMETERMANAGER_H__
#define __PARAMETERMANAGER_H__


#include "Parameter.h"
#include <iostream>
#include <sstream>
#include <map>

class ParameterManager
{
    private:
        std::map<std::string, Parameter_base*> parameterList;

    public:
        ParameterManager()
        {
        }
        bool initialize()
        {
            try {
                Parameter<unsigned long long> *batching = new Parameter<unsigned long long>(0);
                batching->setCommandLineArgument(std::make_pair("-batchsize","<bytes>"));
                batching->setDescription("Size in bytes of batched message, default 8kB.\n(Disabled for LatencyTest mode or if dataLen > 4kB)");
                batching->setType(T_NUMERIC);
                batching->setExtraArgument(YES);
                batching->setRange(0, 63000);
                parameterList["batching"] = batching;
            } catch(std::bad_alloc &ex) {
                fprintf(stderr, "Exception in ParameterManager::initialize(): %s.\n", ex.what());
                return false;
            }

            try {
                Parameter<std::string> *flowController = new Parameter<std::string>("default");
                flowController->setCommandLineArgument(std::make_pair("-flowController", "<flow>"));
                flowController->setDescription("In the case asynchronous writer use a specific flow controller.\nThere are several flow controller predefined:\n\t{'default', '1Gbps', '10Gbps'}\nDefault: \"default\" (If using asynchronous).");
                flowController->setType(T_STR);
                flowController->setExtraArgument(YES);
                flowController->addValidStrValue("default");
                flowController->addValidStrValue("1Gbps");
                flowController->addValidStrValue("10Gbps");
                parameterList["flowController"] = flowController;
            } catch(std::bad_alloc &ex) {
                fprintf(stderr, "Exception in ParameterManager::initialize(): %s.\n", ex.what());
                return false;
            }

            try {
                Parameter<std::string> *nic = new Parameter<std::string>("");
                nic->setCommandLineArgument(std::make_pair("-nic","<ipaddr>"));
                nic->setDescription("Use only the nic specified by <ipaddr>.\nIf not specified, use all available interfaces");
                nic->setType(T_STR);
                nic->setExtraArgument(YES);
                parameterList["nic"] = nic;
            } catch(std::bad_alloc &ex) {
                fprintf(stderr, "Exception in ParameterManager::initialize(): %s.\n", ex.what());
                return false;
            }

            try {
                Parameter<bool> * pub = new Parameter<bool>(false);
                pub->setCommandLineArgument(std::make_pair("-pub",""));
                pub->setDescription("Set test to be a publisher");
                pub->setType(T_BOOL);
                pub->setExtraArgument(NO);
                parameterList["pub"] = pub;
            } catch(std::bad_alloc &ex) {
                fprintf(stderr, "Exception in ParameterManager::initialize(): %s.\n", ex.what());
                return false;
            }

            try {
                ParameterVector<std::string> * peer = new ParameterVector<std::string>();
                peer->setCommandLineArgument(std::make_pair("-peer","<address>"));
                peer->setDescription("Adds a peer to the peer host address list.\nThis argument may be repeated to indicate multiple peers");
                peer->setType(T_VECTOR_STR);
                peer->setExtraArgument(YES);
                parameterList["peer"] = peer;
            } catch(std::bad_alloc &ex) {
                fprintf(stderr, "Exception in ParameterManager::initialize(): %s.\n", ex.what());
                return false;
            }

            try {
                std::vector<unsigned long long> _scanDataLenSizes;
                _scanDataLenSizes.push_back(32);
                _scanDataLenSizes.push_back(64);
                _scanDataLenSizes.push_back(128);
                _scanDataLenSizes.push_back(256);
                _scanDataLenSizes.push_back(512);
                _scanDataLenSizes.push_back(1024);
                _scanDataLenSizes.push_back(2048);
                _scanDataLenSizes.push_back(4096);
                _scanDataLenSizes.push_back(8192);
                _scanDataLenSizes.push_back(16384);
                _scanDataLenSizes.push_back(32768);
                _scanDataLenSizes.push_back(63000);
                ParameterVector<unsigned long long> * scan = new ParameterVector<unsigned long long>(_scanDataLenSizes);
                scan->setCommandLineArgument(std::make_pair("-scan","<size1>:<size2>:...:<sizeN>"));
                scan->setDescription("Run test in scan mode, traversing\na range of sample data sizes from\n[32,63000] or [63001,2147483128] bytes,\nin the case that you are using large data or not.\nThe list of sizes is optional.\nDefault values are '32:64:128:256:512:1024:2048:4096:8192:16384:32768:63000'\nDefault: Not set\n");
                scan->setType(T_VECTOR_NUMERIC);
                scan->setExtraArgument(OPTIONAL);
                scan->setRange(32, 2147483128);
                scan->setParseMethod(SPLIT);
                parameterList["scan"] = scan;
            } catch(std::bad_alloc &ex) {
                fprintf(stderr, "Exception in ParameterManager::initialize(): %s.\n", ex.what());
                return false;
            }

            try {
                Parameter<unsigned long long> *unbounded = new Parameter<unsigned long long>(63000);
                unbounded->setCommandLineArgument(std::make_pair("-unbounded","<allocation_threshold>"));
                unbounded->setDescription("Use unbounded Sequences\n<allocation_threshold> is optional, default 63000 Bytes.");
                unbounded->setType(T_NUMERIC);
                unbounded->setExtraArgument(OPTIONAL);
                unbounded->setRange(0, 2147483128);
                parameterList["unbounded"] = unbounded;
            } catch(std::bad_alloc &ex) {
                fprintf(stderr, "Exception in ParameterManager::initialize(): %s.\n", ex.what());
                return false;
            }

            return true;
        }


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
        bool parse(int argc, char *argv[])
        {
            unsigned long long var;
            bool success = true;
            // Copy all arguments into a container of strings
            std::vector<std::string> allArgs(argv, argv + argc);

            std::map<std::string, Parameter_base *>::iterator it;
            for (unsigned int i = 1; i < allArgs.size(); i++) {
                for (it = parameterList.begin(); it != parameterList.end(); it++) {
                    if (allArgs[i] == it->second->getCommandLineArgument().first) { // TODO check for small string compare
                        // NumArguments == 0
                        if (it->second->getExtraArgument() == NO) {
                            // Type is T_BOOL
                            if (it->second->getType() == T_BOOL) {
                                ((Parameter<bool>*)it->second)->setValue(true);
                            }
                        // NumArguments is 1 or optional
                        } else if (it->second->getExtraArgument() > NO) {
                            // Check for error in num of arguments
                            if (i+1 >= allArgs.size() || allArgs[i+1].find("-") == 0) {
                                if (it->second->getExtraArgument() == YES) {
                                    fprintf(stderr, "Missing '%s' after '%s'\n",
                                        it->second->getCommandLineArgument().second.c_str(),
                                        it->second->getCommandLineArgument().first.c_str());
                                    return false;
                                } else if (it->second->getExtraArgument() == OPTIONAL) {
                                    break;
                                }
                            }
                            ++i;
                            // Type is T_STR
                            if (it->second->getType() == T_STR) {
                                if (!it->second->validateStrRange(allArgs[i])) {
                                    success = false;
                                }
                                ((Parameter<std::string>*)it->second)->setValue(allArgs[i]);
                            }
                            // Type is T_NUMERIC
                            else if (it->second->getType() == T_NUMERIC) {
                                if (sscanf(allArgs[i].c_str(), "%llu", &var) != 1) {
                                    fprintf(stderr, "Cannot parse '%s' '%s', invalid input.\n",
                                            it->second->getCommandLineArgument().second.c_str(),
                                            it->second->getCommandLineArgument().first.c_str());
                                    success = false;
                                }
                                if (!it->second->validateNumericRange(var)) {
                                    success = false;
                                }
                                ((Parameter<unsigned long long>*)it->second)->setValue(var);
                            }
                            // Type is T_VECTOR_STR
                            else if (it->second->getType() == T_VECTOR_STR) {
                                if (NOSPLIT == ((ParameterVector<std::string>*)it->second)->getParseMethod()) {
                                    if (!it->second->validateStrRange(allArgs[i])) {
                                        success = false;
                                    }
                                    ((ParameterVector<std::string>*)it->second)->setValue(allArgs[i]);
                                }
                            }
                            // Type is T_VECTOR_NUMERIC
                            else if (it->second->getType() == T_VECTOR_NUMERIC) {
                                if (SPLIT == ((ParameterVector<unsigned long long>*)it->second)->getParseMethod()) {
                                    std::vector<std::string> v = split(allArgs[i]);
                                    for (unsigned int j = 0; j < v.size(); j++) {
                                        if (sscanf(v[j].c_str(), "%llu", &var) != 1) {
                                            fprintf(stderr, "Cannot parse '%s' '%s', invalid input.\n",
                                                    it->second->getCommandLineArgument().second.c_str(),
                                                    it->second->getCommandLineArgument().first.c_str());
                                            success = false;
                                        }
                                        if (!it->second->validateNumericRange(var)) {
                                            success = false;
                                        }
                                        ((ParameterVector<unsigned long long>*)it->second)->setValue(var);
                                    }
                                }
                            }
                        }
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
            for (it = parameterList.begin(); it != parameterList.end(); it++) {
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
            for (unsigned int i = 1; i < allArgs.size(); i++) {
                if (allArgs[i] == "-help" || allArgs[i] == "-h") {
                    std::cout << displayHelp() <<'\n';
                    return true;
                }
            }
            return false;
        }

        ~ParameterManager()
        {
            std::map<std::string, Parameter_base *>::iterator it;
            for (it = parameterList.begin(); it != parameterList.end(); it++) {
                delete it->second;
            }
            parameterList.clear();
        }

        // check if a variable has been set
        bool isSet(std::string parameterKey)
        {
            std::map<std::string, Parameter_base*>::iterator it;
            it = parameterList.find(parameterKey);
            if (it != parameterList.end()) {
                return parameterList[parameterKey]->getIsSet();
            } else {
                return false;
            }
        }


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

        std::vector<std::string> split(std::string str, char delimiter = ':')
        {
            std::vector<std::string> v;
            std::size_t current, previous = 0;
            current = str.find_first_of(delimiter);
            while (current != std::string::npos) {
                v.push_back(str.substr(previous, current - previous));
                previous = current + 1;
                current = str.find_first_of(delimiter, previous);
            }
            v.push_back(str.substr(previous, current - previous));
            return v;
        }

};

#endif // __PARAMETERMANAGER_H__