/*
 * (c) 2005-2018  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */

#include "Parameter.h"

Parameter_base::Parameter_base()
{
    internal = false;
    isSet = false;
    type = T_NULL;
    extraArgument = NO;
    rangeStart = 0;
    rangeEnd = ULLONG_MAX;
}

Parameter_base::~Parameter_base()
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
bool Parameter_base::validateNumericRange(unsigned long long var)
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
bool Parameter_base::validateStrRange(std::string var)
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
void Parameter_base::setCommandLineArgument(std::pair <std::string, std::string> var)
{
    commandLineArgument = var;
}

void Parameter_base::setDescription(std::string var)
{
    description = var;
}

void Parameter_base::setIsSet(bool var)
{
    isSet = var;
}

void Parameter_base::setType(TYPE var)
{
    type = var;
}

void Parameter_base::setExtraArgument(EXTRAARGUMENT var)
{
    extraArgument = var;
}

void Parameter_base::setRangeStart(unsigned long long var)
{
    rangeStart = var;
}

void Parameter_base::setRangeEnd(unsigned long long var)
{
    rangeEnd = var;
}

void Parameter_base::setRange(unsigned long long rangeStart, unsigned long long rangeEnd)
{
    this->rangeStart = rangeStart;
    this->rangeEnd = rangeEnd;
}

void Parameter_base::addValidStrValue(std::string validStrValue)
{
    this->validStrValues.push_back(validStrValue);
}

void Parameter_base::setInternal(bool var)
{
    internal = var;
}

// Get members
std::pair <std::string, std::string> Parameter_base::getCommandLineArgument()
{
    return commandLineArgument;
}

std::string Parameter_base::getDescription()
{
    return description;
}

bool Parameter_base::getIsSet()
{
    return isSet;
}

TYPE Parameter_base::getType()
{
    return type;
}

EXTRAARGUMENT Parameter_base::getExtraArgument()
{
    return extraArgument;
}

bool Parameter_base::getInternal()
{
    return internal;
}

PARSEMETHOD Parameter_base::getParseMethod()
{
    return NOSPLIT;
}

// Get the help message
std::string Parameter_base::printCommandLineParameter()
{
    const std::string spaces (42, ' ');
    std::string description = getDescription();
    std::size_t foundPosition = description.find("\n");
    while (foundPosition != std::string::npos) {
        description.insert(foundPosition + 1, spaces);
        foundPosition = description.find("\n", foundPosition + 1);
    }
    std::ostringstream oss;
    oss << "\t"
        <<  getCommandLineArgument().first
        << " "
        << getCommandLineArgument().second
        << std::string(32 - (getCommandLineArgument().first.size() + getCommandLineArgument().second.size()), ' ' )
        << "- "
        << description
        << "\n";
    return oss.str();
}