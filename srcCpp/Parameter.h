#ifndef __PARAMETER_H__
#define __PARAMETER_H__

/*
 * (c) 2005-2018  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */


#include <stdio.h>
#include <climits>
#include <string>
#include <utility>      // std::pair, std::make_pair
#include <vector>
#include <iostream>
#include <sstream>

enum TYPE {
    T_NULL,           // Default type
    T_NUMERIC,        // Numeric type, unsigned long long
    T_STR,            // std::string tpy
    T_BOOL,           // bool type
    T_VECTOR_NUMERIC, // std::vector<unsigened long long>
    T_VECTOR_STR,      // std::vector<std::string>
    T_PAIR_NUMERIC_STR // std::pair<unsigened long long, std::string>
};

enum PARSEMETHOD {
    NOSPLIT,          // If the value is not splited into the vector
    SPLIT             // If the value is splited into the vector
};

// This specifies if a parameter will be followed by one extra argument.
enum EXTRAARGUMENT {
    NO,               // There is not extra argument
    OPTIONAL,         // It is possible to have one extra argument
    YES              // There is not one extra argument
};

enum GROUP {
    GENERAL,
    PUB,
    SUB,
    TRANSPORT,
    SECURE,
    RAWTRANSPORT
};

class ParameterBase  {
    private:
        std::pair <std::string, std::string> commandLineArgument;
        std::string description;
        bool isSet;
        TYPE type;
        EXTRAARGUMENT  extraArgument;
        bool internal; // Does not have description
        GROUP group;

        // Only used for numeric argument
        unsigned long long rangeStart;
        unsigned long long rangeEnd;

        // Only used for std::string argument
        std::vector<std::string> validStrValues;

    public:
        ParameterBase();
        virtual ~ParameterBase();

        // Validate range
        bool validateNumericRange(unsigned long long var);

        // Validate str Valuesi if not empty
        bool validateStrRange(std::string var);

        std::string print_command_line_parameter();

        // Set members
        virtual void setCommandLineArgument(std::pair <std::string, std::string> var);
        virtual void setDescription(std::string var);
        virtual void setIsSet(bool var);
        virtual void setType(TYPE var);
        virtual void setExtraArgument(EXTRAARGUMENT var);
        virtual void setInternal(bool var);
        virtual void setGroup(GROUP var);
        virtual void setRangeStart(unsigned long long var);
        virtual void setRangeEnd(unsigned long long var);
        virtual void setRange(unsigned long long rangeStart, unsigned long long rangeEnd);
        virtual void addValidStrValue(std::string validStrValue);
        virtual void setParseMethod(PARSEMETHOD var) {}

        // Get members
        virtual std::pair <std::string, std::string> getCommandLineArgument();
        virtual std::string getDescription();
        virtual bool getIsSet();
        virtual TYPE getType();
        virtual EXTRAARGUMENT getExtraArgument();
        virtual bool getInternal();
        virtual GROUP getGroup();
        virtual PARSEMETHOD getParseMethod();
};

template <typename T>
class Parameter : public ParameterBase {
    private:
        T value;

    public:
        Parameter()
        {
        }

        Parameter(T var)
        {
            value = var;
        }
        ~Parameter()
        {
        }
        Parameter(ParameterBase& p)
        {
        }

        T getValue()
        {
            return value;
        }

        void setValue(T var)
        {
            value = var;
            setIsSet(true);
        }
};

template <typename T>
class ParameterVector : public ParameterBase {
    private:
        std::vector<T> value;
        PARSEMETHOD parseMethod;

    public:
        ParameterVector()
        {
            parseMethod = NOSPLIT;
        }
        ParameterVector(T var)
        {
            parseMethod = NOSPLIT;
            value.clear();
            value.push_back(var);
        }
        ParameterVector(std::vector<T> var)
        {
            parseMethod = NOSPLIT;
            value.insert(value.begin(),var.begin(), var.end());
        }
        ~ParameterVector()
        {
            value.clear();
        }
        ParameterVector(ParameterBase& p)
        {
        }

        std::vector<T> getValue()
        {
            return value;
        }

        void setValue(T var)
        {
            if (!getIsSet()) {
                // In the case of is not set, remove default values.
                value.clear();
            }
            value.push_back(var);
            setIsSet(true);
        }

        void setParseMethod(PARSEMETHOD var)
        {
            parseMethod = var;
        }

        PARSEMETHOD getParseMethod()
        {
            return parseMethod;
        }
};



template <typename K, typename V>
class ParameterPair : public ParameterBase {
    private:
        std::pair <K, V> value;

    public:
        ParameterPair()
        {
        }
        ParameterPair(K key, V val)
        {
            value = std::make_pair(key, val);
        }
        ~ParameterPair()
        {
        }
        ParameterPair(ParameterBase& p)
        {
        }

        std::pair<K,V> getValue()
        {
            return value;
        }

        void setValue(K key, V val)
        {

            value = std::make_pair(key, val);
            setIsSet(true);
        }
};

class AnyParameter {
    private:
        ParameterBase* param;
    public:
        AnyParameter()
        {
            param = NULL;
        }

        template <typename T>
        AnyParameter(Parameter<T> *var) : param(var) {}

        template <typename T>
        AnyParameter(ParameterVector<T> *var) : param(var) {}

        template <typename K, typename V>
        AnyParameter(ParameterPair<K,V> *var) : param(var) {}

        ParameterBase* get()
        {
            return static_cast<ParameterBase*>(param);
        }

        template <typename T>
        Parameter<T>* get()
        {
            return static_cast<Parameter<T>*>(param);
        }

        template <typename T>
        ParameterVector<T>* getVector()
        {
            return static_cast<ParameterVector<T>*>(param);
        }

        template <typename K, typename V>
        ParameterPair<K,V>* getPair()
        {
            return static_cast<ParameterPair<K,V>*>(param);
        }

        AnyParameter& operator=(AnyParameter other)
        {
            param = other.param;
            other.param = NULL;
            return *this;
        };

        ~AnyParameter()
        {
            if (param != NULL) {
                delete param;
                param = NULL;
            }
        }
};

#endif // __PARAMETER_H__
