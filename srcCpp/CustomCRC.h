#include "rti_me_cpp.hxx"

RTI_BOOL
CustomCRC_crc16(void *context,
                const struct REDA_Buffer *buf,
                unsigned int buf_length,
                union RTPS_CrcChecksum *checksum);

RTI_BOOL
CustomCRC_crc32(void *context,
                const struct REDA_Buffer *buf,
                unsigned int buf_length,
                union RTPS_CrcChecksum *checksum);


RTI_BOOL
CustomCRC_crc64(void *context,
                const struct REDA_Buffer *buf,
                unsigned int buf_length,
                union RTPS_CrcChecksum *checksum);

RTI_BOOL
CustomCRC_crc128(void *context,
                 const struct REDA_Buffer *buf,
                 unsigned int buf_length,
                 union RTPS_CrcChecksum *checksum);




// TODO: Only for testing, remove

RTI_BOOL
CustomCRC_crc32_Hardware_1_byte(void *context,
                                const struct REDA_Buffer *buf,
                                unsigned int buf_length,
                                union RTPS_CrcChecksum *checksum);

RTI_BOOL
CustomCRC_crc32_Hardware_8_byte(void *context,
                                const struct REDA_Buffer *buf,
                                unsigned int buf_length,
                                union RTPS_CrcChecksum *checksum);

CustomCRC_crc32_Linux_Kernel(void *context,
                             const struct REDA_Buffer *buf,
                             unsigned int buf_length,
                             union RTPS_CrcChecksum *checksum);