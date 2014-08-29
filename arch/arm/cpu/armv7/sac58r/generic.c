/*
 * Copyright 2013 Freescale Semiconductor, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/clock.h>
#include <asm/arch/crm_regs.h>
#include <netdev.h>
#include <div64.h>
#ifdef CONFIG_FSL_ESDHC
#include <fsl_esdhc.h>
#endif

#ifdef CONFIG_FSL_ESDHC
DECLARE_GLOBAL_DATA_PTR;
#endif

void enable_periph_clk(u32 aips_num, u32 periph_number)
{
	u32 periph_num, reg_addr, bit_pos;
	struct gpc_reg *gpc = (struct gpc_reg *)GPC_BASE_ADDR;

	if (periph_number >= 0 && periph_number < 32) {
		printf("Incorrect peripheral number:%d - expecting periph number between 32 -127\n",periph_number);
	}
	else {
		periph_num = (periph_number - 32);
		reg_addr=periph_num/16;
		bit_pos=periph_num%16;
		bit_pos=bit_pos*2;
		switch(aips_num){
		case AIPS0:
			setbits_le32((&gpc->aips0_offpf_pctl0) + reg_addr, 0x3<<bit_pos);
			break;
		case AIPS1:
			setbits_le32((&gpc->aips1_offpf_pctl0) + reg_addr, 0x3<<bit_pos);
			break;
		case AIPS2:
			setbits_le32((&gpc->aips2_offpf_pctl0) + reg_addr, 0x3<<bit_pos);
			break;
		default:
			printf("wrong aips_id:%d\n",aips_num);
		}
	}
}

void disable_periph_clk(u32 aips_num, u32 periph_number)
{
	u32 periph_num, reg_addr, bit_pos;
	struct gpc_reg *gpc = (struct gpc_reg *)GPC_BASE_ADDR;

	if (periph_number >= 0 && periph_number < 32) {
		printf("Incorrect peripheral number:%d - expecting periph number between 32 -127\n",periph_number);
	}
	else {
		periph_num = (periph_number - 32);
		reg_addr=periph_num/16;
		bit_pos=periph_num%16;
		bit_pos=bit_pos*2;
		switch(aips_num){
		case AIPS0:
			clrbits_le32((&gpc->aips0_offpf_pctl0) + reg_addr, 0x3<<bit_pos);
			break;
		case AIPS1:
			clrbits_le32((&gpc->aips1_offpf_pctl0) + reg_addr, 0x3<<bit_pos);
			break;
		case AIPS2:
			clrbits_le32((&gpc->aips2_offpf_pctl0) + reg_addr, 0x3<<bit_pos);
			break;
		default:
			printf("wrong aips_id:%d\n",aips_num);
		}
	}
}

#ifdef CONFIG_MXC_OCOTP
void enable_ocotp_clk(unsigned char enable)
{
	if (enable) {
		enable_periph_clk(0,AIPS0_OFF_OCOTP0);
		enable_periph_clk(0,AIPS0_OFF_OCOTP1);
	} else {
		disable_periph_clk(0,AIPS0_OFF_OCOTP0);
		disable_periph_clk(0,AIPS0_OFF_OCOTP1);
	}
}
#endif


/* PLL frequency decoding for pll_pfd in KHz                                                  */
/* Only works for PLL_ARM, PLL_SYS, PLL_USB0, PLL_USB1, PLL_AUDIO0, PLL_AUDIO1, PLL_VIDEO     */
/* PLL_ENET not implemented                                                                   */
static u32 decode_pll(enum pll_clocks pll, u32 infreq, u8 pfd)
{
	u32 freq_main=0;
	u32 mfi=0, pfd_frac=0;
	u32 pll_ctrl, pll_num , pll_denom, pll_pfd;

	struct anadig_reg *anadig = (struct anadig_reg *)ANADIG_BASE_ADDR;

	switch (pll) {
	case PLL_ARM: /* PLL1 */
		pll_ctrl = readl(&anadig->pll1_ctrl);
		pll_num = readl(&anadig->pll1_num);
		pll_denom = readl(&anadig->pll1_denom);

		mfi = pll_ctrl & ANADIG_PLL1_CTRL_DIV_SELECT_MASK;
		return lldiv( (u64)infreq * ((u64)mfi*(u64)pll_denom + (u64)pll_num), pll_denom);
	case PLL_SYS: /* PLL2 */
		pll_ctrl = readl(&anadig->pll2_ctrl);
		pll_num = readl(&anadig->pll2_num);
		pll_denom = readl(&anadig->pll2_denom);
		pll_pfd = readl(&anadig->pll2_pfd);

		mfi = pll_ctrl & ANADIG_PLL2_CTRL_DIV_SELECT_MASK; /* bit 0*/
		if (mfi == 0)
			freq_main = infreq * 20; /* 480 Mhz */
		else
			freq_main = infreq * 22; /* 528 Mhz */
		switch(pfd){
		case 0:
			return freq_main;
		case 1:
			pfd_frac = (pll_pfd & ANADIG_PLL_PFD1_FRAC_MASK) >> ANADIG_PLL_PFD1_OFFSET;
			return freq_main*18/pfd_frac;
		case 2:
			pfd_frac = (pll_pfd & ANADIG_PLL_PFD2_FRAC_MASK) >> ANADIG_PLL_PFD2_OFFSET;
			return freq_main*18/pfd_frac;
		case 3:
			pfd_frac = (pll_pfd & ANADIG_PLL_PFD3_FRAC_MASK) >> ANADIG_PLL_PFD3_OFFSET;
			return freq_main*18/pfd_frac;
		case 4:
			pfd_frac = (pll_pfd & ANADIG_PLL_PFD4_FRAC_MASK) >> ANADIG_PLL_PFD4_OFFSET;
			return freq_main*18/pfd_frac;
		default:
			printf("pfd id not supported:%d\n",pfd);
			return -1;
		} /* switch(pfd) */
	case PLL_USBOTG0: /* PLL3 */
		pll_ctrl = readl(&anadig->pll3_ctrl);
		pll_pfd = readl(&anadig->pll3_pfd);

		mfi = (pll_ctrl & ANADIG_PLL3_CTRL_DIV_SELECT_MASK); /*bit 0*/
		if (mfi == 0)
			freq_main = infreq * 20; /* 480 Mhz */
		else
			freq_main = infreq * 22; /* 528 Mhz */

		switch(pfd){
		case 0:
			return freq_main;
		case 1:
			pfd_frac = (pll_pfd & ANADIG_PLL_PFD1_FRAC_MASK) >> ANADIG_PLL_PFD1_OFFSET;
			return freq_main*18/pfd_frac;
		case 2:
			pfd_frac = (pll_pfd & ANADIG_PLL_PFD2_FRAC_MASK) >> ANADIG_PLL_PFD2_OFFSET;
			return freq_main*18/pfd_frac;
		case 3:
			pfd_frac = (pll_pfd & ANADIG_PLL_PFD3_FRAC_MASK) >> ANADIG_PLL_PFD3_OFFSET;
			return freq_main*18/pfd_frac;
		case 4:
			pfd_frac = (pll_pfd & ANADIG_PLL_PFD4_FRAC_MASK) >> ANADIG_PLL_PFD4_OFFSET;
			return freq_main*18/pfd_frac;
		default:
			printf("pfd id not supported:%d\n",pfd);
			return -1;
		} /* switch(pfd) */
	case PLL_USBOTG1: /* PLL7 */
		pll_ctrl = readl(&anadig->pll7_ctrl);

		mfi = pll_ctrl & ANADIG_PLL7_CTRL_DIV_SELECT_MASK; /*bit 0 */

		if (mfi == 0)
			freq_main = infreq * 20; /* 480 Mhz */
		else
			freq_main = infreq * 22; /* 528 Mhz */
		return freq_main;
	case PLL_AUDIO0: /* PLL4 */
		pll_ctrl = readl(&anadig->pll4_ctrl);
		pll_num = (readl(&anadig->pll4_num) & ANADIG_PLL_NUM_MASK);
		pll_denom = (readl(&anadig->pll4_denom) & ANADIG_PLL_DENOM_MASK);
		
		mfi = pll_ctrl & ANADIG_PLL4_CTRL_DIV_SELECT_MASK;

		return lldiv( (u64)infreq * ((u64)mfi*(u64)pll_denom + (u64)pll_num), pll_denom);
	case PLL_AUDIO1: /* PLL8 */
		pll_ctrl = readl(&anadig->pll8_ctrl);
		pll_num = (readl(&anadig->pll8_num) & ANADIG_PLL_NUM_MASK);
		pll_denom = (readl(&anadig->pll8_denom) & ANADIG_PLL_DENOM_MASK);

		mfi = pll_ctrl & ANADIG_PLL8_CTRL_DIV_SELECT_MASK;

		return lldiv( (u64)infreq * ((u64)mfi*(u64)pll_denom + (u64)pll_num), pll_denom);
	case PLL_VIDEO: /* PLL6 */
		pll_ctrl = readl(&anadig->pll6_ctrl);
		pll_num = (readl(&anadig->pll6_num) & ANADIG_PLL_NUM_MASK);
		pll_denom = (readl(&anadig->pll6_denom) & ANADIG_PLL_DENOM_MASK);

		mfi = pll_ctrl & ANADIG_PLL6_CTRL_DIV_SELECT_MASK;

		return lldiv( (u64)infreq * ((u64)mfi*(u64)pll_denom + (u64)pll_num), pll_denom);
	case PLL_ENET: /*PLL5 */
		printf("PLL decode not supported for PLL_ENET\n");
		return -1;
	default:
		printf("not able to decode the PLL frequency - PLL ID doesn't exist\n");
		return -1;
	} /* switch(pll) */
}

