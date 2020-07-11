#include "FileDataLoader.h"

/* Initialize function that calculates and loads into memory a given file*/
bool FileDataLoader::initialize(std::string path, ParameterManager *PM)
{
    _PM = PM;

    std::cerr << "\nLoading data from file:" << std::endl;

    /*
     * This is the maximum size we allow Perftest to use to store
     * things in memory. This value needs to be changeable via Command line
     * since it might be too high for small systems.
     */
    _maximumAllocableSpace
            = _PM->get<unsigned long long>("maximumAllocableBufferSize");
    std::cerr << "\tMaximum buffer space: "
              << _maximumAllocableSpace
              << " Bytes"
              << std::endl;

    /* Check if the path is regular file */
    if (!PerftestFileHandler::path_is_file(path)) {
        std::cerr << "[Error] FileDataLoader::initialize: Error loading \""
                  << path << "\""
                  << std::endl;
        return false;
    }

    _filePath = path;
    std::cerr << "\tFile name: " << path << std::endl;

    long fileSize = PerftestFileHandler::get_file_size(_filePath);
    if (fileSize > 0) {
        _fileSize = (unsigned long) fileSize;
    } else {
        std::cerr << "[Error] FileDataLoader::initialize File size "
                  << fileSize
                  << " is <= 0"
                  << std::endl;
        return false;
    }
    std::cerr << "\tFile Size: " << _fileSize << " Bytes" << std::endl;

    /*
     * If datalen is set in RTI Perftest, we will use it as the size of the
     * buffers we will store. If DataSize is not set. We will try to send the
     * file as a whole, by setting DataSize to the size of the file + overhead.
     */
    if (_PM->is_set("dataLen")) {
        //TODO: Use the overhead function once it is available.
        /*
         * We will not handle if the dataLen is larger than the maximum sample
         * size. Perftest_cpp will is in charge of doing that already.
         */
        _bufferSize = _PM->get<unsigned long long>("dataLen")
                - perftest_cpp::OVERHEAD_BYTES;

        if (_bufferSize > _maximumAllocableSpace) {
            std::cerr << "[Error] FileDataLoader::initialize: Data Size is set to "
                      << _PM->get<unsigned long long>("dataLen")
                      << " Bytes, but the maximum size allocable for loadind "
                      << "files is set to "
                      << _maximumAllocableSpace
                      << " Bytes"
                      << std::endl;
            return false;
        }
    } else {
        _bufferSize = _fileSize;
        /*
         * If the file is larger than the maximum buffer we are allowed to
         * allocate, then use that maximum buffer.
         */
        if (_bufferSize > _maximumAllocableSpace) {
            _bufferSize = _maximumAllocableSpace;
            std::cerr << "[WARNING] FileDataLoader::initialize: RTI Perftest will "
                      << "only load in memory the first "
                      << _maximumAllocableSpace
                      << " Bytes of the file. Use the "
                      << "-maximumAllocableBufferSize option top increase this"
                      << "limit."
                      << std::endl;
        }
        _PM->set<unsigned long long>(
                "dataLen",
                _bufferSize + perftest_cpp::OVERHEAD_BYTES);
        std::cerr << "\tData Size set to: "
                  << _bufferSize + perftest_cpp::OVERHEAD_BYTES
                  << " Bytes (File Size + overhead bytes)"
                  << std::endl;
    }

    std::cerr << "\tBuffer Size: " << _bufferSize
              << " Bytes (Datalen - overhead bytes) " << std::endl;

    /* Calculate the number of buffers we will use */
    _numberOfBuffers = (std::min)(_maximumAllocableSpace, _fileSize)
                            / _bufferSize;
    if (_numberOfBuffers < 1) {
        std::cerr << "[Error] FileDataLoader::initialize: Number of buffers cannot "
                  << " be smaller than 1. Function returned: "
                  << _numberOfBuffers
                  << std::endl;
        return false;
    }
    std::cerr << "\tNumber of Buffers: " << _numberOfBuffers << std::endl;

    if (!load_file_into_buffers()) {
        std::cerr << "[Error] FileDataLoader::initialize: Failure loading "
                  << "data into buffers."
                  << std::endl;
        return false;
    }

    std::cerr << "Data loaded successfully."
              << std::endl;
    return true;
}

bool FileDataLoader::load_file_into_buffers()
{
    int totalBytesRead = 0;

    /* Lets first resize the vector of buffers to the proper size */
    _dataBuffers.resize(_numberOfBuffers);

    /* Now we need to iterate through the vector filling each of the buffers */
    for (unsigned int i = 0; i < _dataBuffers.size(); i++) {

        _dataBuffers[i] = new char[_bufferSize];
        int currentBytesRead = PerftestFileHandler::read_file(
                    _filePath,        // filename
                    _dataBuffers[i],  // where to store it
                    _bufferSize,      // size to read
                    totalBytesRead);  // where to start reading

        if (currentBytesRead == -1) {
            std::cerr << "[Error] FileDataLoader::load_file_into_buffers "
                        << "error reading file"
                        << std::endl;
            return false;
        }

        totalBytesRead += currentBytesRead;
        /* This should be equivalent to say _bufferSize if everything is ok */
    }

    return true;
}

unsigned int FileDataLoader::get_next_index()
{
    _readIndex = (_readIndex + 1) % _numberOfBuffers;
    return _readIndex;
}

char * FileDataLoader::get_next_buffer()
{
    char * currentBuffer = _dataBuffers[_readIndex];

    /* Move already the iterator to the next buffer */
    get_next_index();

    return currentBuffer;
}
