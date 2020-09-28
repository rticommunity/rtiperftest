#ifndef __FILE_DATA_LOADER__
#define __FILE_DATA_LOADER__

#include <vector>
#include <algorithm>
#include "ParameterManager.h"
#include "Infrastructure_common.h"

class FileDataLoader {

  private:
    /* This is the maximum size we allow to store when creating all buffers */
    unsigned long _maximumAllocableSpace;
    /* Path to file */
    std::string _filePath;
    /* Size of the file */
    unsigned long _fileSize;
    /* Size of each of the buffers */
    unsigned long _bufferSize;
    /* Number of buffers */
    unsigned int _numberOfBuffers;
    /* Queue */
    std::vector<char *> _dataBuffers;
    /* Reading index*/
    unsigned int _readIndex;

    ParameterManager *_PM;

    /* Auxiliary function in charge of loading the buffers */
    bool load_file_into_buffers();

    /*
     * Auxiliary function to obtain the next index to use, for the next buffer
     * to read.
     */
    unsigned int get_next_index();

  public:

    FileDataLoader() :
            _maximumAllocableSpace(0),
            _fileSize(0),
            _bufferSize(0),
            _numberOfBuffers(0),
            _readIndex(0)
    {};

    FileDataLoader(ParameterManager *PM) :
            _maximumAllocableSpace(0),
            _fileSize(0),
            _bufferSize(0),
            _numberOfBuffers(0),
            _readIndex(0),
            _PM(PM)
    {};

    ~FileDataLoader()
    {
        for (unsigned int i = 0; i < _numberOfBuffers; i++) {
            delete []_dataBuffers[i];
        }
    };

    /* Initialice function that calculates and loads into memory a given file */
    bool initialize(std::string path, ParameterManager *PM);

    char * get_next_buffer();
};

/*
 * perftest_cpp class needs FileDataLoader to be declared, since it uses it.
 * And we need perftest since we need to gather the information about the
 * overhead.
 */
#include "perftest_cpp.h"

#endif //__FILE_DATA_LOADER__
