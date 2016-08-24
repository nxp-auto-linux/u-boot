/*
 * (C) Copyright 2016 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Idenfifier:	GPL-2.0+
 */

#ifndef USE_HOSTCC
#include <common.h>
#include <asm/arch/cse.h>
#include <image.h>
#include <u-boot/cmac.h>


#if IMAGE_ENABLE_SIGN
int cse_genmac(ulong start_addr, unsigned long len, unsigned long key_id,
		uint8_t mac[]);

int cmac_sign(struct image_sign_info *info,
	     const struct image_region region[],
	     int region_count, uint8_t **sigp, uint *sig_len)
{
	if (region_count != 1) {
		printf("CMAC cannot be used with multiple memory regions.\n");
		return -EINVAL;
	}

	*sig_len = CMAC_DIGEST_SIZE;
	*sigp = malloc(*sig_len);
	if (!(*sigp))
		return -ENOMEM;

	return cse_genmac((ulong)region[0].data, region[0].size,
			SECURE_BOOT_KEY_ID, *sigp);
}
#endif


#if IMAGE_ENABLE_VERIFY
int cse_auth(ulong start_addr, unsigned long len, int key_id,
					uint8_t *exp_mac);

int cmac_verify(struct image_sign_info *info,
		const struct image_region region[], int region_count,
		uint8_t *sig, uint sig_len)
{
	/* CSE cannot manage multiple memory regions */
	if (region_count != 1) {
		printf("CMAC cannot be used with multiple memory regions.\n");
		return -EINVAL;
	}

	return cse_auth((ulong)region[0].data, region[0].size,
		SECURE_BOOT_KEY_ID, sig);
}

#endif

#endif /* USE_HOSTCC */
