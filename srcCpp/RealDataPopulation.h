#ifndef __REAL_DATA_POPULATION__
#define __REAL_DATA_POPULATION__

#include <iostream>
#include <vector>
#include <fstream>
#include "ParameterManager.h"
#include "perftest_cpp.h"
#include "Infrastructure_common.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

struct RealPayload {

    std::vector<char> _payload;
    bool _read;
    PerftestSemaphore *_payloadAccessSem;

    RealPayload() : _read(false){

         _payloadAccessSem = PerftestSemaphore_new();

        if (_payloadAccessSem == NULL) {
            fprintf(stderr, "Fail to create a Semaphore for RealPayload\n");
            //TODO: throw exception
        }
    }
};

class RealData {

    //const unsigned int MAXIMUN_REALDATA_ALLOCABLE = 1073741824; // 1Gb
    const unsigned int MAXIMUN_REALDATA_ALLOCABLE = 20000; //TODO: remove // test

    std::string _pathToData;

    /* Circular Queue */
    std::vector <RealPayload> _dataBuffer;
    unsigned int _payloadSize;
    unsigned int _head;
    unsigned int _tail;
    unsigned int _nPayloads;
    unsigned int _mFiles;
    bool _allDataFit;
    bool _initializationFinish;

    std::vector<std::string> _filesPath;

    bool _waitForData;

    ParameterManager *_PM;

    struct PerftestThread *_loadRealDataThread;

    /*
     * Increase any index in a circular way to always reference inside of
     * _dataBuffer vector.
     */
    unsigned int nextIndex(unsigned int &index){
        index = (index + 1) % _nPayloads;
        return index;
    }

