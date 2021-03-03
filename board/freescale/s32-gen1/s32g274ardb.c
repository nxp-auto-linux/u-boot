// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright 2018-2021 NXP
 */
#include <asm/arch/soc.h>
#include <board_common.h>
#include <common.h>
#include <s32g274a_common.h>

#define ADC_TIMEOUT_VALUE 0x1000000

#define SARADC_MCR		0x0
#define SARADC_MSR		0x4
#define SARADC_ISR		0x10
#define SARADC_CTR0		0x94
#define SARADC_NCMR0	0xA4
#define SARADC_PCDR(x)	(0x100 + (x) * 4)

#define SARADC_MCR_PWDN			BIT(0)
#define SARADC_MCR_ADCLKSE		BIT(8)
#define SARADC_MCR_TSAMP_MASK	(BIT(10) | BIT(9))
#define SARADC_MCR_AVGEN		BIT(13)
#define SARADC_MCR_CALSTART		BIT(14)
#define SARADC_MCR_NSTART		BIT(24)
#define SARADC_MCR_SCAN_MODE	BIT(29)
#define SARADC_MCR_WLSIDE		BIT(30)
#define SARADC_MCR_OWREN		BIT(31)

#define SARADC_MSR_CALBUSY		BIT(29)
#define SARADC_MSR_CALFAIL		BIT(30)

#define SARADC_ISR_ECH			BIT(0)

#define SARADC_CTR0_INPSAMP(x)	(x)

#define SARADC_NCMR0_CH5		BIT(5)

#define SARADC_PCDR_VALID		BIT(19)
#define SARADC_PCDR_CDATA(x)	((x) & 0xfff)

void setup_iomux_uart(void)
{
#if (CONFIG_FSL_LINFLEX_MODULE == 0)

	/* Muxing for linflex0 */
	setup_iomux_uart0_pc09_pc10();

#elif (CONFIG_FSL_LINFLEX_MODULE == 1)
	/* Muxing for linflex1 */

	/* set PA13 - MSCR[13] - for UART1 TXD */
	writel(SIUL2_MSCR_S32G_G1_PORT_CTRL_UART1_TXD,
	       SIUL2_0_MSCRn(SIUL2_PA13_MSCR_S32_G1_UART1));

	/* set PB00 - MSCR[16] - for UART1 RXD */
	writel(SIUL2_MSCR_S32G_G1_PORT_CTRL_UART_RXD,
	       SIUL2_0_MSCRn(SIUL2_PB00_MSCR_S32_G1_UART1));

	/* set PB00 - MSCR[736]/IMCR[224] - for UART1 RXD */
	writel(SIUL2_IMCR_S32G_G1_UART1_RXD_to_pad,
	       SIUL2_1_IMCRn(SIUL2_PB00_IMCR_S32_G1_UART1));
#else
#error "Unsupported UART pinmuxing configuration"
#endif
}

