#ifndef _RDA_5807_H_
#define _RDA_5807_H_

#ifdef __C51__
#include "iic.h"
#include "typedef.h"

#define RDA5807_WR_ADDRESS  0x20       ///<RDA5807 Ð´µØÖ·
#define RDA5807_RD_ADDRESS  0x21       ///<RDA5807 ¶ÁµØÖ·


void init_rda5807(void);
bool set_fre_rda5807(u16 fre, u8 mode);
void rda5807_poweroff(void);
void rda5807_mute(u8 dir);
//u8 rda5807p_online(void);
bool rda5807_read_id(void);
void rda5807_setch(u8 db);
#define RDA5807_RSSI  0x4

#endif

#endif