/* return ARM A7 clock frequency in Hz                         */
static u32 get_mcu_main_clk(void)
{
	struct ccm_reg *ccm = (struct ccm_reg *)CCM_BASE_ADDR;
	u32 ccm_a7_clk, armclk_div;
	u32 a7clk_sel;
	u32 freq = 0;

	ccm_a7_clk = readl(&ccm->a7_clk);
	a7clk_sel = ccm_a7_clk & CCM_MUX4_CTL_MASK;
	armclk_div = ((ccm_a7_clk & CCM_PREDIV3_CTRL_MASK) >> CCM_PREDIV_CTRL_OFFSET)+1;

	switch (a7clk_sel) {
	case 1:
		freq = FIRC_CLK_FREQ;
		break;
	case 2:
		freq = FAST_CLK_FREQ;
		break;
	case 3:
		freq = decode_pll(PLL_ARM,24000,0); /* arm_pll_clk */
		break;
	case 4:
		freq = decode_pll(PLL_SYS,24000,0); /* sys_pll_main */
		break;
	case 5:
		freq = decode_pll(PLL_SYS,24000,1); /* sys_pll_pfd0 */
		break;
	case 6:
		freq = decode_pll(PLL_SYS,24000,2); /* sys_pll_pfd1 */
		break;
	case 7:
		freq = decode_pll(PLL_SYS,24000,3); /* sys_pll_pfd2 */
		break;
	case 8:
		freq = decode_pll(PLL_SYS,24000,4); /* sys_pll_pfd3 */
		break;
	case 9:
		freq = decode_pll(PLL_USBOTG0,24000,0); /* usb0_pll_main */
		break;
	case 10:
		freq = decode_pll(PLL_USBOTG0,24000,1); /* usb0_pll_pfd0 */
		break;
	case 11:
		freq = decode_pll(PLL_ARM,24000,0)/2; /* arm_pll_clk/2 */
		break;
	default:
		printf("unsupported arm clock select\n");
		return -1;
	}

	return ((freq * 1000) / armclk_div);
}

