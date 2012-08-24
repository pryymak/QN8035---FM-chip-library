/*--------------------------------------------------------------------------*/
/**@file     CL6017.h
   @brief    CL6017Í·ÎÄ¼þ
   @details
   @author
   @date   2011-3-30
   @note
*/
/*----------------------------------------------------------------------------*/
#ifndef	_CL6017_H_
#define _CL6017_H_
/*--------------------------------------------------------------------------------------
*This file is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
*****************
Compiler:uVision2 2.40
*****************
* website: http://www.bekencorp.com
---------------------------------------------------------------------------------------*/

#include "iic.h"
#include "typedef.h"


void init_CL6017G(void);
void CL6017G_SetFre (u16 curFreq);
bool CL6017G_ValidStop(u16 unFreq);
void CL6017G_PowerOff(void);
bool cl6017g_read_id(void);
void cl6017g_mute(u8 flag);
void CL6017_setch(u8 db);

#endif	//_CL6017_H_