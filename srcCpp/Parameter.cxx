/*
 * (c) 2005-2018  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */

#include "Parameter.h"

ParameterBase::ParameterBase()
{
    internal = false;
    isSet = false;
    type = T_NULL;
    extraArgument = NO;
    rangeStart = 0;
    rangeEnd = ULLONG_MAX;
    group = GENERAL;
}

ParameterBase::~ParameterBase()
{
    internal = false;
    description.clear();
    isSet = false;
    type = T_NULL;
    extraArgument = NO;
    commandLineArgument.first.clear();
    commandLineArgument.second.clear();
    rangeStart = 0;
    rangeEnd = 0;
    validStrValues.clear();
}

// Validate range
bool ParameterBase::validate_numeric_range(unsigned long long var)
{
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

// Validate str Valuesi if not empty
bool ParameterBase::validate_str_range(std::string var)
{
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
void ParameterBase::set_command_line_argument(std::pair<std::string, std::string> var)
{
    commandLineArgument = var;
}

void ParameterBase::set_description(std::string var)
{
    description = var;
}

void ParameterBase::set_isSet(bool var)
{
    isSet = var;
}

void ParameterBase::set_type(TYPE var)
{
    type = var;
}

void ParameterBase::set_extra_argument(EXTRAARGUMENT var)
{
    extraArgument = var;
}

void ParameterBase::set_range_start(unsigned long long var)
{
    rangeStart = var;
}

void ParameterBase::set_range_end(unsigned long long var)
{
    rangeEnd = var;
}

void ParameterBase::set_range(unsigned long long rangeStart, unsigned long long rangeEnd)
{
    this->rangeStart = rangeStart;
    this->rangeEnd = rangeEnd;
}

void ParameterBase::add_valid_str_value(std::string validStrValue)
{
    this->validStrValues.push_back(validStrValue);
}

void ParameterBase::set_internal(bool var)
{
    internal = var;
}

void ParameterBase::set_group(GROUP var)
{
    group = var;
}

// Get members
std::pair <std::string, std::string> ParameterBase::get_command_line_argument()
{
    return commandLineArgument;
}

std::string ParameterBase::get_description()
{
    return description;
}

bool ParameterBase::get_isSet()
{
    return isSet;
}

TYPE ParameterBase::get_type()
{
    return type;
}

EXTRAARGUMENT ParameterBase::get_extra_argument()
{
    return extraArgument;
}

bool ParameterBase::get_internal()
{
    return internal;
}

GROUP ParameterBase::get_group()
{
    return group;
}

PARSEMETHOD ParameterBase::get_parse_method()
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
        + get_command_line_argument().first
        + std::string(" ")
        + get_command_line_argument().second;
    oss << "- "
        << description
        << "\n";
    return oss.str();
}