/* return the qos301, DDR or bus clock  frequency in Hz depending on id */
/* id==0 => get the qos301_clk                                          */
/* id==1 => get the bus_clk                                             */
/* id==2 => get the DDR_clk                                             */
static u32 get_bus_clk(u8 id)
{
	struct ccm_reg *ccm = (struct ccm_reg *)CCM_BASE_ADDR;
	u32 ccm_qos_ddr_clk, ccm_qos_clk, ccm_bus_clk, qosclk_div, busclk_div;
	u32 qosclk_sel;
	u32 freq = 0;

	ccm_qos_ddr_clk = readl(&ccm->QoS_DDR_root);
	qosclk_sel = ccm_qos_ddr_clk & CCM_MUX4_CTL_MASK;
	ccm_qos_clk = readl(&ccm->QoS301_clk);
	qosclk_div = ((ccm_qos_clk & CCM_PREDIV3_CTRL_MASK) >> CCM_PREDIV_CTRL_OFFSET)+1;
	ccm_bus_clk = readl(&ccm->BUS_clk);
	busclk_div = ((ccm_bus_clk & CCM_PREDIV3_CTRL_MASK) >> CCM_PREDIV_CTRL_OFFSET)+1;

	switch (qosclk_sel) {
	case 1:
		freq = FIRC_CLK_FREQ;
		break;
	case 2:
		freq = FAST_CLK_FREQ;
		break;
	case 3:
		freq = decode_pll(PLL_ARM,24000,0); /* arm_pll_clk */
		break;
	case 4:
		freq = decode_pll(PLL_SYS,24000,0); /* sys_pll_main */
		break;
	case 5:
		freq = decode_pll(PLL_SYS,24000,1); /* sys_pll_pfd0 */
		break;
	case 6:
		freq = decode_pll(PLL_SYS,24000,2); /* sys_pll_pfd1 */
		break;
	case 7:
		freq = decode_pll(PLL_SYS,24000,3); /* sys_pll_pfd2 */
		break;
	case 8:
		freq = decode_pll(PLL_SYS,24000,4); /* sys_pll_pfd3 */
		break;
	case 9:
		freq = decode_pll(PLL_USBOTG0,24000,0); /* usb0_pll_main */
		break;
	case 10:
		freq = decode_pll(PLL_USBOTG0,24000,1); /* usb0_pll_pfd0 */
		break;
	case 11:
		freq = decode_pll(PLL_ARM,24000,0)/2; /* arm_pll_clk/2 */
		break;
	default:
		printf("unsupported qos301 clock select\n");
		return -1;
	}

	switch(id) {
	case 0:
		return ((freq*1000) / qosclk_div); /* qos301_clk */
	case 1:
		return ((freq*1000) / qosclk_div / busclk_div); /* bus_clk */
	case 2:
		return ((freq*1000) / qosclk_div); /* DDR_clk */
	default:
		printf("unsupported id:%d for get_bus_clk()\n",id);
		return -1;
	}
}

