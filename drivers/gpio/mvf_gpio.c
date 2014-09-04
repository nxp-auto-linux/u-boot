/*
 * Freescale VF610/SAC58R GPIO control code
 *
 * Copyright 2014 Freescale Semiconductor, Inc.
 * Gilles Talis, Freescale Semiconductors, Inc <gilles.talis@freescale.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
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
#include <asm/arch/imx-regs.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <errno.h>

enum mvf_gpio_direction {
	MVF_GPIO_DIRECTION_IN,
	MVF_GPIO_DIRECTION_OUT,
};

#define GPIO_TO_PORT(n)		(n / 32)

#define GPIO_BANK_OFFSET			0x40

#define PORT_PCR					0x00
#define	PORT_PCR_IRQC_BOTH_EDGES	(11<<16)
#define	PORT_PCR_IRQC_DISABLED		0

#define GPIO_PDOR					0x00
#define GPIO_PSOR					0x04
#define GPIO_PCOR					0x08
#define GPIO_PDIR					0x10


static unsigned long gpio_ports[] = {
	[0] = PORTA_BASE_ADDR,
	[1] = PORTB_BASE_ADDR,
	[2] = PORTC_BASE_ADDR,
	[3] = PORTD_BASE_ADDR,
	[4] = PORTE_BASE_ADDR,
	[5] = PORTF_BASE_ADDR,
	[6] = PORTG_BASE_ADDR,
	[7] = PORTH_BASE_ADDR,
	[8] = 0,
	[9] = PORTJ_BASE_ADDR,
	[10] = PORTK_BASE_ADDR,
	[11] = PORTL_BASE_ADDR,
};

static int mvf_gpio_direction(unsigned int gpio,
	enum mvf_gpio_direction direction)
{
	unsigned int port = GPIO_TO_PORT(gpio);

	if (port >= ARRAY_SIZE(gpio_ports))
		return -1;
	
	gpio &= 0x1f;

	if (direction == MVF_GPIO_DIRECTION_IN) {		
		writel( PORT_PCR_IRQC_BOTH_EDGES,
			gpio_ports[port] + (gpio * 4));
	}
	else {		
		writel( PORT_PCR_IRQC_DISABLED,
			gpio_ports[port] + (gpio * 4));
	}
	
	return 0;
}

int gpio_set_value(unsigned gpio, int value)
{
	unsigned int port = GPIO_TO_PORT(gpio);
	unsigned int bank;

	if (port >= ARRAY_SIZE(gpio_ports))
		return -1;

	bank = GPIO_BASE_ADDR + (port * GPIO_BANK_OFFSET);

	gpio &= 0x1f;

	if (value)
		writel( 1<<gpio, bank + GPIO_PSOR);
	else
		writel( 1<<gpio, bank + GPIO_PCOR);

	return 0;
}

int gpio_get_value(unsigned gpio)
{
	unsigned int port = GPIO_TO_PORT(gpio);
	unsigned int bank;
	
	u32 val;

	if (port >= ARRAY_SIZE(gpio_ports))
		return -1;

	bank = GPIO_BASE_ADDR + (port * GPIO_BANK_OFFSET);

	gpio &= 0x1f;

	val = readl(gpio_ports[port] + (gpio * 4));

	if (val & PORT_PCR_IRQC_BOTH_EDGES) {
		/* GPIO configured as input */
		val =(readl(bank + GPIO_PDIR) >> gpio) & 0x01;
	}
	else {
		/* GPIO configured as output */
		val =(readl(bank + GPIO_PDOR) >> gpio) & 0x01;		
	}
	
	return val;
}

int gpio_request(unsigned gpio, const char *label)
{
	unsigned int port = GPIO_TO_PORT(gpio);
	if (port >= ARRAY_SIZE(gpio_ports))
		return -1;
	return 0;
}

int gpio_free(unsigned gpio)
{
	return 0;
}

int gpio_direction_input(unsigned gpio)
{
	return mvf_gpio_direction(gpio, MVF_GPIO_DIRECTION_IN);
}

int gpio_direction_output(unsigned gpio, int value)
{
	int ret = mvf_gpio_direction(gpio, MVF_GPIO_DIRECTION_OUT);

	if (ret < 0)
		return ret;

	return gpio_set_value(gpio, value);
}
