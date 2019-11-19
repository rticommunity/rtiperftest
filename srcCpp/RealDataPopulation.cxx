
#include "RealDataPopulation.h"

/*
 * Spawned Method for the asynchronous thread.
 * This method will load the files circularly and push them in to the queue
 * _dataBuffer if the files on the specified directory have reach the
 * maximun allocable memory defined, in other case, this thread will not be
 * longer necessary and will finish. The files will be read circularly
 */
void *RealData::loadDataAsynchronous(void *arg)
{
    RealData *rd = static_cast<RealData *>(arg);

    std::vector<char> newPayload(rd->_payloadSize);

    /* Allocate as many entries on the queue as could be possible */
    rd->_dataBuffer.resize(rd->MAX_REALDATA_ALLOCABLE / rd->_payloadSize);
    int currentFileSize = 0;
    int currentBytesRead = 0;
    int totalBytesRead = 0;
    bool queueFull = false;
    bool allFilesRead = false;

    /* This while condition should not ever be fulfilled */
    while (rd->_mFiles < rd->_filesPath.size()) {
        currentFileSize = getFileSize(rd->_filesPath[rd->_mFiles]);

        while ((unsigned int) (currentFileSize - currentBytesRead)
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

            if (!queueFull && (rd->_nPayloads == rd->_dataBuffer.size())) {
                /* This code can only be reached one time */
                queueFull = true;
            }

            if (queueFull || allFilesRead) {
                /*
                 * Let the main thread know that we have fill the queue in
                 * order to start pulling data from the queue.
                 */
                rd->_initializationFinished = true;

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
             * For now on, the data will be pushed circularly.
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

                /*
                 * Let the main thread know that we have fill the queue in
                 * order to start pulling data from the queue.
                 */
                rd->_initializationFinished = true;
                return NULL;
            }
        }

        /* Reset the index and start over from the first file */
        rd->_mFiles = (rd->_mFiles + 1) % rd->_filesPath.size();
    }

    return NULL;
}


/* Initialice the queue and the asynchronous thread to push the data */
bool RealData::initialize(std::string path, ParameterManager *PM)
{
    _pathToData = path;
    _PM = PM;

    struct stat s;
    _payloadSize = 0;

    _ensureDataOrder = _PM->get<unsigned long long>("ensureRealDataOrder");

    /* Check if the path given is a directory or a regular file */
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
        fprintf(stderr, "Could not open directory or file %s\n", path.c_str());
        return false;
    }

    /*
     * If -dataLen is set, the files will be divided in N peaces of -dataLen
     * size. If -dataLen is not set, the size of the payload will be the
     * smallest size on the files given.
     */
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
        if (_payloadSize > MAX_REALDATA_ALLOCABLE) {
            fprintf(stderr,
                    "[WARNING] - The file/s %s is/are to big to be "
                    "allocated and sent them as one samples, set -dataLen "
                    "value to divide the file/s on samples of size of "
                    "-dataLen. The file/s are going to be truncated to %d "
                    "size\nCheck -realPayload documentation for more "
                    "information\n",
                    path.c_str(),
                    MAX_REALDATA_ALLOCABLE);
            _payloadSize = MAX_REALDATA_ALLOCABLE;
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
    while (!_initializationFinished) {
        /* We can wait, the test did not start yet */
        PerftestClock::milliSleep(2000);
    };

    return true;
}

std::vector<char>& RealData::getNextPayload()
{
    std::vector<char> *resultPayload;
    RTIOsapiSemaphoreStatus semStatus;
    struct RTINtpTime block_inf = RTI_NTP_TIME_MAX;
    struct RTINtpTime block_zero;
    RTINtpTime_packFromMillisec(block_zero, 0, 0);


    /* Release previous pulled data to be filled again if it's need */
    if (!PerftestSemaphore_give(
            _dataBuffer[_head]._payloadAccessSem)) {
        fprintf(stderr, "Unexpected error giving semaphore\n");
        return *resultPayload;
    }

    /*
        * There are two possible scenarios, we want to ensure the order by
        * waiting for the data to be pushed via the asynchronous thread or
        * just take the first available data. This depends on
        * the _ensureDataOrder query from -ensureRealDataorder command line.
        */
    do{
        nextIndex(_head);
        semStatus = RTIOsapiSemaphore_take(
            _dataBuffer[_head]._payloadAccessSem,
            _ensureDataOrder? &block_inf : &block_zero);
    }while (semStatus == RTI_OSAPI_SEMAPHORE_STATUS_TIMEOUT);

    if (semStatus == RTI_OSAPI_SEMAPHORE_STATUS_OK) {
        /* If the status is OK, this means that we have available data */
        /*
            * If this payload has been read before, we wait for asynchronous
            * thread filling the queue to push new data except if all the Data
            * fit on the Queue, this means that the asynchronous thread is no
            * longer pushing data, or we do not need to ensure the order so
            * just take the first aviable data on the queue.
            */
        if (_dataBuffer[_head]._read && _ensureDataOrder && !_allDataFit) {
            /*
                * Release the access to let the asynchronous thread rewrite
                * the piece of data with a new one.
                */
            if (!PerftestSemaphore_give(
                    _dataBuffer[_head]._payloadAccessSem)) {
                fprintf(stderr, "Unexpected error giving semaphore\n");
                return *resultPayload;
            }
            /* Wait until the data is new */
            while (_dataBuffer[_head]._read){}

            /* Request the acces for this data */
            RTIOsapiSemaphore_take(
                _dataBuffer[_head]._payloadAccessSem,
                &block_inf);
        }
        /* Take the payload and set the flag read to True */
        resultPayload = &_dataBuffer[_head]._payload;
        _dataBuffer[_head]._read = true;
    } else {
        fprintf(stderr, "Unexpected error taking semaphore\n");
        return *resultPayload;
    }
    return *resultPayload;
}

bool RealData::pushPayload(std::vector<char> &realPayload)
{
    bool payloadRead = false;
    struct RTINtpTime block_inf = RTI_NTP_TIME_MAX;

    /* Request the acces fot the next piece of memory */
    if (RTIOsapiSemaphore_take(
            _dataBuffer[_tail]._payloadAccessSem, &block_inf)
            != RTI_OSAPI_SEMAPHORE_STATUS_OK) {
        fprintf(stderr, "Unexpected error taking semaphore\n");
        return false;
    }

    payloadRead = _dataBuffer[_tail]._read;

    /* If the payload has been read, we can just overwrite it */
    if (payloadRead) {
        _dataBuffer[_tail]._payload.swap(realPayload);
    }

    /* Release the memory even if has been overwrote or not */
    if (!PerftestSemaphore_give(
            _dataBuffer[_tail]._payloadAccessSem)) {
        fprintf(stderr, "Unexpected error giving semaphore\n");
        return false;
    }

    /*
        * If the payload has not been read, we wait until the memory is pull
        * out and then request the acces for that memory section to write over
        * it.
        */
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

    /* This is new data so it's has not been read yet */
    _dataBuffer[_tail]._read = false;

    /* Release the memory access */
    if (!PerftestSemaphore_give(
            _dataBuffer[_tail]._payloadAccessSem)) {
        fprintf(stderr, "Unexpected error giving semaphore\n");
        return false;
    }

    nextIndex(_tail);

    return true;
}
