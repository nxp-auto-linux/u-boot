/*
 * (C) Copyright 2013-2015 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __comp_h__
#define __comp_h__ 1

#ifdef __cplusplus
extern "C" {
#endif


#define _STRINGIFY(x) #x

#define _ASM_NOP()    asm("nop")
#define _ASM_STOP(x)  asm("stop #(" _STRINGIFY(x) ")")
#define _ASM_WFI()    asm("wfi")

#define MRC(coproc, opcode1, Rt, CRn, CRm, opcode2) \
asm volatile ("mrc " _STRINGIFY(p##coproc) " , " _STRINGIFY(opcode1) \
" , %[output] , " _STRINGIFY(c##CRn) " , " _STRINGIFY(c##CRm) " , " \
_STRINGIFY(opcode2) : [output] "=r" (Rt) : )

#define MCR(coproc, opcode1, Rt, CRn, CRm, opcode2) \
asm volatile ("mcr " _STRINGIFY(p##coproc) " , " _STRINGIFY(opcode1) \
" , %[input] , " _STRINGIFY(c##CRn) " , " _STRINGIFY(c##CRm) " , " \
_STRINGIFY(opcode2) : : [input] "r" (Rt))

#define DSB()   asm("dsb")
#define ISB()   asm("isb")

#define _PSP_SYNC()     _ASM_NOP()

#define _PSP_GET_CPSR(x)        (x = __get_CPSR())

#define _PSP_SET_ENABLE_SR(x)   { ICCPMR = GIC_PMR_PRIORITY(x); }
#define _PSP_SET_DISABLE_SR(x)  _PSP_SET_ENABLE_SR(x)

#define _WEAK_SYMBOL(x)     x __attribute__((weak))
#define _WEAK_FUNCTION(x)   __attribute__((weak)) x

/* compiler dependent structure packing option */
#define PACKED_STRUCT_BEGIN
#define PACKED_STRUCT_END __attribute__((__packed__))

/* compiler dependent union packing option */
#define PACKED_UNION_BEGIN
#define PACKED_UNION_END  __attribute__((__packed__))

#ifdef __cplusplus
}
#endif

#endif   /* __comp_h__ */
