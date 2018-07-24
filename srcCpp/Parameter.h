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
        CommandLineArgument _commandLineArgument;
        std::string _description;
        bool _isSet;
        TYPE _type;
        EXTRAARGUMENT  _extraArgument;
        bool _internal; // Does not have description
        GROUP _group;

        /*
         * Only used for numeric Parameter
         * The range are inlcuded.
         */
        unsigned long long _rangeStart;
        unsigned long long _rangeEnd;

        // Only used for std::string Parameter
        std::vector<std::string> _validStrValues;

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
        T _value;

    public:
        Parameter()
        {
        }

        Parameter(T value)
        {
            _value = value;
        }
        ~Parameter()
        {
        }
        Parameter(ParameterBase& p)
        {
        }

        T get_value()
        {
            return _value;
        }

        void set_value(T value)
        {
            _value = value;
            set_isSet(true);
        }
};

template <typename T>
class ParameterVector : public ParameterBase {
    private:
        std::vector<T> _value;
        PARSEMETHOD _parseMethod;

    public:
        ParameterVector()
        {
            _parseMethod = NOSPLIT;
        }

        ParameterVector(T value)
        {
            _parseMethod = NOSPLIT;
            _value.clear();
            _value.push_back(value);
        }

        ParameterVector(std::vector<T> value)
        {
            _parseMethod = NOSPLIT;
            _value.insert(_value.begin(), value.begin(), value.end());
        }

        ~ParameterVector()
        {
            _value.clear();
        }

        ParameterVector(ParameterBase& p)
        {
        }

        std::vector<T> get_value()
        {
            return _value;
        }

        void set_value(T value)
        {
            if (!get_isSet()) {
                // In the case of is not set, remove default values.
                _value.clear();
            }
            _value.push_back(value);

            if (get_type() == T_VECTOR_NUMERIC) {
                std::sort(_value.begin(), _value.end());
            }
            set_isSet(true);
        }

        void set_parse_method(const PARSEMETHOD parseMethod)
        {
            _parseMethod = parseMethod;
        }

        const PARSEMETHOD get_parse_method()
        {
            return _parseMethod;
        }
};



template <typename K, typename V>
class ParameterPair : public ParameterBase {
    private:
        std::pair <K, V> _value;

    public:
        ParameterPair()
        {
        }

        ParameterPair(K key, V val)
        {
            _value = std::make_pair(key, val);
        }

        ~ParameterPair()
        {
        }

        ParameterPair(ParameterBase& p)
        {
        }

        std::pair<K,V> get_value()
        {
            return _value;
        }

        void set_value(K key, V val)
        {

            _value = std::make_pair(key, val);
            set_isSet(true);
        }
};

class AnyParameter {
    private:
        ParameterBase* _param;
    public:
        AnyParameter()
        {
            _param = NULL;
        }

        template <typename T>
        AnyParameter(Parameter<T> *var) : _param(var) {}

        template <typename T>
        AnyParameter(ParameterVector<T> *var) : _param(var) {}

        template <typename K, typename V>
        AnyParameter(ParameterPair<K, V> *var) : _param(var) {}

        ParameterBase* get()
        {
            return static_cast<ParameterBase*>(_param);
        }

        template <typename T>
        Parameter<T>* get()
        {
            return static_cast<Parameter<T>*>(_param);
        }

        template <typename T>
        ParameterVector<T>* get_vector()
        {
            return static_cast<ParameterVector<T>*>(_param);
        }

        template <typename K, typename V>
        ParameterPair<K, V>* get_pair()
        {
            return static_cast<ParameterPair<K, V>*>(_param);
        }

        AnyParameter& operator=(AnyParameter other)
        {
            _param = other._param;
            other._param = NULL;
            return *this;
        };

        ~AnyParameter()
        {
            if (_param != NULL) {
                delete _param;
                _param = NULL;
            }
        }
};

#endif // __PARAMETER_H__
