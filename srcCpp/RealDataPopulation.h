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
    //const unsigned int MAXIMUN_REALDATA_ALLOCABLE = 20000; //TODO: remove // test

    std::string _pathToData;

    /* Circular Queue */
    std::vector <RealPayload> _dataBuffer;
    unsigned int _payloadSize;
    unsigned int _head;
    unsigned int _tail;
    unsigned int _maxFiles;
    unsigned int _nPayloads;
    unsigned int _mFiles;
    unsigned int _bytesReadFromLastFile;

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
        int actualFileSize = 0;
        int actualbytesRead = rd->_bytesReadFromLastFile;

        while(rd->_mFiles < rd->_filesPath.size()) {

            actualFileSize = getFileSize(rd->_filesPath[rd->_mFiles]);

            while ((actualFileSize - actualbytesRead) >= (int)rd->_payloadSize) {

                actualbytesRead += readFile(
                        rd->_filesPath[rd->_mFiles],
                        newPayload,
                        rd->_payloadSize,
                        actualbytesRead);

                if (actualbytesRead == -1) {
                    fprintf(stderr,
                            "Error reading file %s\n",
                            rd->_filesPath[rd->_mFiles].c_str());
                    return NULL;
                }
                /*
                 * We do not increment the number of payloads, this thread just
                 * override an existing one on the queue.
                 */

                if (!rd->pushPayload(newPayload)){
                    fprintf(stderr,
                        "Error pushing payload into the Queue %s\n",
                        rd->_filesPath[rd->_mFiles].c_str());
                    return NULL;
                };
            }
            actualbytesRead = 0;
            rd->_mFiles = (rd->_mFiles + 1) % rd->_filesPath.size();
        }

        return NULL;

    }

