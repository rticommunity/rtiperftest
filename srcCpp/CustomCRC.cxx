#include "CustomCRC.h"

#include <immintrin.h> // _mm_crc32_u8, _mm_crc32_u64

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

// Hardware slice by byte
RTI_BOOL
CustomCRC_crc32_Hardware_1_byte(void *context,
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

// Hardware slice by 8 bytes
RTI_BOOL
CustomCRC_crc32_Hardware_8_byte(void *context,
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