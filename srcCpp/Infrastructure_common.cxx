/*
 * (c) 2005-2019  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */

#include "Infrastructure_common.h"

void *PerftestTimer::waitAndExecute(void *scheduleInfo)
{
    ScheduleInfo *info = static_cast<ScheduleInfo *>(scheduleInfo);

    PerftestClock::milliSleep(info->timer * 1000u);

    if (info->handlerFunction != NULL) {
        info->handlerFunction();
    }

    return NULL;
}

PerftestTimer &PerftestTimer::getInstance()
{
    static PerftestTimer instance;
    return instance;
}

PerftestThread *PerftestTimer::setTimeout(PerftestTimer::ScheduleInfo &info)
{
    struct PerftestThread *timerThread = NULL;

    timerThread = PerftestThread_new(
            "timerThread",
            Perftest_THREAD_PRIORITY_DEFAULT,
            Perftest_THREAD_OPTION_DEFAULT,
            waitAndExecute,
            &info);

    return timerThread;
}

bool PerftestFileHandler::path_is_file(std::string const& path)
{

  #if defined(RTI_UNIX) || defined(RTI_WIN32)
    struct stat fileStat;
    if (stat(path.c_str(), &fileStat) == 0) {
        if (fileStat.st_mode & S_IFREG) {
            return true;
        } else {
            std::cerr << "[Error] path_is_file: \"" << path
                      << "\" is not a regular file" << std::endl;
            return false;
        }
    } else {
        std::cerr << "[Error] path_is_file: Could not open file: \"" << path
                  << "\"" << std::endl;
        return false;
    }
  #else
    std::cerr << "[Error] path_is_file: Function not implemented for this OS"
              << std::endl;
    return false;
  #endif
}

long PerftestFileHandler::get_file_size(std::string const& fileName)
{

  #if defined(RTI_UNIX) || defined(RTI_WIN32)
    std::ifstream ifs(fileName.c_str(), std::ios::binary | std::ios::ate);
    if (!ifs.good()) {
        std::cerr << "[Error] get_file_size: Could not open file: \""
                  << fileName
                  << "\""
                  << std::endl;
        return -1;
    }
    std::ifstream::pos_type pos = ifs.tellg();
    ifs.close();
    return pos;
  #else
    std::cerr << "[Error] get_file_size: Function not implemented for this OS"
              << std::endl;
    return -1;
  #endif
}

/* If the function can not read the file would return -1 */
long PerftestFileHandler::read_file(
        std::string const& fileName,
        char * outputData,
        unsigned int bytesToRead,
        unsigned int startPos)
{
  #if defined(RTI_UNIX) || defined(RTI_WIN32)
    std::ifstream ifs(fileName.c_str(), std::ios::binary | std::ios::ate);

    if (!ifs.good()) {
        std::cerr << "[Error] read_file: Could not open file: \"" << fileName
                  << "\""
                  << std::endl;
        return -1;
    }

    std::ifstream::pos_type pos = ifs.tellg();
    if (bytesToRead != 0 && bytesToRead < pos) {
        pos = bytesToRead;
    }
    else if (bytesToRead > pos) {
        std::cerr << "[Error] read_file: The file \""
                  << fileName
                  << "\" does not have the number of bytes requested ("
                  << bytesToRead
                  << ") from the start position "
                  << startPos
                  << "."
                  << std::endl;
        return -1;
    }

    /* By default startPos is 0 */
    ifs.seekg(startPos, std::ios::beg);
    ifs.read(outputData, pos);
    ifs.close();

    return pos;
  #else
    std::cerr << "[Error] read_file: Function not implemented for this OS"
              << std::endl;
    return -1;
  #endif
}

/*
 * Function used to check if a given string is a number
 */
bool is_number(const std::string &str)
{
    return !str.empty()
        && (str.find_first_not_of("[0123456789]") == std::string::npos);
}

/*
 * Function to divide a string into substrings given a delimiter char
 */
std::vector<std::string> split(const std::string& str, char delim)
{
    std::vector<std::string> list;
    int pos = str.find(delim);
    int i = 0;
    while (pos != std::string::npos) {
        list.push_back(str.substr(i, pos - i));
        i = ++pos;
        pos = str.find(delim, pos);
    }
    list.push_back(str.substr(i, str.length()));
    return list;
}

/*
 * Function to check if a given string is an ip (it does not check for ranges)
 */
bool is_ip_address(std::string ip_string)
{
    std::vector<std::string> list = split(ip_string, '.');

    if (list.size() != 4) {
        return false;
    }

    for (int i = 0; i < list.size(); i++) {
        std::string part = list[i];
        if (!is_number(part) || stoi(part) > 255 || stoi(part) < 0) {
            return false;
        }
    }
    return true;
}
