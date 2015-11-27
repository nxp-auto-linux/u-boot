#
# Copyright 2013-2015 Freescale Semiconductor, Inc.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of
# the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston,
# MA 02111-1307 USA
#

obj-y   += generic.o
obj-y	+= lowlevel.o
obj-y	+= mp.o
obj-y   += cpu.o
obj-y   += cse.o
obj-$(CONFIG_SYS_PIT_TIMER)	+= timer.o
obj-$(CONFIG_S32V234_LPDDR2)	+= lpddr2.o
obj-$(CONFIG_S32V234_DDR3)	+= ddr3.o
obj-$(CONFIG_MP) += mp.o
obj-$(CONFIG_OF_LIBFDT) += fdt.o
