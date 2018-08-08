

/*
 * (c) 2005-2018  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */


#include "Parameter.h"


////////////////////////////////////////////////////////////////////////////
/* Implementation of middleware bit mask */
const MiddlewareMask Middleware::RTIDDSPRO = 1 << 0;
const MiddlewareMask Middleware::RTIDDSMICRO = 1 << 1;
const MiddlewareMask Middleware::RAWTRANSPORT = 1 << 2;

////////////////////////////////////////////////////////////////////////////
/* Implementation Class CommandLineArgument*/

CommandLineArgument::CommandLineArgument() {}

CommandLineArgument::CommandLineArgument(std::string option, std::string arg) :
        _option(option),
        _arg(arg)
{}

CommandLineArgument::~CommandLineArgument()
{
    _option.clear();
    _arg.clear();
}

void CommandLineArgument::set(const std::string option, const std::string arg)
{
    _option.assign(option);
    _arg.assign(arg);
}

const std::string CommandLineArgument::get_option()
{
    return _option;
}

const std::string CommandLineArgument::get_arg()
{
    return _arg;
}

////////////////////////////////////////////////////////////////////////////////
/* Implementation Class ParameterBase*/

ParameterBase::ParameterBase()
{
    _internal = false;
    _isSet = false;
    _type = T_NULL;
    _extraArgument = NO;
    _numericRange= std::make_pair(0, MAX_ULLONG);
    _group = GENERAL;
}

ParameterBase::~ParameterBase()
{
    _internal = false;
    _description.clear();
    _isSet = false;
    _type = T_NULL;
    _extraArgument = NO;
    _numericRange= std::make_pair(0, MAX_ULLONG);
    _validStrValues.clear();
}

// Validate range
bool ParameterBase::validate_numeric_range(unsigned long long var)
{
    if (_numericRange.first > var || var > _numericRange.second) {
        fprintf(stderr,
                "In the argument '%s', '%s' should be in the range [%llu,%llu]\n",
                _commandLineArgument.get_option().c_str(),
                _commandLineArgument.get_arg().c_str(),
                _numericRange.first,
                _numericRange.second);
        return false;
    } else {
        return true;
    }
}

bool ParameterBase::validate_str_range(std::string var)
{
    if (!_validStrValues.empty()) {
        if (_validStrValues.end() ==
                std::find(_validStrValues.begin(), _validStrValues.end(), var)) {
            fprintf(stderr, "In the argument '%s', incorrect '%s':  %s\n",
                    _commandLineArgument.get_option().c_str(),
                    _commandLineArgument.get_arg().c_str(),
                    var.c_str());
            return false;
        }
    }
    return true;
}

// Set members
void ParameterBase::set_command_line_argument(
        const std::string option,
        const std::string arg)
{
    _commandLineArgument.set(option, arg);
}

void ParameterBase::set_description(const std::string var)
{
    _description = var;
}

void ParameterBase::set_isSet(const bool var)
{
    _isSet = var;
}

void ParameterBase::set_type(const Type var)
{
    _type = var;
}

void ParameterBase::set_extra_argument(const ExtraArgument var)
{
    _extraArgument = var;
}

void ParameterBase::set_range(
        const unsigned long long rangeStart,
        const unsigned long long rangeEnd)
{
    _numericRange = std::make_pair(rangeStart, rangeEnd);
}

void ParameterBase::add_valid_str_value(const std::string validStrValue)
{
    _validStrValues.push_back(validStrValue);
}

void ParameterBase::set_internal(const bool var)
{
    _internal = var;
}

void ParameterBase::set_group(const Group var)
{
    _group = var;
}

void ParameterBase::set_supported_middleware(const MiddlewareMask var)
{
    _middlewareMask = var;
}

// Get members
const CommandLineArgument ParameterBase::get_command_line_argument()
{
    return _commandLineArgument;
}

const std::string ParameterBase::get_arg()
{
    return _commandLineArgument.get_arg();
}

const std::string ParameterBase::get_option()
{
    return _commandLineArgument.get_option();
}

const std::string ParameterBase::get_description()
{
    return _description;
}

const bool ParameterBase::get_isSet()
{
    return _isSet;
}

const Type ParameterBase::get_type()
{
    return _type;
}

const ExtraArgument ParameterBase::get_extra_argument()
{
    return _extraArgument;
}

const bool ParameterBase::get_internal()
{
    return _internal;
}

const Group ParameterBase::get_group()
{
    return _group;
}

const MiddlewareMask ParameterBase::get_supported_middleware()
{
    return _middlewareMask;
}

const ParseMethod ParameterBase::get_parse_method()
{
    return NO_SPLIT;
}

// Get the help message of a expecific parameter
std::string ParameterBase::print_command_line_parameter()
{
    const std::string spaces (42, ' ');
    std::string description = _description;
    std::size_t foundPosition = description.find("\n");
    while (foundPosition != std::string::npos) {
        description.insert(foundPosition + 1, spaces);
        foundPosition = description.find("\n", foundPosition + 1);
    }
    std::ostringstream oss;
    oss.fill(' ');
    oss.width(33);
    oss << std::left
        << std::string("\t") + get_option() + std::string(" ") + get_arg()
        << "- "
        << description
        << "\n";
    return oss.str();
}
