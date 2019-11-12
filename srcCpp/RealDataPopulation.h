#ifndef __REAL_DATA_POPULATION__
#define __REAL_DATA_POPULATION__

/* Forward declaration since there are som mutual dependencies between classes */
class RealData;

#include <iostream>
#include <vector>
#include <fstream>
#include "ParameterManager.h"
#include "Infrastructure_common.h"

/* #include "perftest_cpp.h" is included at the end of the file */

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
    ~RealPayload()
    {
        if (_payloadAccessSem != NULL) {
            PerftestSemaphore_delete(_payloadAccessSem);
            _payloadAccessSem = NULL;
        }
    }
};

class RealData {
    const unsigned int MAXIMUN_REALDATA_ALLOCABLE = 1073741824; // 1Gb

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

    bool _ensureDataOrder;

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
    static void *loadDataAsynchronous(void *arg);

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

    /* Initialice the queue and the asynchronous thread to push the data */
    bool initialize(std::string path, ParameterManager *PM);

    std::vector<char> &getNextPayload();

    bool pushPayload(std::vector<char> &realPayload);
};

#endif

/*
 * perftest_cpp class needs RealData to be declared since is one of his members
 */
#include "perftest_cpp.h"
