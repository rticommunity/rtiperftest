/*
 * (c) 2005-2018  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */

#include "Parameter.h"

/* Implementation Class CommandLineArgument*/
CommandLineArgument::CommandLineArgument()
{
}

CommandLineArgument::CommandLineArgument(std::string option, std::string arg) :
        _option(option),
        _arg(arg)
{}

CommandLineArgument::~CommandLineArgument()
{
    _option.clear();
    _arg.clear();
}

void CommandLineArgument::set(std::string option, std::string arg)
{
    _option.assign(option);
    _arg.assign(arg);
}

std::string CommandLineArgument::get_option()
{
    return _option;
}

std::string CommandLineArgument::get_arg()
{
    return _arg;
}

/* Implementation Class ParameterBase*/
ParameterBase::ParameterBase()
{
    _internal = false;
    _isSet = false;
    _type = T_NULL;
    _extraArgument = NO;
    _rangeStart = 0;
    _rangeEnd = ULLONG_MAX;
    _group = GENERAL;
}

ParameterBase::~ParameterBase()
{
    _internal = false;
    _description.clear();
    _isSet = false;
    _type = T_NULL;
    _extraArgument = NO;
    _rangeStart = 0;
    _rangeEnd = 0;
    _validStrValues.clear();
}

// Validate range
bool ParameterBase::validate_numeric_range(unsigned long long var)
{
    if (_rangeEnd < var || _rangeStart > var) {
        fprintf(stderr, "In the argument '%s', '%s' should be in the range [%llu, %llu]\n",
                _commandLineArgument.get_option().c_str(),
                _commandLineArgument.get_arg().c_str(),
                _rangeStart,
                _rangeEnd);
        return false;
    } else {
        return true;
    }
}

// Validate str Valuesi if not empty
bool ParameterBase::validate_str_range(std::string var)
{
    if (!_validStrValues.empty()) {
        if (_validStrValues.end() == std::find(_validStrValues.begin(), _validStrValues.end(), var)) {
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
void ParameterBase::set_command_line_argument(const std::string option, std::string arg)
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

void ParameterBase::set_type(const TYPE var)
{
    _type = var;
}

void ParameterBase::set_extra_argument(const EXTRAARGUMENT var)
{
    _extraArgument = var;
}

void ParameterBase::set_range_start(const unsigned long long var)
{
    _rangeStart = var;
}

void ParameterBase::set_range_end(const unsigned long long var)
{
    _rangeEnd = var;
}

void ParameterBase::set_range(const unsigned long long rangeStart, const unsigned long long rangeEnd)
{
    _rangeStart = rangeStart;
    _rangeEnd = rangeEnd;
}

void ParameterBase::add_valid_str_value(const std::string validStrValue)
{
    _validStrValues.push_back(validStrValue);
}

void ParameterBase::set_internal(const bool var)
{
    _internal = var;
}

void ParameterBase::set_group(const GROUP var)
{
    _group = var;
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

const TYPE ParameterBase::get_type()
{
    return _type;
}

const EXTRAARGUMENT ParameterBase::get_extra_argument()
{
    return _extraArgument;
}

const bool ParameterBase::get_internal()
{
    return _internal;
}

const GROUP ParameterBase::get_group()
{
    return _group;
}

const PARSEMETHOD ParameterBase::get_parse_method()
{
    return NOSPLIT;
}

// Get the help message
std::string ParameterBase::print_command_line_parameter()
{
    const std::string spaces (42, ' ');
    std::string description = get_description();
    std::size_t foundPosition = description.find("\n");
    while (foundPosition != std::string::npos) {
        description.insert(foundPosition + 1, spaces);
        foundPosition = description.find("\n", foundPosition + 1);
    }
    std::ostringstream oss;
    oss.fill(' ');
    oss.width(33);
    oss << std::left
        << std::string("\t")
        + get_option()
        + std::string(" ")
        + get_arg();
    oss << "- "
        << description
        << "\n";
    return oss.str();
}