uint32_t read_ch5_adc_value(void)
{
	u32 result = (uint32_t)-1;
	u32 tmp;
	u32 timeout = ADC_TIMEOUT_VALUE;

	/* Start calibration */
	/* 1) Power down */
	tmp = readl(SARADC0_BASE_ADDR + SARADC_MCR) | SARADC_MCR_PWDN;
	writel(tmp, SARADC0_BASE_ADDR + SARADC_MCR);

	/* 2) Configure clock = bus clock /2 */
	tmp = readl(SARADC0_BASE_ADDR + SARADC_MCR) & ~SARADC_MCR_ADCLKSE;
	writel(tmp, SARADC0_BASE_ADDR + SARADC_MCR);

	/* 3) Power up */
	tmp = readl(SARADC0_BASE_ADDR + SARADC_MCR) & ~SARADC_MCR_PWDN;
	writel(tmp, SARADC0_BASE_ADDR + SARADC_MCR);

	/* 4) Set: avgen = 1, tsamp = 0, calstart = 1 */
	tmp = readl(SARADC0_BASE_ADDR + SARADC_MCR);
	tmp |= SARADC_MCR_AVGEN;
	tmp &= ~SARADC_MCR_TSAMP_MASK;
	tmp |= SARADC_MCR_CALSTART;
	writel(tmp, SARADC0_BASE_ADDR + SARADC_MCR);

	/* 4) Waiting for calibration done */
	do {
		tmp = readl(SARADC0_BASE_ADDR + SARADC_MSR);
	} while (tmp & SARADC_MSR_CALBUSY);

	if (tmp & SARADC_MSR_CALFAIL)
		goto power_down;

	/* Conversion CH5 */
	/* 1) Power down */
	tmp = readl(SARADC0_BASE_ADDR + SARADC_MCR) | SARADC_MCR_PWDN;
	writel(tmp, SARADC0_BASE_ADDR + SARADC_MCR);

	/* 2) Configure clock = 1 */
	tmp = readl(SARADC0_BASE_ADDR + SARADC_MCR) | SARADC_MCR_ADCLKSE;
	writel(tmp, SARADC0_BASE_ADDR + SARADC_MCR);

	writel(SARADC_CTR0_INPSAMP(0xFF), SARADC0_BASE_ADDR + SARADC_CTR0);

	/* 3) Power up */
	tmp = readl(SARADC0_BASE_ADDR + SARADC_MCR) & ~SARADC_MCR_PWDN;
	writel(tmp, SARADC0_BASE_ADDR + SARADC_MCR);

	/* 4) Enable CH5 */
	writel(SARADC_NCMR0_CH5, SARADC0_BASE_ADDR + SARADC_NCMR0);

	/* 5) Set: owren=1, wlside=0, mode=0, nstart=1 */
	tmp = readl(SARADC0_BASE_ADDR + SARADC_MCR);
	tmp |= SARADC_MCR_OWREN;
	tmp &= ~SARADC_MCR_WLSIDE;
	tmp &= ~SARADC_MCR_SCAN_MODE;
	tmp |= SARADC_MCR_NSTART;
	writel(tmp, SARADC0_BASE_ADDR + SARADC_MCR);

	while (--timeout) {
		if (!(readl(SARADC0_BASE_ADDR + SARADC_ISR) & SARADC_ISR_ECH))
			continue;

		/* clear status */
		writel(SARADC_ISR_ECH, SARADC0_BASE_ADDR + SARADC_ISR);

		/* wait channel data  */
		do {
			tmp = readl(SARADC0_BASE_ADDR + SARADC_PCDR(5));
		} while (!(tmp & SARADC_PCDR_VALID));

		result = SARADC_PCDR_CDATA(tmp);
		break;
	}

power_down:
	tmp = readl(SARADC0_BASE_ADDR + SARADC_MCR) | SARADC_MCR_PWDN;
	writel(tmp, SARADC0_BASE_ADDR + SARADC_MCR);
	return result;
}

static const struct {
	const char *rev;
	const char *desc;
	u32 lower;
	u32 upper;
} rdb_revisions[] = {
	{ /* 0V */
		.rev = "",
		.desc = "RDB",
		.lower = 0,
		.upper = 400,
	},
	{ /* 0.8v */
		.rev = "C",
		.desc = "RDB2/GLDBOX Revision C",
		.lower = 1820 - 200,
		.upper = 1820 + 200,
	},
	{ /* 1.0v */
		.rev = "D",
		.desc = "RDB2/GLDBOX Revision D",
		.lower = 2275 - 200,
		.upper = 2275 + 200,
	},
};

static int find_rdb_rev(uint32_t adc_value)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(rdb_revisions); i++) {
		if (rdb_revisions[i].lower <= adc_value &&
		    adc_value <= rdb_revisions[i].upper)
			return i;
	}
	return -1;
}

static int check_rdb_rev(uint32_t *adc_value)
{
	u32 adc_result;
	int rev_idx;
	u32 i;

	for (i = 0; i < 3; i++) {
		adc_result = read_ch5_adc_value();
		rev_idx = find_rdb_rev(adc_result);
		if (rev_idx != -1)
			break;
	}

	if (adc_value)
		*adc_value = adc_result;
	return rev_idx;
}

int board_late_init(void)
{
	int rev_idx;
	u32 adc_value;

	rev_idx = check_rdb_rev(&adc_value);
	if (rev_idx != -1) {
		env_set("board_rev", rdb_revisions[rev_idx].rev);
		switch (rdb_revisions[rev_idx].rev[0]) {
		case 'C':
			env_set("pfe1_phy_addr", "3");
			break;
		case 'D':
			env_set("pfe1_phy_addr", "8");
			break;
		default:
			env_set("pfe1_phy_addr", NULL);
			break;
		}
		printf("Board revision:\t%s\n", rdb_revisions[rev_idx].desc);
		return 0;
	}

	env_set("board_rev", NULL);
	env_set("pfe1_phy_addr", NULL);
	printf("Board revision:\tRevision Unknown: (0x%x)\n", adc_value);
	return 0;
}
