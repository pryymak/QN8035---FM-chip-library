/*--------------------------------------------------------------------------*/
/**@file   fm_rev.h
   @brief  dac头文件
   @details
   @author
   @date   2011-9-8
   @note
*/
/*----------------------------------------------------------------------------*/

#ifndef _FM_REV_
#define _FM_REV_

#include "config.h"
#include "iic.h"

#define MAX_FRE    1080
#define MIN_FRE	   875
#define MAX_CHANNL (MAX_FRE - MIN_FRE + 1)


///<**********收音选择****///////
//#define RDA5807      1
//#define CL6017G      0
//#define BK1080       0
//#define KT0830EG     0
//#define QN8035       0
#ifdef __C51__
void fm_radio(void);
void fm_rev( void );
void dealmsgforfm(u8 msg);
void enter_fm_rev(void);
void scan_chan(u8 dir);
#endif

#endif
