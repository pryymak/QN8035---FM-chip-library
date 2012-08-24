/*--------------------------------------------------------------------------*/
/**@file     bk1080.h
   @brief    BK1080 ’“Ù
   @details
   @author
   @date   2011-3-30
   @note
*/
/*----------------------------------------------------------------------------*/
#ifndef _BK_1080_H_
#define _BK_1080_H_

#ifdef __C51__
#include "iic.h"
#include "typedef.h"
#include "fm_rev.h"

#define XTAL_CLOCK			0
#define CHIP_DEV_ID 		0x80


void Chip_wire2_Initialize(void);
void  BK1080_SetFreq(u16 curFreq);
bool BK1080_ValidStop(u16 freq);//,u16 start_freq);
void BK1080_PowerDown(void);
bool  BK1080_Read_ID(void);
void BK1080_MUTE(u8 flag);
void BK1080_setch(u8 db);
#endif		//__C51__

#endif		//_BK_1080_H_