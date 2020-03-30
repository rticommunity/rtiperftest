#include "CustomCRC.h"

#if defined(CRC_HARDWARE_SLIDE_BY_ONE) || defined(CRC_HARDWARE_SLIDE_BY_EIGHT)
  #include <immintrin.h> // _mm_crc32_u8, _mm_crc32_u64
#endif

#ifdef CRC_ZLIB
  #include <zlib.h> // Zlib's crc32
#endif

RTI_BOOL
CustomCRC_crc32(void *context,
                const struct REDA_Buffer *buf,
                unsigned int buf_length,
                RTPS_Checksum_T *checksum)
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
                RTPS_Checksum_T *checksum)
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

    checksum->checksum64 = crc;

    return RTI_TRUE;
}

// RTI_BOOL
// CustomCRC_crc128(void *context,
//                  const struct REDA_Buffer *buf,
//                  unsigned int buf_length,
//                  RTPS_Checksum_T *checksum)
// {
//     return RTI_FALSE;
// }

RTI_BOOL
CustomCRC_crc256(void *context,
                 const struct REDA_Buffer *buf,
                 unsigned int buf_length,
                 RTPS_Checksum_T *checksum)
{
    return RTI_FALSE;
}




// TODO: Only for testing, remove

inline RTI_BOOL
CustomCRC_crc32_BitByBit(void *context,
                         const struct REDA_Buffer *buf,
                         unsigned int buf_length,
                         RTPS_Checksum_T *checksum)
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

    checksum->checksum32 = ~crc;

    return RTI_TRUE;
}

#ifdef CRC_HARDWARE_SLIDE_BY_ONE
// Hardware slice by byte
inline RTI_BOOL
CustomCRC_crc32_Hardware_SlideByOne(void *context,
                                const struct REDA_Buffer *buf,
                                unsigned int buf_length,
                                RTPS_Checksum_T *checksum)
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

    checksum->checksum32 = crc;

    return RTI_TRUE;
}
#endif

#ifdef CRC_HARDWARE_SLIDE_BY_EIGHT
// Hardware slice by 8 bytes
inline RTI_BOOL
CustomCRC_crc32_Hardware_SlideByEight(void *context,
                                      const struct REDA_Buffer *buf,
                                      unsigned int buf_length,
                                      RTPS_Checksum_T *checksum)
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

    checksum->checksum32 = crc;

    return RTI_TRUE;
}
#endif

#ifdef CRC_ZLIB
// Zlib CRC-32 Implementation
inline RTI_BOOL
CustomCRC_crc32_Zlib(void *context,
                     const struct REDA_Buffer *buf,
                     unsigned int buf_length,
                     RTPS_Checksum_T *checksum)
{
    UNUSED_ARG(context);
    UNUSED_ARG(buf_length);

    RTI_UINT32 crc = crc32(0L, Z_NULL, 0);

    crc = crc32(crc, (unsigned char*) buf[0].pointer, buf[0].length);

    checksum->checksum32  = crc;

    return RTI_TRUE;
}
#endif

#define DO1 crc = crc_table_SlideByFour[0][((int)crc ^ (*current_byte++)) & 0xff] ^ (crc >> 8)
#define DO8 DO1; DO1; DO1; DO1; DO1; DO1; DO1; DO1

inline RTI_BOOL
CustomCRC_crc32_SlideByOne(void *context,
                           const struct REDA_Buffer *buf,
                           unsigned int buf_length,
                           RTPS_Checksum_T *checksum)
{
    const unsigned char *current_byte = (const unsigned char *) buf[0].pointer;
    RTI_UINT32 crc = ~checksum->checksum32;
    RTI_UINT32 length = buf[0].length;

    UNUSED_ARG(context);
    UNUSED_ARG(buf_length);

    while (length >= 8) {
        DO8;
        length -= 8;
    }

    checksum->checksum32  = crc;

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
                            RTPS_Checksum_T *checksum)
{
    const RTI_UINT32 *current_word = (const RTI_UINT32 *) buf[0].pointer;
    RTI_UINT32 crc = ~checksum->checksum32;
    RTI_UINT32 length = buf[0].length;

    UNUSED_ARG(context);
    UNUSED_ARG(buf_length);

    while (length >= 32) {
        DOLIT32;
        length -= 32;
    }

    checksum->checksum32  = crc;

    return RTI_TRUE;
}