    /*
     * Spawned Method for the asynchronous thread.
     * This method will load the files circularly and push them in to the queue
     * _dataBuffer if the files on the specified directory have reach the
     * maximun allocable memory defined, in other case, this thread will not be
     * longer necessary and will finish. The files will be read circularly
     */
    static void *loadDataAsynchronous (void *arg) {

        RealData *rd = static_cast<RealData *>(arg);

        std::vector<char> newPayload(rd->_payloadSize);

        /* Allocate as many entries on the queue as could be possible */
        rd->_dataBuffer.resize(rd->MAXIMUN_REALDATA_ALLOCABLE / rd->_payloadSize);
        int currentFileSize = 0;
        int currentBytesRead = 0;
        int totalBytesRead = 0;
        bool queueFull = false;
        bool allFilesRead = false;

        /* This while condition should not ever be fulfill */
        while(rd->_mFiles < rd->_filesPath.size()) {

            currentFileSize = getFileSize(rd->_filesPath[rd->_mFiles]);

            while ((unsigned int)(currentFileSize - currentBytesRead)
                   >= rd->_payloadSize) {
                currentBytesRead += readFile(
                        rd->_filesPath[rd->_mFiles],
                        newPayload,
                        rd->_payloadSize,
                        currentBytesRead);

                if (currentBytesRead == -1) {
                    fprintf(stderr,
                            "Error reading file %s\n",
                            rd->_filesPath[rd->_mFiles].c_str());
                    return NULL;
                }

                if (!queueFull
                        && (rd->_nPayloads == rd->_dataBuffer.size()) {
                    /* This code can only be reached one time */
                    queueFull = true;
                }

                if (queueFull || allFilesRead) {
                    /*
                     * Let the main thread know that we have fill the queue in
                     * order to start pulling data from the queue.
                     */
                    rd->_initializationFinish = true;

                    /*
                    * We do not increment the number of payloads if the queue is
                    * full, just override an existing one on the queue.
                    */
                    if (!rd->pushPayload(newPayload)) {
                        fprintf(stderr,
                                "Error pushing payload into the Queue %s\n",
                                rd->_filesPath[rd->_mFiles].c_str());
                        return NULL;
                    };
                } else {
                    /* The Queue is not full yet, so initialize the entries */
                    rd->_dataBuffer[rd->_nPayloads++]._payload.swap(newPayload);
                }
            }

            if (rd->_nPayloads == 0) {
                /*
                 * This condition should be only fulfill if -dataLen has been
                 * set and the size of the first file is lower than -dataLen.
                 * This means that we can not allocate any single payload for
                 * the requested size (-dataLen).
                 */
                fprintf(stderr,
                        "The file %s is to small to allocate the requested"
                        " size (%d bytes)\n",
                        rd->_filesPath[rd->_mFiles].c_str(),
                        rd->_payloadSize);
                return NULL;
            }

            /* Calculate the full amount of bytes read and reset counters */
            totalBytesRead += currentBytesRead;
            currentBytesRead = 0;

            if (!allFilesRead && rd->_mFiles + 1 == rd->_filesPath.size()) {
                /*
                 *-- This code can only be reached one time --
                 * If this code has been reach, all files has been read, this
                 * prevent increasing the size of the queue more that it's need.
                 * For now on the data will be pushed circularly.
                 */
                allFilesRead = true;
                /* Shrink the queue to the actual used size. */
                rd->_dataBuffer.resize(rd->_nPayloads);

                if (!queueFull) {
                    /*
                     * If all the files has been read but the queue is not full,
                     * this means that all the files fit on memory and this
                     * thread is not useful anymore
                     */
                    rd->_allDataFit = true;
                    return NULL;
                }
            }

            /* Reset the index and start over from the first file */
            rd->_mFiles = (rd->_mFiles + 1) % rd->_filesPath.size();
        }

        return NULL;

    }

public:
    RealData ()
            : _payloadSize(0),
            _head(0),
            _tail(0),
            _nPayloads(0),
            _mFiles(0),
            _allDataFit(false),
            _initializationFinish(false),
            _loadRealDataThread(NULL)
    {};

    RealData (ParameterManager *PM)
            : _payloadSize(0),
            _head(0),
            _tail(0),
            _nPayloads(0),
            _mFiles(0),
            _allDataFit(false),
            _initializationFinish(false),
            _PM(PM),
            _loadRealDataThread(NULL)
    {};

    RealData (std::string path, ParameterManager *PM)
            : _pathToData(path),
            _payloadSize(0),
            _head(0),
            _tail(0),
            _nPayloads(0),
            _mFiles(0),
            _allDataFit(false),
            _initializationFinish(false),
            _PM(PM),
            _loadRealDataThread(NULL)
    {
        if (!initialize(path,PM)){
            /* Trow Exception */
        }
    }

    // TODO: create destructor

    bool initialize(std::string path, ParameterManager *PM)
    {
        _pathToData = path;
        _PM = PM;

        struct stat s;
        int actualbytesRead = 0;
        int actualFileSize = 0;
        int totalBytesRead = 0;
        _payloadSize = 0;

        // TODO: change for ensureOrderReading
        _waitForData = _PM->get<unsigned long long>("waitForNewData");

        if (stat(path.c_str(), &s) == 0) {
            if (s.st_mode & S_IFDIR) {
                /* The path is a directory */
                _filesPath = getFilesPathFromDirectory(path);
                if (_filesPath.empty()) {
                    fprintf(stderr,
                            "Any regular file found on directory %s\n",
                            path.c_str());
                    return false;
                }
            } else if (s.st_mode & S_IFREG) {
                /* The path is a file */
                _filesPath.push_back(path);
            }
        } else {
            fprintf(stderr,
                    "Could not open directory or file %s\n",
                    path.c_str());
            return false;
        }

        if (_PM->is_set("dataLen")) {
            _payloadSize = _PM->get<unsigned long long>("dataLen");
        } else {
            /* Search for the lowest file size */
            unsigned int lowerSize = getFileSize(_filesPath[0]);
            unsigned int actualSize = lowerSize;
            for (unsigned int i = 1; i < _filesPath.size(); i++) {
                actualSize = getFileSize(_filesPath[i]);
                if (actualSize < lowerSize) {
                    lowerSize = actualSize;
                }
            }
            _payloadSize = actualSize;
            if (_payloadSize > MAXIMUN_REALDATA_ALLOCABLE) {
                fprintf(stderr,
                        "[WARNING] - The file/s %s is/are to big to be "
                        "allocated and sent them as one samples, set -dataLen "
                        "value to divide the file/s on samples of size of "
                        "-dataLen. The file/s are going to be truncated to %d "
                        "size\nCheck -realPayload documentation for more "
                        "information\n",
                        path.c_str(),
                        MAXIMUN_REALDATA_ALLOCABLE);
                _payloadSize = MAXIMUN_REALDATA_ALLOCABLE;
            }
            // TODO: Should we check the range of the valid size?
            _PM->set<unsigned long long>("dataLen", _payloadSize);
        }

        /*
         * Launch the asynchronous thread that will initialize the queue and
         * will keep writing on it if the real data is to big
         */
        _loadRealDataThread = PerftestThread_new(
                "loadingThread",
                Perftest_THREAD_PRIORITY_DEFAULT,
                RTI_OSAPI_THREAD_OPTION_STDIO,
                loadDataAsynchronous,
                this);

        /*
         * Wait for the _loadRealDataThread to signal that has fill the queue or
         * read all the data.
         */
        while (!_initializationFinish){};

        return true;
    }

    /* TODO: document properly that function unblock the previous payload */
    std::vector<char>& getNextPayload()
    {
        std::vector<char> *resultPayload;
        RTIOsapiSemaphoreStatus semStatus;
        struct RTINtpTime block_inf = RTI_NTP_TIME_MAX;
        struct RTINtpTime block_zero;
        RTINtpTime_packFromMillisec(block_zero, 0, 0);


        /* Release previous data */
        if (!PerftestSemaphore_give(
                _dataBuffer[_head]._payloadAccessSem)) {
            fprintf(stderr, "Unexpected error giving semaphore\n");
            return *resultPayload;
        }

        do{
            nextIndex(_head);
            semStatus = RTIOsapiSemaphore_take(
                _dataBuffer[_head]._payloadAccessSem,
                _waitForData? &block_inf : &block_zero);
        }while (semStatus == RTI_OSAPI_SEMAPHORE_STATUS_TIMEOUT);

        if (semStatus == RTI_OSAPI_SEMAPHORE_STATUS_OK) {
            if (_dataBuffer[_head]._read && _waitForData) {
                if (!PerftestSemaphore_give(
                        _dataBuffer[_head]._payloadAccessSem)) {
                    fprintf(stderr, "Unexpected error giving semaphore\n");
                    return *resultPayload;
                }
                while (_dataBuffer[_head]._read || _allDataFit){}
                RTIOsapiSemaphore_take(
                    _dataBuffer[_head]._payloadAccessSem,
                    &block_inf);
            }
            resultPayload = &_dataBuffer[_head]._payload;
            _dataBuffer[_head]._read = true;
        } else {
            fprintf(stderr, "Unexpected error taking semaphore\n");
            return *resultPayload;
        }
        return *resultPayload;
    }

    bool pushPayload(std::vector<char>& realPayload){

        bool payloadRead = false;
        struct RTINtpTime block_inf = RTI_NTP_TIME_MAX;

        if (RTIOsapiSemaphore_take(
                _dataBuffer[_tail]._payloadAccessSem, &block_inf)
                != RTI_OSAPI_SEMAPHORE_STATUS_OK) {
            fprintf(stderr, "Unexpected error taking semaphore\n");
            return false;
        }

        payloadRead = _dataBuffer[_tail]._read;

        if (payloadRead) {
            _dataBuffer[_tail]._payload.swap(realPayload);
        }

        if (!PerftestSemaphore_give(
                _dataBuffer[_tail]._payloadAccessSem)) {
            fprintf(stderr, "Unexpected error giving semaphore\n");
            return false;
        }

        if (!payloadRead) {
            while(!_dataBuffer[_tail]._read){}
            if (RTIOsapiSemaphore_take(
                    _dataBuffer[nextIndex(_tail)]._payloadAccessSem,
                    &block_inf)
                    != RTI_OSAPI_SEMAPHORE_STATUS_OK) {
                fprintf(stderr, "Unexpected error taking semaphore\n");
                return false;
            }
            _dataBuffer[_tail]._payload.swap(realPayload);
        }

        _dataBuffer[_tail]._read = false;

        if (!PerftestSemaphore_give(
                _dataBuffer[_tail]._payloadAccessSem)) {
            fprintf(stderr, "Unexpected error giving semaphore\n");
            return false;
        }

        nextIndex(_tail);

        return true;
    }

};

#endif