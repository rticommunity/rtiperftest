#include "CustomCRC.h"

#if defined(CRC_HARDWARE_SLIDE_BY_ONE) || defined(CRC_HARDWARE_SLIDE_BY_EIGHT)
  #include <immintrin.h> // _mm_crc32_u8, _mm_crc32_u64
#endif

#ifdef CRC_ZLIB
  #include <zlib.h> // Zlib's crc32
#endif

RTI_BOOL
CustomCRC_crc16(void *context,
                const struct REDA_Buffer *buf,
                unsigned int buf_length,
                union RTPS_CrcChecksum *checksum)
{
    RTI_UINT16 crc = 0;
    unsigned char *data = (unsigned char *) buf[0].pointer;
    RTI_UINT32 length = buf[0].length;
    int k;

    UNUSED_ARG(k);
    UNUSED_ARG(context);
    UNUSED_ARG(buf_length);

    if (data == NULL)
    {
        return 0x6363;
    }

    crc &= 0xffff;

    while (length--)
    {
        crc ^= *data++;
        for (k = 0; k < 8; k++)
        {
            crc = crc & 1 ? (crc >> 1) ^ 0x8408 : crc >> 1;
        }
    }

    checksum->crc16 = crc;

    return RTI_TRUE;
}

RTI_BOOL
CustomCRC_crc32(void *context,
                const struct REDA_Buffer *buf,
                unsigned int buf_length,
                union RTPS_CrcChecksum *checksum)
{
  #ifdef CRC_BIT_BY_BIT
    return CustomCRC_crc32_BitByBit(NULL, buf, buf_length, checksum);
  #elif CRC_SLIDE_BY_ONE
    return CustomCRC_crc32_SlideByOne(NULL, buf, buf_length, checksum);
  #elif CRC_SLIDE_BY_FOUR
    return CustomCRC_crc32_SlideByFour(NULL, buf, buf_length, checksum);
  #elif CRC_HARDWARE_SLIDE_BY_ONE
    return CustomCRC_crc32_Hardware_SlideByOne(NULL, buf, buf_length, checksum);
  #elif CRC_HARDWARE_SLIDE_BY_EIGHT
    return CustomCRC_crc32_Hardware_SlideByEight(NULL, buf, buf_length, checksum);
  #elif CRC_ZLIB
    return CustomCRC_crc32_Zlib(NULL, buf, buf_length, checksum);
  #else
    #warning "Using CustomCRC_crc32_BitByBit as default Custom CRC-32 function"
    return CustomCRC_crc32_BitByBit(NULL, buf, buf_length, checksum);
  #endif
}

RTI_BOOL
CustomCRC_crc64(void *context,
                const struct REDA_Buffer *buf,
                unsigned int buf_length,
                union RTPS_CrcChecksum *checksum)
{
    RTI_UINT64 crc = 0;
    unsigned char *data = (unsigned char *) buf[0].pointer;
    RTI_UINT32 length = buf[0].length;
    int k;

    UNUSED_ARG(k);
    UNUSED_ARG(context);
    UNUSED_ARG(buf_length);

    if (data == NULL)
    {
        return 0;
    }

    while (length--)
    {
        crc ^= (unsigned long)(*data++) << 56;

        for (k = 0; k < 8; k++)
        {
            crc = crc & 0x8000000000000000 ? (crc << 1) ^ 0x42f0e1eba9ea3693 : crc << 1;
        }
    }

    checksum->crc64 = crc;

    return RTI_TRUE;
}

RTI_BOOL
CustomCRC_crc128(void *context,
                 const struct REDA_Buffer *buf,
                 unsigned int buf_length,
                 union RTPS_CrcChecksum *checksum)
{
    return RTI_TRUE;
}




// TODO: Only for testing, remove

inline RTI_BOOL
CustomCRC_crc32_BitByBit(void *context,
                         const struct REDA_Buffer *buf,
                         unsigned int buf_length,
                         union RTPS_CrcChecksum *checksum)
{
    RTI_UINT32 crc = 0;
    unsigned char *data = (unsigned char *) buf[0].pointer;
    RTI_UINT32 length = buf[0].length;
    int k;

    UNUSED_ARG(k);
    UNUSED_ARG(context);
    UNUSED_ARG(buf_length);

    if (data == NULL)
    {
        return 0;
    }

    crc = ~crc;

    while (length--)
    {
        crc ^= *data++;
        for (k = 0; k < 8; k++)
        {
            crc = crc & 1 ? (crc >> 1) ^ 0xedb88320 : crc >> 1;
        }
    }

