

/*
 * (c) 2005-2018  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */

#ifndef __PARAMETER_H__
#define __PARAMETER_H__

#include <stdio.h>
#include <climits>
#include <string>
#include <utility>      // std::pair, std::make_pair
#include <algorithm>    // std::sort
#include <vector>
#include <iostream>
#include <sstream>

#define MAX_ULLONG 18446744073709551615ULL

typedef unsigned int MiddlewareMask;

/*
 * This enum specifies the type of the parameter.
 * It is used to determine how to parse them.
 */
enum Type {
    T_NULL,            // Default type
    T_NUMERIC_LLU,     // Numeric type unsigned long long
    T_NUMERIC_LD,      // Numeric type long
    T_NUMERIC_D,       // Numeric type int
    T_STR,             // std::string pype
    T_BOOL,            // bool type
    T_VECTOR_NUMERIC,  // std::vector<unsigened long long>
    T_VECTOR_STR,      // std::vector<std::string>
    T_PAIR_NUMERIC_STR // std::pair<unsigened long long, std::string>
};

// This enum specifies the way how to parser an argument for a ParameterVector.
enum ParseMethod {
    NO_SPLIT,          // If the value is not splited into the vector
    SPLIT              // If the value is splited into the vector
};

// This enum specifies if a parameter will be followed by one extra argument.
enum ExtraArgument {
    NO,                // There is not extra argument
    POSSIBLE,          // It is possible to have one extra argument
    YES                // There is an extra argument
};

// This enum specifies the group of a parameter. It is used to sort the help message.
enum Group {
    GENERAL,
    PUB,
    SUB,
    TRANSPORT,
    SECURE,
    RAWTRANSPORT
};

// This struct specifies witch parameters are supported by each middleware

struct Middleware {
    static const MiddlewareMask RTIDDSPRO;
    static const MiddlewareMask RTIDDSMICRO;
    static const MiddlewareMask RAWTRANSPORT;
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
        void set(const std::string option, const std::string arg);
        const std::string get_option();
        const std::string get_arg();
};

class ParameterBase {
    private:
        CommandLineArgument _commandLineArgument;
        std::string _description;
        bool _isSet;
        Type _type;
        ExtraArgument  _extraArgument;
        bool _internal; // It will not be displayed to the customer
        Group _group;
        MiddlewareMask _middlewareMask;

        /*
         * Only used for numeric Parameter
         * The value is include in the range.
         * The default minimum is 0, because not negative paramter.
         */
        std::pair<unsigned long long, unsigned long long> _numericRange;

        // Only used for std::string Parameter
        std::vector<std::string> _validStrValues;

    public:
        ParameterBase();
        virtual ~ParameterBase();

        // Validate if the value is include in the range
        bool validate_numeric_range(unsigned long long var);

        /*
         * If validStrValues is not empty
        *      Validate if the value is include in the list of valid string
        */
        bool validate_str_range(std::string var);

        // Get the help message of a expecific parameter
        std::string print_command_line_parameter();

        // Set members
        void set_command_line_argument(
                const std::string option,
                const std::string arg);
        void set_description(const std::string var);
        void set_isSet(const bool var);
        void set_type(const Type var);
        void set_extra_argument(const ExtraArgument var);
        void set_internal(const bool var);
        void set_group(const Group var);
        void set_supported_middleware(const MiddlewareMask var);
        void set_range(
                const unsigned long long rangeStart,
                unsigned long long rangeEnd);
        void add_valid_str_value(const std::string validStrValue);
        virtual void set_parse_method(const ParseMethod var) {}

        // Get members
        const std::string get_arg();
        const std::string get_option();
        const CommandLineArgument get_command_line_argument();
        const std::string get_description();
        const bool get_isSet();
        const Type get_type();
        const ExtraArgument get_extra_argument();
        const bool get_internal();
        const Group get_group();
        const MiddlewareMask get_supported_middleware();
        const ParseMethod get_parse_method();
};

template <typename T>
class Parameter : public ParameterBase {
    private:
        T _value;

    public:
        Parameter() {}

        Parameter(T value) : _value(value) {}

        ~Parameter() {}

        Parameter(ParameterBase& p) {}

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
        ParseMethod _parseMethod;

    public:
        ParameterVector() : _parseMethod(NO_SPLIT) {}

        ParameterVector(T value) : _parseMethod(NO_SPLIT)
        {
            _value.clear();
            _value.push_back(value);
        }

        ParameterVector(std::vector<T> value)
        {
            _parseMethod = NO_SPLIT;
            _value.insert(_value.begin(), value.begin(), value.end());
        }

        ~ParameterVector()
        {
            _value.clear();
        }

        ParameterVector(ParameterBase& p) {}

        std::vector<T> get_value()
        {
            return _value;
        }

        /*
         * Add the value to the vector<T>:
         *     - If the isSet is false: clear the vector.
         *       It is used in order to clean the default values.
         *     - After add the element, sort them.
         *     - Set isSet to True
         */
        void set_value(T value)
        {
            if (!get_isSet()) {
                // In the case it is not set, remove default values
                _value.clear();
            }
            _value.push_back(value);
            std::sort(_value.begin(), _value.end());

            set_isSet(true);
        }

        void set_parse_method(const ParseMethod parseMethod)
        {
            _parseMethod = parseMethod;
        }

        const ParseMethod get_parse_method()
        {
            return _parseMethod;
        }
};



template <typename K, typename V>
class ParameterPair : public ParameterBase {
    private:
        std::pair<K, V> _value;

    public:
        ParameterPair() {}

        ParameterPair(K key, V val)
        {
            _value = std::make_pair(key, val);
        }

        ~ParameterPair() {}

        ParameterPair(ParameterBase& p) {}

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
        ParameterBase *_param;
    public:
        AnyParameter() :  _param(NULL) {}

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

        AnyParameter& operator=(AnyParameter &other)
        {
            _param = other._param;
            other._param = NULL;
            return *this;
        }

        ~AnyParameter()
        {
            if (_param != NULL) {
                delete _param;
                _param = NULL;
            }
        }
};

#endif // __PARAMETER_H__
