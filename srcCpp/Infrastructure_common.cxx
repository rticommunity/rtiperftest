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

int getFileSize(std::string const& filename)
{
    std::ifstream ifs(filename.c_str(), std::ios::binary | std::ios::ate);
    if (!ifs.good()) {
        fprintf(stderr, "Could not open file %s\n", filename.c_str());
        return -1;
    }
    std::ifstream::pos_type pos = ifs.tellg();
    ifs.close();
    return pos;
}

/* If the function can not read the file would return -1 */
int readFile(
        std::string const& filename,
        std::vector<char>& fileData,
        unsigned int bytesToRead, /* File size */
        unsigned int startPos)
{
    std::ifstream ifs(filename.c_str(), std::ios::binary | std::ios::ate);

    if (!ifs.good()) {
        fprintf(stderr, "Could not open file %s\n", filename.c_str());
        return -1;
    }

    std::ifstream::pos_type pos = ifs.tellg();
    if (bytesToRead != 0 && bytesToRead < pos) {
        pos = bytesToRead;
    }
    else if (bytesToRead > pos) {
        fprintf(
                stderr,
                "The file %s does not have the number of bytes requested (%d) "
                "from the start possition %d\n",
                filename.c_str(),
                bytesToRead,
                startPos);
        return -1;
    }

    fileData.resize(pos);

    /* By default startPos is 0 */
    ifs.seekg(startPos, std::ios::beg);
    ifs.read(&fileData[0], pos);
    ifs.close();

    return pos;
}

std::vector<std::string> getFilesPathFromDirectory(const std::string &dirPath)
{
    DIR *dir;
    struct dirent *ent;
    std::vector<std::string> files;

    dir = opendir(dirPath.c_str());

    if (dir == NULL) {
        fprintf(stderr, "Could not open directory %s\n", dirPath.c_str());
        return files;
    }

    /* Iterate through all the files on the directory */
    while ((ent = readdir (dir)) != NULL) {
        if (ent->d_type ==  DT_REG /* | DT_FIFO*/) {
            files.push_back(dirPath + std::string(ent->d_name));
        }
    }

    closedir(dir);
    return files;
}
