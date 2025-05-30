#ifndef LIB_MODBUS
#define LIB_MODBUS

#include "appl_main.h"

#define MODB_READ           0x03
#define MODB_READ2          0x04
#define MODB_WRITE          0x06
#define MODB_WRITEMULT      0x10


/*
 * #define CRC_POLY_xxxx
 *
 * The constants of the form CRC_POLY_xxxx define the polynomials for some well
 * known CRC calculations.
 */

 #define		CRC_POLY_16		0xA001
 #define		CRC_POLY_32		0xEDB88320ul
 #define		CRC_POLY_64		0x42F0E1EBA9EA3693ull
 #define		CRC_POLY_CCITT		0x1021
 #define		CRC_POLY_DNP		0xA6BC
 #define		CRC_POLY_KERMIT		0x8408
 #define		CRC_POLY_SICK		0x8005
 
 /*
  * #define CRC_START_xxxx
  *
  * The constants of the form CRC_START_xxxx define the values that are used for
  * initialization of a CRC value for common used calculation methods.
  */
 
 #define		CRC_START_8		0x00
 #define		CRC_START_16		0x0000
 #define		CRC_START_MODBUS	0xFFFF
 #define		CRC_START_XMODEM	0x0000
 #define		CRC_START_CCITT_1D0F	0x1D0F
 #define		CRC_START_CCITT_FFFF	0xFFFF
 #define		CRC_START_KERMIT	0x0000
 #define		CRC_START_SICK		0x0000
 #define		CRC_START_DNP		0x0000
 #define		CRC_START_32		0xFFFFFFFFul
 #define		CRC_START_64_ECMA	0x0000000000000000ull
 #define		CRC_START_64_WE		0xFFFFFFFFFFFFFFFFull
 

uint16_t crc_modbus( const unsigned char *input_str, size_t num_bytes);


void modbus_read(uint16_t* modb_db, uint8_t* buf, uint8_t len);

void modbus_write(uint16_t* modb_db, uint8_t* buf, uint8_t len);

#endif