/* return ipg_clk frequency in Hz                         */
static u32 get_ipg_clk(void)
{
	struct ccm_reg *ccm = (struct ccm_reg *)CCM_BASE_ADDR;
	u32 ccm_per_clk, ipgclk_div;

	ccm_per_clk = readl(&ccm->PER_clk);
	ipgclk_div = ((ccm_per_clk & CCM_PREDIV3_CTRL_MASK) >> CCM_PREDIV_CTRL_OFFSET)+1;

	return (get_bus_clk(1) / ipgclk_div);
}

static u32 get_uart_clk(void)
{
	return get_ipg_clk();
}

/* return the sdhc clock frequency for the SDHC(id) controller with id=[0..2] in Hz */
static u32 get_sdhc_clk(u8 id)
{
	struct ccm_reg *ccm = (struct ccm_reg *)CCM_BASE_ADDR;
	u32 ccm_sdhc_clk, sdhcclk_sel, sdhcclk_div, sdhcclk_fracdiv;
	u32 freq = 0;

	switch (id) {
	case 0:
	ccm_sdhc_clk = readl(&ccm->uSDHC0_perclk);
		break;
	case 1:
	ccm_sdhc_clk = readl(&ccm->uSDHC1_perclk);
		break;
	case 2:
	ccm_sdhc_clk = readl(&ccm->uSDHC2_perclk);
		break;
	default:
		printf("uSDHC not defined\n");
		return -1;
	}

	sdhcclk_sel = ccm_sdhc_clk & CCM_MUX3_CTL_MASK;
	sdhcclk_div = ((ccm_sdhc_clk & CCM_PREDIV3_CTRL_MASK) >> CCM_PREDIV_CTRL_OFFSET)+1;
	sdhcclk_fracdiv = ((ccm_sdhc_clk & CCM_PREDIV_FRAC_DIV_CTRL_MASK) >>CCM_PREDIV_FRAC_DIV_CTRL_OFFSET)+1;
	
	switch (sdhcclk_sel) {
	case 0:
		freq = decode_pll(PLL_ARM,24000,0); /* arm_pll_clk */
		break;
	case 1:
		freq = decode_pll(PLL_SYS,24000,0); /* sys_pll_clk */
		break;
	case 2:
		freq = decode_pll(PLL_SYS,24000,2); /* sys_pll_pfd1 */
		break;
	case 3:
		freq = decode_pll(PLL_USBOTG0,24000,0); /* usb0_pll_main */
		break;
	case 4:
		freq = decode_pll(PLL_USBOTG0,24000,1); /* usb0_pll_pfd0 */
		break;
	case 5:
		freq = decode_pll(PLL_ARM,24000,0)/2; /* arm_pll_clk/2 */
		break;
	case 6:
		freq = get_bus_clk(0)/1000; /* QoS301_clk */
		break;
	default:
		printf("unsupported sdhc clock select\n");
		return -1;
	}
	return ((freq*1000) / sdhcclk_div / sdhcclk_fracdiv);
}

