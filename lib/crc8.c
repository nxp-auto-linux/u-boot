// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2013 Google, Inc
 */

#ifdef USE_HOSTCC
#include <arpa/inet.h>
#else
#include <common.h>
#endif
#include <u-boot/crc.h>

/* Default polynomial: x^8 + x^2 + x^1 + 1 */
#define DEFAULT_POLY 0x7
#define CRC8_POLY(P) ((0x1000U | ((P) << 4)) << 3)

static unsigned char _crc8(unsigned short data, unsigned short poly)
{
	int i;

	for (i = 0; i < 8; i++) {
		if (data & 0x8000)
			data = data ^ poly;
		data = data << 1;
	}

	return (unsigned char)(data >> 8);
}

unsigned int crc8poly(unsigned int crc, unsigned short poly,
		      const unsigned char *vptr, int len)
{
	int i;

	poly = CRC8_POLY(poly);
	for (i = 0; i < len; i++)
		crc = _crc8((crc ^ vptr[i]) << 8, poly);

	return crc;
}

unsigned int crc8(unsigned int crc, const unsigned char *vptr, int len)
{
	return crc8poly(crc, DEFAULT_POLY, vptr, len);
}
