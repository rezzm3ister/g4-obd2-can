#include "lib_modbus.h"

static void             init_crc16_tab( void );

static bool             crc_tab16_init          = false;
static uint16_t         crc_tab16[256];

static void init_crc16_tab( void ) {

	uint16_t i;
	uint16_t j;
	uint16_t crc;
	uint16_t c;

	for (i=0; i<256; i++) {

		crc = 0;
		c   = i;

		for (j=0; j<8; j++) {

			if ( (crc ^ c) & 0x0001 ) crc = ( crc >> 1 ) ^ CRC_POLY_16;
			else                      crc =   crc >> 1;

			c = c >> 1;
		}

		crc_tab16[i] = crc;
	}

	crc_tab16_init = true;

}  /* init_crc16_tab */

uint16_t crc_modbus( const unsigned char *input_str, size_t num_bytes ) {

	uint16_t crc;
	const unsigned char *ptr;
	size_t a;

	if ( ! crc_tab16_init ) init_crc16_tab();

	crc = CRC_START_MODBUS;
	ptr = input_str;

	if ( ptr != NULL ) for (a=0; a<num_bytes; a++) {

		crc = (crc >> 8) ^ crc_tab16[ (crc ^ (uint16_t) *ptr++) & 0x00FF ];
	}

	return crc;

}  /* crc_modbus */


void modbus_read(uint16_t* modb_db, uint8_t* buf, uint8_t len)
{
    uint8_t tmp;
    memcpy(buf,modb_db,len*2);
    for(int i=0;i<len*2;i+=2)
    {
        tmp=buf[i];
        buf[i]=buf[i+1];
        buf[i+1]=tmp;
    }
}

void modbus_write(uint16_t* modb_db, uint8_t* buf, uint8_t len)
{
    uint8_t tmp;
    uint8_t tbuf[300];
    memcpy(tbuf,buf,300);
    for(int i=0;i<len*2;i+=2)
    {
        tmp=tbuf[i];
        tbuf[i]=tbuf[i+1];
        tbuf[i+1]=tmp;
    }
    memcpy(modb_db,tbuf,len*2);
}