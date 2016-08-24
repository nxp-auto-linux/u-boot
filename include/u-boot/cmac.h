/*
 * (C) Copyright 2016 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Idenfifier:	GPL-2.0+
 */

#ifndef _CMAC_H
#define _CMAC_H

#if defined(CONFIG_FIT)

#include <errno.h>
#include <image.h>

#define CMAC_DIGEST_SIZE 16

#if IMAGE_ENABLE_SIGN && !defined (USE_HOSTCC)
/**
 * sign() - calculate and return signature for given input data
 *
 * @info:	Specifies key and FIT information
 * @data:	Pointer to the input data
 * @data_len:	Data length
 * @sigp:	Set to an allocated buffer holding the signature
 * @sig_len:	Set to length of the calculated hash
 *
 * This computes input data signature according to selected algorithm.
 * Resulting signature value is placed in an allocated buffer, the
 * pointer is returned as *sigp. The length of the calculated
 * signature is returned via the sig_len pointer argument. The caller
 * should free *sigp.
 *
 * @return: 0, on success, -ve on error
 */
int cmac_sign(struct image_sign_info *info,
	     const struct image_region region[],
	     int region_count, uint8_t **sigp, uint *sig_len);

/**
 * add_verify_data() - Add verification information to FDT
 *
 * Add public key information to the FDT node, suitable for
 * verification at run-time. The information added depends on the
 * algorithm being used.
 *
 * @info:	Specifies key and FIT information
 * @keydest:	Destination FDT blob for public key data
 * @return: 0, on success, -ENOSPC if the keydest FDT blob ran out of space,
		other -ve value on error
*/
int cmac_add_verify_data(struct image_sign_info *info, void *keydest);
#else
static inline int cmac_sign(struct image_sign_info *info,
		const struct image_region region[], int region_count,
		uint8_t **sigp, uint *sig_len)
{
	return -ENXIO;
}

static inline int cmac_add_verify_data(struct image_sign_info *info,
				      void *keydest)
{
	return -ENXIO;
}
#endif

#if IMAGE_ENABLE_VERIFY && !defined (USE_HOSTCC)
/**
 * rsa_verify() - Verify a signature against some data
 *
 * Verify a RSA PKCS1.5 signature against an expected hash.
 *
 * @info:	Specifies key and FIT information
 * @data:	Pointer to the input data
 * @data_len:	Data length
 * @sig:	Signature
 * @sig_len:	Number of bytes in signature
 * @return 0 if verified, -ve on error
 */
int cmac_verify(struct image_sign_info *info,
	       const struct image_region region[], int region_count,
	       uint8_t *sig, uint sig_len);
#else
static inline int cmac_verify(struct image_sign_info *info,
		const struct image_region region[], int region_count,
		uint8_t *sig, uint sig_len)
{
	return -ENXIO;
}
#endif

#endif
#endif /* defined(CONFIG_FIT) */