    checksum->crc32 = ~crc;

    return RTI_TRUE;
}

#ifdef CRC_HARDWARE_SLIDE_BY_ONE
// Hardware slice by byte
inline RTI_BOOL
CustomCRC_crc32_Hardware_SlideByOne(void *context,
                                const struct REDA_Buffer *buf,
                                unsigned int buf_length,
                                union RTPS_CrcChecksum *checksum)
{
    RTI_UINT32 crc = 0;
    unsigned char *data = (unsigned char *) buf[0].pointer;
    RTI_UINT32 length = buf[0].length;
    int k;

    UNUSED_ARG(k);
    UNUSED_ARG(context);
    UNUSED_ARG(buf_length);

    for (k = 0; k < length; k++) {
        crc = _mm_crc32_u8(crc, data[k]);
    }

    checksum->crc32 = crc;

    return RTI_TRUE;
}
#endif

#ifdef CRC_HARDWARE_SLIDE_BY_EIGHT
// Hardware slice by 8 bytes
inline RTI_BOOL
CustomCRC_crc32_Hardware_SlideByEight(void *context,
                                      const struct REDA_Buffer *buf,
                                      unsigned int buf_length,
                                      union RTPS_CrcChecksum *checksum)
{
    RTI_UINT32 crc = 0;
    RTI_UINT64 *data = (RTI_UINT64 *) buf[0].pointer;
    RTI_UINT32 length = buf[0].length / (sizeof(RTI_UINT64) / sizeof(char));
    int k;

    UNUSED_ARG(k);
    UNUSED_ARG(context);
    UNUSED_ARG(buf_length);

    for (k = 0; k < length; k++) {
        crc = _mm_crc32_u64(crc, data[k]);
    }

    checksum->crc32 = crc;

    return RTI_TRUE;
}
#endif

#ifdef CRC_ZLIB
// Zlib CRC-32 Implementation
inline RTI_BOOL
CustomCRC_crc32_Zlib(void *context,
                     const struct REDA_Buffer *buf,
                     unsigned int buf_length,
                     union RTPS_CrcChecksum *checksum)
{
    UNUSED_ARG(context);
    UNUSED_ARG(buf_length);

    RTI_UINT32 crc = crc32(0L, Z_NULL, 0);

    crc = crc32(crc, (unsigned char*) buf[0].pointer, buf[0].length);

    checksum->crc32  = crc;

    return RTI_TRUE;
}
#endif

#define DO1 crc = crc_table_SlideByFour[0][((int)crc ^ (*current_byte++)) & 0xff] ^ (crc >> 8)
#define DO8 DO1; DO1; DO1; DO1; DO1; DO1; DO1; DO1

inline RTI_BOOL
CustomCRC_crc32_SlideByOne(void *context,
                           const struct REDA_Buffer *buf,
                           unsigned int buf_length,
                           union RTPS_CrcChecksum *checksum)
{
    const unsigned char *current_byte = (const unsigned char *) buf[0].pointer;
    RTI_UINT32 crc = ~checksum->crc32;
    RTI_UINT32 length = buf[0].length;

    UNUSED_ARG(context);
    UNUSED_ARG(buf_length);

    while (length >= 8) {
        DO8;
        length -= 8;
    }

    checksum->crc32  = crc;

    return RTI_TRUE;
}

#define DOLIT4 crc ^= *current_word++; \
        crc = crc_table_SlideByFour[3][crc & 0xff] ^ crc_table_SlideByFour[2][(crc >> 8) & 0xff] ^ \
            crc_table_SlideByFour[1][(crc >> 16) & 0xff] ^ crc_table_SlideByFour[0][crc >> 24]
#define DOLIT32 DOLIT4; DOLIT4; DOLIT4; DOLIT4; DOLIT4; DOLIT4; DOLIT4; DOLIT4

inline RTI_BOOL
CustomCRC_crc32_SlideByFour(void *context,
                            const struct REDA_Buffer *buf,
                            unsigned int buf_length,
                            union RTPS_CrcChecksum *checksum)
{
    const RTI_UINT32 *current_word = (const RTI_UINT32 *) buf[0].pointer;
    RTI_UINT32 crc = ~checksum->crc32;
    RTI_UINT32 length = buf[0].length;

    UNUSED_ARG(context);
    UNUSED_ARG(buf_length);

    while (length >= 32) {
        DOLIT32;
        length -= 32;
    }

    checksum->crc32  = crc;

    return RTI_TRUE;
}