/* MD5 calc helpers */
#define F1(x, y, z) (z ^ (x & (y ^ z)))
#define F2(x, y, z) F1(z, x, y)
#define F3(x, y, z) (x ^ y ^ z)
#define F4(x, y, z) (y ^ (x | ~z))

/* This is the central step in the MD5 algorithm. */
#define RTPSMd5Checksum_Step(f, w, x, y, z, data, s) \
	( w += f(x, y, z) + data, w &= 0xffffffff, w = w<<s | w>>(32-s), w += x )

#define RTPSMd5Checksum_Transform(a, b, c, d, md5_buffer, data) \
    a = md5_buffer[0]; \
    b = md5_buffer[1]; \
    c = md5_buffer[2]; \
    d = md5_buffer[3]; \
    \
    RTPSMd5Checksum_Step(F1, a, b, c, d, data[ 0]+0xd76aa478,  7); \
    RTPSMd5Checksum_Step(F1, d, a, b, c, data[ 1]+0xe8c7b756, 12); \
    RTPSMd5Checksum_Step(F1, c, d, a, b, data[ 2]+0x242070db, 17); \
    RTPSMd5Checksum_Step(F1, b, c, d, a, data[ 3]+0xc1bdceee, 22); \
    RTPSMd5Checksum_Step(F1, a, b, c, d, data[ 4]+0xf57c0faf,  7); \
    RTPSMd5Checksum_Step(F1, d, a, b, c, data[ 5]+0x4787c62a, 12); \
    RTPSMd5Checksum_Step(F1, c, d, a, b, data[ 6]+0xa8304613, 17); \
    RTPSMd5Checksum_Step(F1, b, c, d, a, data[ 7]+0xfd469501, 22); \
    RTPSMd5Checksum_Step(F1, a, b, c, d, data[ 8]+0x698098d8,  7); \
    RTPSMd5Checksum_Step(F1, d, a, b, c, data[ 9]+0x8b44f7af, 12); \
    RTPSMd5Checksum_Step(F1, c, d, a, b, data[10]+0xffff5bb1, 17); \
    RTPSMd5Checksum_Step(F1, b, c, d, a, data[11]+0x895cd7be, 22); \
    RTPSMd5Checksum_Step(F1, a, b, c, d, data[12]+0x6b901122,  7); \
    RTPSMd5Checksum_Step(F1, d, a, b, c, data[13]+0xfd987193, 12); \
    RTPSMd5Checksum_Step(F1, c, d, a, b, data[14]+0xa679438e, 17); \
    RTPSMd5Checksum_Step(F1, b, c, d, a, data[15]+0x49b40821, 22); \
    \
    RTPSMd5Checksum_Step(F2, a, b, c, d, data[ 1]+0xf61e2562,  5); \
    RTPSMd5Checksum_Step(F2, d, a, b, c, data[ 6]+0xc040b340,  9); \
    RTPSMd5Checksum_Step(F2, c, d, a, b, data[11]+0x265e5a51, 14); \
    RTPSMd5Checksum_Step(F2, b, c, d, a, data[ 0]+0xe9b6c7aa, 20); \
    RTPSMd5Checksum_Step(F2, a, b, c, d, data[ 5]+0xd62f105d,  5); \
    RTPSMd5Checksum_Step(F2, d, a, b, c, data[10]+0x02441453,  9); \
    RTPSMd5Checksum_Step(F2, c, d, a, b, data[15]+0xd8a1e681, 14); \
    RTPSMd5Checksum_Step(F2, b, c, d, a, data[ 4]+0xe7d3fbc8, 20); \
    RTPSMd5Checksum_Step(F2, a, b, c, d, data[ 9]+0x21e1cde6,  5); \
    RTPSMd5Checksum_Step(F2, d, a, b, c, data[14]+0xc33707d6,  9); \
    RTPSMd5Checksum_Step(F2, c, d, a, b, data[ 3]+0xf4d50d87, 14); \
    RTPSMd5Checksum_Step(F2, b, c, d, a, data[ 8]+0x455a14ed, 20); \
    RTPSMd5Checksum_Step(F2, a, b, c, d, data[13]+0xa9e3e905,  5); \
    RTPSMd5Checksum_Step(F2, d, a, b, c, data[ 2]+0xfcefa3f8,  9); \
    RTPSMd5Checksum_Step(F2, c, d, a, b, data[ 7]+0x676f02d9, 14); \
    RTPSMd5Checksum_Step(F2, b, c, d, a, data[12]+0x8d2a4c8a, 20); \
    \
    RTPSMd5Checksum_Step(F3, a, b, c, d, data[ 5]+0xfffa3942,  4); \
    RTPSMd5Checksum_Step(F3, d, a, b, c, data[ 8]+0x8771f681, 11); \
    RTPSMd5Checksum_Step(F3, c, d, a, b, data[11]+0x6d9d6122, 16); \
    RTPSMd5Checksum_Step(F3, b, c, d, a, data[14]+0xfde5380c, 23); \
    RTPSMd5Checksum_Step(F3, a, b, c, d, data[ 1]+0xa4beea44,  4); \
    RTPSMd5Checksum_Step(F3, d, a, b, c, data[ 4]+0x4bdecfa9, 11); \
    RTPSMd5Checksum_Step(F3, c, d, a, b, data[ 7]+0xf6bb4b60, 16); \
    RTPSMd5Checksum_Step(F3, b, c, d, a, data[10]+0xbebfbc70, 23); \
    RTPSMd5Checksum_Step(F3, a, b, c, d, data[13]+0x289b7ec6,  4); \
    RTPSMd5Checksum_Step(F3, d, a, b, c, data[ 0]+0xeaa127fa, 11); \
    RTPSMd5Checksum_Step(F3, c, d, a, b, data[ 3]+0xd4ef3085, 16); \
    RTPSMd5Checksum_Step(F3, b, c, d, a, data[ 6]+0x04881d05, 23); \
    RTPSMd5Checksum_Step(F3, a, b, c, d, data[ 9]+0xd9d4d039,  4); \
    RTPSMd5Checksum_Step(F3, d, a, b, c, data[12]+0xe6db99e5, 11); \
    RTPSMd5Checksum_Step(F3, c, d, a, b, data[15]+0x1fa27cf8, 16); \
    RTPSMd5Checksum_Step(F3, b, c, d, a, data[ 2]+0xc4ac5665, 23); \
    \
    RTPSMd5Checksum_Step(F4, a, b, c, d, data[ 0]+0xf4292244,  6); \
    RTPSMd5Checksum_Step(F4, d, a, b, c, data[ 7]+0x432aff97, 10); \
    RTPSMd5Checksum_Step(F4, c, d, a, b, data[14]+0xab9423a7, 15); \
    RTPSMd5Checksum_Step(F4, b, c, d, a, data[ 5]+0xfc93a039, 21); \
    RTPSMd5Checksum_Step(F4, a, b, c, d, data[12]+0x655b59c3,  6); \
    RTPSMd5Checksum_Step(F4, d, a, b, c, data[ 3]+0x8f0ccc92, 10); \
    RTPSMd5Checksum_Step(F4, c, d, a, b, data[10]+0xffeff47d, 15); \
    RTPSMd5Checksum_Step(F4, b, c, d, a, data[ 1]+0x85845dd1, 21); \
    RTPSMd5Checksum_Step(F4, a, b, c, d, data[ 8]+0x6fa87e4f,  6); \
    RTPSMd5Checksum_Step(F4, d, a, b, c, data[15]+0xfe2ce6e0, 10); \
    RTPSMd5Checksum_Step(F4, c, d, a, b, data[ 6]+0xa3014314, 15); \
    RTPSMd5Checksum_Step(F4, b, c, d, a, data[13]+0x4e0811a1, 21); \
    RTPSMd5Checksum_Step(F4, a, b, c, d, data[ 4]+0xf7537e82,  6); \
    RTPSMd5Checksum_Step(F4, d, a, b, c, data[11]+0xbd3af235, 10); \
    RTPSMd5Checksum_Step(F4, c, d, a, b, data[ 2]+0x2ad7d2bb, 15); \
    RTPSMd5Checksum_Step(F4, b, c, d, a, data[ 9]+0xeb86d391, 21); \
    \
    md5_buffer[0] += a; \
    md5_buffer[1] += b; \
    md5_buffer[2] += c; \
    md5_buffer[3] += d;