/* return ENET_clk in Hz                         */
u32 get_fec_clk(void)
{
#if 0
	struct ccm_reg *ccm = (struct ccm_reg *)CCM_BASE_ADDR;
	u32 ccm_enet_rmii_clk, rmii_clk_sel, enetrmii_div;
	u32 freq = 0;

	ccm_enet_rmii_clk = readl(&ccm->enet_rmii_clk);
	rmii_clk_sel = ccm_enet_rmii_clk & CCM_MUX2_CTL_MASK;

	enetrmii_div = ((ccm_enet_rmii_clk & CCM_PREDIV2_CTRL_MASK) >> CCM_PREDIV_CTRL_OFFSET)+1;

	switch (rmii_clk_sel) {
	case 0:
		freq = DFT_TEST_CLK_FREQ;
		break;
	case 1:
		freq = PLL8_MAIN_FREQ;
		break;
	case 2:
		freq = ENET_EXTERNAL_CLK;
		break;
	default:
		printf("unsupported enet clock select\n");
	}

	return (freq / enetrmii_div);
#endif
	return 0;
}

/* return I2C_clk frequency in Hz                         */
static u32 get_i2c_clk(void)
{
	return get_ipg_clk();
}

/* return NFC_clk frequency in Hz                         */
static u32 get_nfc_clk(void)
{
	struct ccm_reg *ccm = (struct ccm_reg *)CCM_BASE_ADDR;
	u32 freq, ccm_nfc_clk, nfc_clk_sel, nfcclk_div, nfcclk_fracdiv;

	ccm_nfc_clk = readl(&ccm->nfc_flash_clk_div);
	nfc_clk_sel = ccm_nfc_clk & CCM_MUX3_CTL_MASK;
	nfcclk_div = ((ccm_nfc_clk & CCM_PREDIV4_CTRL_MASK) >> CCM_PREDIV_CTRL_OFFSET)+1;
	nfcclk_fracdiv = ((ccm_nfc_clk & CCM_PREDIV_FRAC_DIV_CTRL_MASK) >> CCM_PREDIV_FRAC_DIV_CTRL_OFFSET)+1;

	switch(nfc_clk_sel)
	{
	case 0:
		freq = decode_pll(PLL_SYS,24000,0); /* sys_pll_clk */
		break;
	case 1:
		freq = decode_pll(PLL_SYS,24000,2); /* sys_pll_pfd1 */
		break;
	case 2:
		freq = decode_pll(PLL_SYS,24000,3); /* sys_pll_pfd2 */
		break;
	case 3:
		freq = decode_pll(PLL_SYS,24000,4); /* sys_pll_pfd3 */
		break;
	case 4:
		freq = decode_pll(PLL_USBOTG0,24000,0); /* usb0_pll_main */
		break;
	case 5:
		freq = decode_pll(PLL_USBOTG0,24000,3); /* usb0_pll_pfd2 */
		break;
	case 6:
		freq = decode_pll(PLL_USBOTG0,24000,4); /* usb0_pll_pfd3 */
		break;
	case 7:
		freq = get_bus_clk(0)/1000; /* QoS301_clk */
		break;
	} /* switch(nfc_clk_sel) */
	return ((freq*1000)/nfcclk_fracdiv/nfcclk_div);
}