public:
    RealData ()
            : _payloadSize(0),
            _head(0),
            _tail(0),
            _maxFiles(0),
            _nPayloads(0),
            _mFiles(0),
            _loadRealDataThread(NULL)
    {};

    RealData (ParameterManager *PM)
            : _payloadSize(0),
            _head(0),
            _tail(0),
            _maxFiles(0),
            _nPayloads(0),
            _mFiles(0),
            _PM(PM),
            _loadRealDataThread(NULL)
    {};

    RealData (std::string path, ParameterManager *PM)
            : _pathToData(path),
            _payloadSize(0),
            _head(0),
            _tail(0),
            _maxFiles(0),
            _nPayloads(0),
            _mFiles(0),
            _PM(PM),
            _loadRealDataThread(NULL)
    {
        if (!initialize(path,PM)){
            /* Trow Exception */
        }
    }

    // RealData (const RealData &dt)
    // {
    //     _pathToData = dt._pathToData;
    //     _dataBuffer = dt._dataBuffer;
    //     _head = dt._head;
    //     _tail = dt._tail;
    //     _maxFiles = dt._maxFiles;
    //     _nPayloads = dt._nPayloads;
    //     _mFiles = dt._mFiles;
    //     _filesPath = dt._filesPath;
    //     _waitForData = dt._waitForData;
    //     _PM = dt._PM;
    // }

    bool initialize(std::string path, ParameterManager *PM)
    {
        _pathToData = path;
        _PM = PM;
        if (!loadData(path)){
            //TODO: Call Destructor
            return false;
        }

        _loadRealDataThread = PerftestThread_new(
                "loadingThread",
                Perftest_THREAD_PRIORITY_DEFAULT,
                RTI_OSAPI_THREAD_OPTION_STDIO,
                loadDataAsynchronous,
                this);

        return true;
    }

    //TODO: create destructor

    bool loadData(std::string path){
        _pathToData = path;
        struct stat s;
        int actualbytesRead = 0;
        int actualFileSize = 0;
        int totalBytesRead = 0;
        _maxFiles = _PM->get<unsigned long long>("maximumFilesOnMemory");
        _payloadSize = 0;

        /* This variable will be necessary for a future implementation of live Data */
        _waitForData = _PM->get<unsigned long long>("waitForNewData");
        _waitForData = true;

        /*
         * Initial allocation for _dataBuffer, this will not allocate the
         * payload, just the RealPayload structure
         */
        _dataBuffer.resize(_maxFiles);

        if( stat(path.c_str(),&s) == 0 ) {
            if( s.st_mode & S_IFDIR ) {
                /* The path is a directory */
                _filesPath = getFilesPathFromDirectory(path);
                if (_filesPath.empty()){
                    fprintf(
                        stderr,
                        "Any regular file found on directory %s\n",
                        path.c_str());
                    return false;
                }
            }
            else if( s.st_mode & S_IFREG ) {
                /* The path is a file */
                _filesPath.push_back(path);
            }
        } else {
            fprintf(
                stderr,
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
                fprintf(
                    stderr,
                    "[WARNING] - The file/s %s is/are to big to be allocated "
                    "and sent them as one samples, set -dataLen value to divide"
                    " the file/s on samples of size of -dataLen. The file/s are"
                    " going to be truncated to %d size\n"
                    "Check -realPayload documentation for more information\n",
                    path.c_str(),
                    MAXIMUN_REALDATA_ALLOCABLE);
                _payloadSize = MAXIMUN_REALDATA_ALLOCABLE;
            }
            // TODO: Should we check the range of the valid size?
            _PM->set<unsigned long long>("dataLen", _payloadSize);
        }

        while (_mFiles < _filesPath.size()) {

            actualFileSize = getFileSize(_filesPath[_mFiles]);

            if ((unsigned int)(_payloadSize + totalBytesRead)
                    > MAXIMUN_REALDATA_ALLOCABLE) {
                /*
                 * End the loop. We have reach the maximun data allocable.
                 * If there are more data that has not been load yet, it will be
                 * allocable as soon as the main thread send actual data on the
                 * queue
                 */
                break;
            }

            actualbytesRead = 0;
            while ((actualFileSize - actualbytesRead) >= (int)_payloadSize
                    && (_payloadSize + totalBytesRead + actualbytesRead)
                            <= MAXIMUN_REALDATA_ALLOCABLE) {

                actualbytesRead += readFile(
                        _filesPath[_mFiles],
                        _dataBuffer[_nPayloads]._payload,
                        _payloadSize,
                        actualbytesRead);

                if (actualbytesRead == -1) {
                    fprintf(stderr,
                            "Error reading file %s\n",
                            _filesPath[_mFiles].c_str());
                    return false;
                }

                _nPayloads++;

                /*
                 * If the queue is full, increase the size, later the queue will
                 * be shrink to fit. (We reserve the double of memory due to
                 * performance benefits)
                 */
                if (_dataBuffer.size() == _nPayloads) {
                    _dataBuffer.resize(_nPayloads * 2);
                }

            }
            totalBytesRead += actualbytesRead;
            _mFiles++;
        }

        // TODO: remove, just for testing
        printf ("Buffers read: %d --- Files: %d --- PathFiles: %d\n", _nPayloads, _mFiles, _pathToData.size());

        /* Reduce the counter of files to the real value */
        _mFiles--;

        /* Shrink the queue to the actual used size */
        _dataBuffer.resize(_nPayloads);

        /*
         * This will be avoid extra calculation for the loadingThread to resolve
         * the index of the last buffer read on the initialization.
         */
        _bytesReadFromLastFile = actualbytesRead;

        return true;
    }


    // std::vector<char>& getFirstPayload()
    // {
    //    return _dataBuffer[0]._payload;
    // }

    // std::vector<char>& getNextPayload()
    // {
    //     return _dataBuffer[nextIndex(_head)]._payload;
    // }

    /* This is a future implementation in case we want to read live Data */
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
                while (_dataBuffer[_head]._read){}
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