RTI_PRIVATE inline void
RTPSMd5Checksum_int_to_str(RTI_UINT32 data, unsigned char *str)
{
	str[0] = (unsigned char)data;
	str[1] = (unsigned char)(data >> 8);
	str[2] = (unsigned char)(data >> 16);
	str[3] = (unsigned char)(data >> 24);
}

/*ci
 * \brief Calculate MD5
 *
 * \details
 * Calculates MD5 checksum for buf.
 *
 * \param[in]   context
 * \param[in]   buf         The buffer to calculate the checksum from.
 * \param[in]   buf_length  The length of buf
 * \param[out]  checksum    The calculated MD5 checksum. Cannot be Null.
 *                          If filled with 0, will calculate the CRC-32 checksum that
 *                          should be used to send the message. If different from 0,
 *                          will overwrite this buffer with the resulting division:
 *                          if the result is different from 0, the message is corrupted.
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
RTI_BOOL
CustomCRC_crc128(void *context,
                 const struct REDA_Buffer *buf,
                 RTI_UINT32 buf_length,
                 RTPS_Checksum_T *checksum)
{
    RTI_UINT32 length = 0;
    const RTI_UINT32 *current_word;
    unsigned char *current_char;
    RTI_UINT32 md5_buffer[4];
    RTI_UINT32 bits[2];
    RTI_UINT32 count;
    unsigned char *p;
    register RTI_UINT32 a, b, c, d;

    UNUSED_ARG(context);
    UNUSED_ARG(buf_length);

    OSAPI_PRECONDITION((buf == NULL) || (checksum == NULL) || (buf_length != 1) ,
                       return RTI_FALSE,
                       OSAPI_Log_entry_add_pointer("buf", buf, RTI_FALSE);
                       OSAPI_Log_entry_add_uint("buf_length", buf_length, RTI_FALSE);
                       OSAPI_Log_entry_add_pointer("checksum", checksum, RTI_TRUE);)

    /* This assumes that the input buffer is aligned to 32 bits, which is
     * the case in Micro.
     */
    current_word = (RTI_UINT32 *) buf[0].pointer;
    current_char = (unsigned char *) buf[0].pointer;
    length = buf[0].length;

    /* Initialize MD5 buffer */
    md5_buffer[0] = 0x67452301;
	md5_buffer[1] = 0xefcdab89;
	md5_buffer[2] = 0x98badcfe;
	md5_buffer[3] = 0x10325476;

	bits[0] = 0;
	bits[1] = 0;

    if ((bits[0] = (bits[0] + ((RTI_UINT32)length << 3)) & 0xffffffff) < bits[0])
    {
        bits[1]++;	/* Carry from low to high */
    }

    /* Process input data in 64-bytes chunks */
    while (length >= 64)
    {
        RTPSMd5Checksum_Transform(a, b, c, d, md5_buffer, current_word);
        current_word += 16;
        current_char += 64;
        length -= 64;
    }

    /*
    * Final wrapup - pad to 64-byte boundary with the bit pattern
    * 1 0* (64-bit count of bits processed, MSB-first)
    */
    /* Compute number of bytes mod 64 */
    count = (bits[0] >> 3) & 0x3F;

    /* Set the first char of padding to 0x80. */
    p = current_char + count;
    *p++ = 0x80;

    /* Bytes of padding needed to make 64 bytes */
	count = 64 - 1 - count;

    /* Pad out to 56 mod 64 */
	if (count < 8) {
		/* Two lots of padding:  Pad the first block to 64 bytes */

		OSAPI_Memory_zero(p, count);
        RTPSMd5Checksum_Transform(a, b, c, d, md5_buffer, current_word);

		/* Now fill the next block with 56 bytes */
        OSAPI_Memory_zero(p, 56);
	} else {
		/* Pad block to 56 bytes */
        OSAPI_Memory_zero(p, count-8);
	}

    /* Append length in bits and transform */
	RTPSMd5Checksum_int_to_str(bits[0], current_char + 56);
	RTPSMd5Checksum_int_to_str(bits[1], current_char + 60);

    RTPSMd5Checksum_Transform(a, b, c, d, md5_buffer, current_word);
	RTPSMd5Checksum_int_to_str(md5_buffer[0], checksum->checksum128);
	RTPSMd5Checksum_int_to_str(md5_buffer[1], checksum->checksum128 + 4);
	RTPSMd5Checksum_int_to_str(md5_buffer[2], checksum->checksum128 + 8);
	RTPSMd5Checksum_int_to_str(md5_buffer[3], checksum->checksum128 + 12);

    return RTI_TRUE;
}