unsigned int mxc_get_clock(enum mxc_clock clk)
{
	switch (clk) {
	case MXC_ARM_CLK:
		return get_mcu_main_clk();
	case MXC_BUS_CLK:
		return get_bus_clk(1);
	case MXC_IPG_CLK:
		return get_ipg_clk();
	case MXC_UART_CLK:
		return get_uart_clk();
	case MXC_USDHC0_CLK:
		return get_sdhc_clk(0);
	case MXC_USDHC1_CLK:
		return get_sdhc_clk(1);
	case MXC_USDHC2_CLK:
		return get_sdhc_clk(2);
	case MXC_FEC_CLK:
		return get_fec_clk();
	case MXC_I2C_CLK:
		return get_i2c_clk();
	case MXC_DDR_CLK:
		return get_bus_clk(2);
	case MXC_NFC_CLK:
		return get_nfc_clk();
	default:
		printf("Not able to get the clock freq - unsupported clk id:%d\n",clk);
		return -1;
	}
}	

/* Dump some core clocks */
int do_sac58r_showclocks(cmd_tbl_t *cmdtp, int flag, int argc,
			 char * const argv[])
{
	printf("\n");
	printf("-------------------------------------------------------------------------------------------------------\n");
	printf("DPLLs settings:\n");
	printf("PLL_ARM  main: %5d MHz\n", decode_pll(PLL_ARM, 24000,0)/1000);
	printf("PLL_SYS  main: %5d MHz - PFD1:%5d MHz - PFD2:%5d MHz - PFD3:%5d MHz - PFD4:%5d MHz\n", decode_pll(PLL_SYS, 24000,0)/1000,
		decode_pll(PLL_SYS, 24000,1)/1000, decode_pll(PLL_SYS, 24000,2)/1000, decode_pll(PLL_SYS, 24000,3)/1000, decode_pll(PLL_SYS, 24000,4)/1000);
	printf("PLL_USB0 main: %5d MHz - PFD1:%5d MHz - PFD2:%5d MHz - PFD3:%5d MHz - PFD4:%5d MHz\n", decode_pll(PLL_USBOTG0, 24000,0)/1000,
		decode_pll(PLL_USBOTG0, 24000,1)/1000, decode_pll(PLL_USBOTG0, 24000,2)/1000, decode_pll(PLL_USBOTG0, 24000,3)/1000, decode_pll(PLL_USBOTG0, 24000,4)/1000);
	printf("PLL_USB1 main: %5d MHz\n", decode_pll(PLL_USBOTG1, 24000,0)/1000);
	printf("PLL_AUD0 main: %5d MHz\n", decode_pll(PLL_AUDIO0, 24000,0)/1000);
	printf("PLL_AUD1 main: %5d MHz\n", decode_pll(PLL_AUDIO1, 24000,0)/1000);
	printf("PLL_VID  main: %5d MHz\n", decode_pll(PLL_VIDEO, 24000,0)/1000);
	printf("-------------------------------------------------------------------------------------------------------\n");
	printf("Root clocks:\n");
	printf("CPU A7 clock:  %5d MHz\n", mxc_get_clock(MXC_ARM_CLK) / 1000000);
	printf("BUS clock:     %5d MHz\n", mxc_get_clock(MXC_BUS_CLK) / 1000000);
	printf("IPG clock:     %5d MHz\n", mxc_get_clock(MXC_IPG_CLK) / 1000000);
	printf("DDR clock:     %5d MHz\n", mxc_get_clock(MXC_DDR_CLK) / 1000000);
	printf("uSDHC0 clock:  %5d MHz\n", mxc_get_clock(MXC_USDHC0_CLK) / 1000000);
	printf("uSDHC1 clock:  %5d MHz\n", mxc_get_clock(MXC_USDHC1_CLK) / 1000000);
	printf("uSDHC2 clock:  %5d MHz\n", mxc_get_clock(MXC_USDHC2_CLK) / 1000000);
	printf("ENET clock:    %5d MHz\n", mxc_get_clock(MXC_FEC_CLK) / 1000000);
	printf("UART clock:    %5d MHz\n", mxc_get_clock(MXC_UART_CLK) / 1000000);
	printf("I2C clock:     %5d MHz\n", mxc_get_clock(MXC_I2C_CLK) / 1000000);
	printf("NFC clock:     %5d MHz\n", mxc_get_clock(MXC_NFC_CLK) /1000000);
	return 0;
}

