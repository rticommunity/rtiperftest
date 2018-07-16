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
#include <algorithm>    // std::sort
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
        bool validate_numeric_range(unsigned long long var);

        // Validate str Valuesi if not empty
        bool validate_str_range(std::string var);

        std::string print_command_line_parameter();

        // Set members
        virtual void set_command_line_argument(std::pair <std::string, std::string> var);
        virtual void set_description(std::string var);
        virtual void set_isSet(bool var);
        virtual void set_type(TYPE var);
        virtual void set_extra_argument(EXTRAARGUMENT var);
        virtual void set_internal(bool var);
        virtual void set_group(GROUP var);
        virtual void set_range_start(unsigned long long var);
        virtual void set_range_end(unsigned long long var);
        virtual void set_range(unsigned long long rangeStart, unsigned long long rangeEnd);
        virtual void add_valid_str_value(std::string validStrValue);
        virtual void set_parse_method(PARSEMETHOD var) {}

        // Get members
        virtual std::pair <std::string, std::string> get_command_line_argument();
        virtual std::string get_description();
        virtual bool get_isSet();
        virtual TYPE get_type();
        virtual EXTRAARGUMENT get_extra_argument();
        virtual bool get_internal();
        virtual GROUP get_group();
        virtual PARSEMETHOD get_parse_method();
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
            set_isSet(true);
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
            if (!get_isSet()) {
                // In the case of is not set, remove default values.
                value.clear();
            }
            value.push_back(var);

            if (get_type() == T_VECTOR_NUMERIC) {
                std::sort(value.begin(), value.end());
            }
            set_isSet(true);
        }

        void set_parse_method(PARSEMETHOD var)
        {
            parseMethod = var;
        }

        PARSEMETHOD get_parse_method()
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
            set_isSet(true);
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
