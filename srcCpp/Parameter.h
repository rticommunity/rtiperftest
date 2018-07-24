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

/*
 * This specifies the the type of the parameter.
 * It is used in the way to parse them.
 */
enum TYPE {
    T_NULL,           // Default type
    T_NUMERIC,        // Numeric type, unsigned long long
    T_STR,            // std::string tpy
    T_BOOL,           // bool type
    T_VECTOR_NUMERIC, // std::vector<unsigened long long>
    T_VECTOR_STR,      // std::vector<std::string>
    T_PAIR_NUMERIC_STR // std::pair<unsigened long long, std::string>
};

// This specifies the way how to parser a argument for a ParameterVector.
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

// This specifies the group of a parameter. It is used to sort the help.
enum GROUP {
    GENERAL,
    PUB,
    SUB,
    TRANSPORT,
    SECURE,
    RAWTRANSPORT
};

class CommandLineArgument {
    private:
        std::string _option;
        std::string _arg;

    public:
        CommandLineArgument();
        CommandLineArgument(std::string option, std::string arg);
        ~CommandLineArgument();

        // Set and Get members
        void set(std::string option, std::string arg);
        std::string get_option();
        std::string get_arg();
};

class ParameterBase  {
    private:
        CommandLineArgument commandLineArgument;
        std::string description;
        bool isSet;
        TYPE type;
        EXTRAARGUMENT  extraArgument;
        bool internal; // Does not have description
        GROUP group;

        /*
         * Only used for numeric argument
         * The range are inlcuded.
         */
        unsigned long long rangeStart;
        unsigned long long rangeEnd;

        // Only used for std::string argument
        std::vector<std::string> validStrValues;

    public:
        ParameterBase();
        virtual ~ParameterBase();

        // Validate if the var is include in the range
        bool validate_numeric_range(unsigned long long var);

        /*
         * If validStrValues is not empty
        *      Validat if var is include in the list of valid string
        */
        bool validate_str_range(std::string var);

        std::string print_command_line_parameter();

        // Set members
        virtual void set_command_line_argument(const std::string option, const std::string arg);
        virtual void set_description(const std::string var);
        virtual void set_isSet(const bool var);
        virtual void set_type(const TYPE var);
        virtual void set_extra_argument(const EXTRAARGUMENT var);
        virtual void set_internal(const bool var);
        virtual void set_group(const GROUP var);
        virtual void set_range_start(const unsigned long long var);
        virtual void set_range_end(const unsigned long long var);
        virtual void set_range(const unsigned long long rangeStart, unsigned long long rangeEnd);
        virtual void add_valid_str_value(const std::string validStrValue);
        virtual void set_parse_method(const PARSEMETHOD var) {}

        // Get members
        virtual const std::string get_arg();
        virtual const std::string get_option();
        virtual const CommandLineArgument get_command_line_argument();
        virtual const std::string get_description();
        virtual const bool get_isSet();
        virtual const TYPE get_type();
        virtual const EXTRAARGUMENT get_extra_argument();
        virtual const bool get_internal();
        virtual const GROUP get_group();
        virtual const PARSEMETHOD get_parse_method();
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

        T get_value()
        {
            return value;
        }

        void set_value(T var)
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

        std::vector<T> get_value()
        {
            return value;
        }

        void set_value(T var)
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

        void set_parse_method(const PARSEMETHOD var)
        {
            parseMethod = var;
        }

        const PARSEMETHOD get_parse_method()
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

        std::pair<K,V> get_value()
        {
            return value;
        }

        void set_value(K key, V val)
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
        ParameterVector<T>* get_vector()
        {
            return static_cast<ParameterVector<T>*>(param);
        }

        template <typename K, typename V>
        ParameterPair<K,V>* get_pair()
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