U_BOOT_CMD(
	clocks, CONFIG_SYS_MAXARGS, 1, do_sac58r_showclocks,
	"display clocks",
	""
);

#ifdef CONFIG_FEC_MXC
void imx_get_mac_from_fuse(int dev_id, unsigned char *mac)
{
	struct ocotp_regs *ocotp = (struct ocotp_regs *)OCOTP_BASE_ADDR;
	struct fuse_bank *bank = &ocotp->bank[4];
	struct fuse_bank4_regs *fuse =
		(struct fuse_bank4_regs *)bank->fuse_regs;

	u32 value = readl(&fuse->mac_addr0);
	mac[0] = (value >> 8);
	mac[1] = value;

	value = readl(&fuse->mac_addr1);
	mac[2] = value >> 24;
	mac[3] = value >> 16;
	mac[4] = value >> 8;
	mac[5] = value;
}
#endif

#if defined(CONFIG_DISPLAY_CPUINFO)


static const char* sac58r_reset_cause[] =
{
	"POR reset",
	"watchdog0 timer",
	"unknown",
	"watchdog2",
	"watchdog1",
	"JTAG HIGH-Z",
	"unknown",
	"external reset",
	"unknown",
	"HP LVD",
	"ULP LVD",
	"unknown",
	"LP_LVD",
	"unknown",
	"unknown",
	"unknown",
	"MDM SYS RST",
	"SNVS hard reset",
	"SW reset",
	"SNVS WDOG",
	"LVD memory brownout",
	"ULPVDD HVD",
	"LPVDD HVD",
	"HPVDD HVD",
	"1.1V unstable",
	"2.5V unstable",
	"3.0V unstable",
	"FOSC freq < 40 MHz",
	"BUS freq out of range",
	"No clock on FOSC",
	"No clock on SOSC",
	"CM4 in lockup",
};

