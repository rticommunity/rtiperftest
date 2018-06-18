#ifndef __PARAMETER_H__
#define __PARAMETER_H__

#include <stdio.h>
#include <climits>
#include <string>
#include <utility>      // std::pair, std::make_pair
#include <vector>

enum TYPE {t_NULL, t_int, t_string, t_bool, t_vector_string_push};


class Parameter_base  {
    private:
        std::pair <std::string, std::string> commandLineArgument;
        std::string description;
        bool isSet;
        TYPE type;
        int  numArguments; // This specifies if a parameter will be followed by another argument.
        std::string errorMessage;
        bool internal; // Does not have description

        // Only used for numeric argument
        unsigned long long rangeStart;
        unsigned long long rangeEnd;

        // Only used for std::string argument
        std::vector<std::string> validStrValues;

    public:
        Parameter_base()
        {
            internal = false;
            isSet = false;
            type = t_NULL;
            numArguments = 0;
            rangeStart = 0;
            rangeEnd = ULLONG_MAX;
        }
        virtual ~Parameter_base()
        {
            internal = false;
            description.clear();
            isSet = false;
            type = t_NULL;
            numArguments = 0;
            commandLineArgument.first.clear();
            commandLineArgument.second.clear();
            errorMessage.clear();
            rangeStart = 0;
            rangeEnd = 0;
            validStrValues.clear();
        }

        // Validate range
        bool validateNumericRange(unsigned long long var) {
            if (rangeEnd < var || rangeStart > var) {
                fprintf(stderr, "In the argument '%s', '%s' should be in the range [%llu, %llu]\n",
                        commandLineArgument.first.c_str(),
                        commandLineArgument.second.c_str(),
                        rangeStart,
                        rangeEnd);
                return false;
            } else {
                return true;
            }
        }

        // Validate str Values
        bool validateStrRange(std::string var) {
            if (!validStrValues.empty()) {
                bool validStr = false;
                for (unsigned int i = 0; i < validStrValues.size(); i++) {
                    if (var == validStrValues[i]) {
                        return true;
                    }
                }
                if (!validStr) {
                    fprintf(stderr, "In the argument '%s', incorrect '%s':  %s\n",
                        commandLineArgument.first.c_str(),
                        commandLineArgument.second.c_str(),
                        var.c_str());
                    return false;
                }
            }
            return true;
        }

        // Set members
        virtual void setCommandLineArgument(std::pair <std::string, std::string> var)
        {
            commandLineArgument = var;
        }

        virtual void setDescription(std::string var)
        {
            description = var;
        }

        virtual void setIsSet(bool var)
        {
            isSet = var;
        }

        virtual void setType(TYPE var)
        {
            type = var;
        }

        virtual void setNumArguments(int var)
        {
            numArguments = var;
        }

        virtual void setErrorMessage(std::string var)
        {
            errorMessage = var;
        }

        virtual void setRangeStart(unsigned long long var)
        {
            rangeStart = var;
        }

        virtual void setRangeEnd(unsigned long long var)
        {
            rangeEnd = var;
        }

        virtual void setRange(unsigned long long rangeStart, unsigned long long rangeEnd)
        {
            this->rangeStart = rangeStart;
            this->rangeEnd = rangeEnd;
        }

        virtual void addValidStrValues(std::string validStrValue)
        {
            this->validStrValues.push_back(validStrValue);
        }

        virtual void setInternal(bool var)
        {
            internal = var;
        }

        // Get members
        virtual std::string getErrorMessage()
        {
            return errorMessage;
        }

        virtual std::pair <std::string, std::string> getCommandLineArgument()
        {
            return commandLineArgument;
        }

        virtual std::string getDescription()
        {
            return description;
        }

        virtual bool getIsSet()
        {
            return isSet;
        }

        virtual TYPE getType()
        {
            return type;
        }

        virtual int getnumArguments()
        {
            return numArguments;
        }

        virtual bool getInternal()
        {
            return internal;
        }

        // Todo create one per type
        virtual void getValue(int &var) {}
        virtual void getValue(std::string &var) {}

        // Todo create one per type
        virtual void setValue(int var) {}
        virtual void setValue(std::string var) {}
};

template <typename T>
class Parameter : public Parameter_base {
    private:
        T value;

    public:
        Parameter()
        {
        }
        ~Parameter()
        {
        }
        Parameter(Parameter_base& p)
        {
        }

        T getValue()
        {
            T var;
            getValueInternal(var);
            return var;
        }

        void setValue(T var)
        {
            value = var;
        }

    private:
        void getValueInternal(T &var)
        {
            var = value;
        }
};


template <typename T>
class ParameterVector : public Parameter_base {
    private:
        std::vector<T> value;

    public:
        ParameterVector()
        {
        }
        ~ParameterVector()
        {
            value.clear();
        }
        ParameterVector(Parameter_base& p)
        {
        }

        std::vector<T> getValue()
        {
            std::vector<T> var;
            getValueInternal(var);
            return var;
        }

        void setValue(T var)
        {
            value.push_back(var);
        }

    private:
        void getValueInternal(std::vector<T> &var)
        {
            var = value;
        }
};


#endif // __PARAMETER_H__