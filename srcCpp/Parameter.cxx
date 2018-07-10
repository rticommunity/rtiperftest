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
bool ParameterBase::validateNumericRange(unsigned long long var)
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
bool ParameterBase::validateStrRange(std::string var)
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
void ParameterBase::setCommandLineArgument(std::pair<std::string, std::string> var)
{
    commandLineArgument = var;
}

void ParameterBase::setDescription(std::string var)
{
    description = var;
}

void ParameterBase::setIsSet(bool var)
{
    isSet = var;
}

void ParameterBase::setType(TYPE var)
{
    type = var;
}

void ParameterBase::setExtraArgument(EXTRAARGUMENT var)
{
    extraArgument = var;
}

void ParameterBase::setRangeStart(unsigned long long var)
{
    rangeStart = var;
}

void ParameterBase::setRangeEnd(unsigned long long var)
{
    rangeEnd = var;
}

void ParameterBase::setRange(unsigned long long rangeStart, unsigned long long rangeEnd)
{
    this->rangeStart = rangeStart;
    this->rangeEnd = rangeEnd;
}

void ParameterBase::addValidStrValue(std::string validStrValue)
{
    this->validStrValues.push_back(validStrValue);
}

void ParameterBase::setInternal(bool var)
{
    internal = var;
}

void ParameterBase::setGroup(GROUP var)
{
    group = var;
}

// Get members
std::pair <std::string, std::string> ParameterBase::getCommandLineArgument()
{
    return commandLineArgument;
}

std::string ParameterBase::getDescription()
{
    return description;
}

bool ParameterBase::getIsSet()
{
    return isSet;
}

TYPE ParameterBase::getType()
{
    return type;
}

EXTRAARGUMENT ParameterBase::getExtraArgument()
{
    return extraArgument;
}

bool ParameterBase::getInternal()
{
    return internal;
}

GROUP ParameterBase::getGroup()
{
    return group;
}

PARSEMETHOD ParameterBase::getParseMethod()
{
    return NOSPLIT;
}

// Get the help message
std::string ParameterBase::print_command_line_parameter()
{
    const std::string spaces (42, ' ');
    std::string description = getDescription();
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
        + getCommandLineArgument().first
        + std::string(" ")
        + getCommandLineArgument().second;
    oss << "- "
        << description
        << "\n";
    return oss.str();
}