#define RESET_CAUSE_POR			(1<<0)
#define RESET_CAUSE_EXTERNAL	(1<<7)
#define RESET_CAUSE_SOFTWARE	(1<<18)
#define RESET_CAUSE_WDOG		(1<<1)
#define SRC_SCR_SW_RST			(1<<12)

static void print_reset_cause(void)
{
	u32 cause;
	int i;

	struct src *src_regs = (struct src *)SRC_BASE_ADDR;

	cause = readl(&src_regs->srsr);
	writel(cause, &src_regs->srsr);


	/* Reset cause register returns a lot of information that
		are not needed in most reset cases:
		- watchdog
		- POR
		- external reset
		- software reset
		The routine catches the most common causes of reset
		and ignore the others.
		If these common cases are not detected, we then
		print all reset cases
	*/

	if (cause & RESET_CAUSE_WDOG) {
		printf ("Reset cause (0x%08x): %s\n", cause, sac58r_reset_cause[1]);
		return;
	}


	if (cause & RESET_CAUSE_POR) {
		printf ("Reset cause (0x%08x): %s\n", cause, sac58r_reset_cause[0]);
		return;
	}

	if (cause & RESET_CAUSE_EXTERNAL) {
		printf ("Reset cause (0x%08x): %s\n", cause, sac58r_reset_cause[7]);
		return;
	}

	if (cause & RESET_CAUSE_SOFTWARE) {
		printf ("Reset cause (0x%08x): %s\n", cause, sac58r_reset_cause[18]);
		return;
	}

	printf("Reset cause (0x%08x): \n", cause);
	i = 0;
	while (cause != 0) {
		if ((cause & 0x1) == 1) {
			if (strcmp(sac58r_reset_cause[i], "unknown")) {
				printf("- %s\n", sac58r_reset_cause[i]);
				}
			}
		cause = cause >> 1;
		i++;
	}
}

void reset_cpu(ulong addr)
{
	struct src *src_regs = (struct src *)SRC_BASE_ADDR;

	/* Generate a SW reset from SRC SCR register */
	writel(SRC_SCR_SW_RST, &src_regs->scr);

	/* If we get there, we are not in good shape */
	mdelay(1000);
	printf("FATAL: Reset Failed!\n");
	hang();
};

int print_cpuinfo(void)
{
	printf("CPU:   Freescale SAC58R at %d MHz\n",
		mxc_get_clock(MXC_ARM_CLK) / 1000000);
	print_reset_cause();

	return 0;
}
#endif

int cpu_eth_init(bd_t *bis)
{
	int rc = -ENODEV;

#if defined(CONFIG_FEC_MXC)
	rc = fecmxc_initialize(bis);
#endif

	return rc;
}

#ifdef CONFIG_FSL_ESDHC
int cpu_mmc_init(bd_t *bis)
{
	return fsl_esdhc_mmc_init(bis);
}
#endif

int get_clocks(void)
{
#ifdef CONFIG_FSL_ESDHC
#ifdef CONFIG_FSL_USDHC
#if CONFIG_SYS_FSL_ESDHC_ADDR == USDHC0_BASE_ADDR
	gd->arch.sdhc_clk = mxc_get_clock(MXC_USDHC0_CLK);
#elif CONFIG_SYS_FSL_ESDHC_ADDR == USDHC1_BASE_ADDR
	gd->arch.sdhc_clk = mxc_get_clock(MXC_USDHC1_CLK);
#elif CONFIG_SYS_FSL_ESDHC_ADDR == USDHC1_BASE_ADDR
	gd->arch.sdhc_clk = mxc_get_clock(MXC_USDHC2_CLK);
#endif /* #if CONFIG_SYS_FSL_ESDHC_ADDR */
#endif /* #ifdef CONFIG_FSL_USDHC */
#endif /* #ifdef CONFIG_FSL_ESDHC */
	return 0;
}

#ifndef CONFIG_SYS_DCACHE_OFF
void enable_caches(void)
{
	/* Enable D-cache. I-cache is already enabled in start.S */
	dcache_enable();
}
#endif
