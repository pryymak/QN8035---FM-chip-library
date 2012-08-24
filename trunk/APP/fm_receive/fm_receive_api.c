/*--------------------------------------------------------------------------*/
/**@file     fm_receive_api.c
   @brief    FM任务应用接口
   @details
   @author
   @date   2011-3-7
   @note
*/
/*----------------------------------------------------------------------------*/


#include "fm_api.h"
#include "fm_rev.h"
#if (FM_MODULE == 1)

extern u16 frequency;

u8 fm_addr;	///<在线的FM收量的地址指针

/*
void (_code * _code fm_init[])(void) = {init_rda5807,init_CL6017G};
bool (_code * _code fm_set_fre[])(u16) = {set_fre_rda5807,CL6017G_ValidStop};
void (_code * _code fm_power_off[])(void) = {rda5807_poweroff,CL6017G_PowerOff};
bool (_code * _code fm_read_id[])(void) = {rda5807_read_id,cl6017g_read_id};
void (_code * _code fm_mute[])(u8) = {rda5807_mute,cl6017g_mute};

void (_code * _code fm_init[])(void) 	  = {init_rda5807, 		init_CL6017G, 		Chip_wire2_Initialize, 	KT_FMWakeUp, 		init_QN8035};
bool (_code * _code fm_set_fre[])(u16)    = {set_fre_rda5807, 	CL6017G_ValidStop, 	BK1080_ValidStop,		KT0830EG_ValidStop, QND_RXValidCH};//QND_TuneToCH};set_fre_QN8035};//
void (_code * _code fm_power_off[])(void) = {rda5807_poweroff, 	CL6017G_PowerOff, 	BK1080_PowerDown, 		KT_FMStandby, 		QN8035_powerdown};
bool (_code * _code fm_read_id[])(void)   = {rda5807_read_id, 	cl6017g_read_id, 	BK1080_Read_ID, 		KT0830EG_ChipID, 	QN8035_Read_ID};
void (_code * _code fm_mute[])(u8)        = {rda5807_mute, 		cl6017g_mute, 		BK1080_MUTE, 			KT_FMMute, 			QN8035_MUTE};
void (_code * _code fm_setch[])(u8)	      = {rda5807_setch,     CL6017_setch,       BK1080_setch,           KT0830EG_setch,     QN8035_setch};
//void (_code * _code fm_set_vol[])(u8) = {0};
*/



void (_code * _code fm_init[])(void) 	  =
{

#if	 RDA5807
    init_rda5807,
#endif

#if	 AR1015
    AR1000_Intialization,
#endif

#if	BK1080
    Chip_wire2_Initialize,
#endif

#if	KT0830EG
    KT_FMWakeUp,
#endif
#if	QN8035
    init_QN8035
#endif

};
bool (_code * _code fm_set_fre[])(u16)    =
{

#if	RDA5807
    set_fre_rda5807,
#endif

#if	 AR1015
    AR1000_ValidStop,
#endif

#if	 BK1080
    BK1080_ValidStop,
#endif

#if	 KT0830EG
    KT0830EG_ValidStop,
#endif

#if	QN8035
    QND_RXValidCH
#endif

};//QND_TuneToCH};set_fre_QN8035};//
void (_code * _code fm_power_off[])(void) =
{

#if	RDA5807
    rda5807_poweroff,
#endif

#if	AR1015
    AR1000_PowerOff,
#endif

#if	 BK1080
    BK1080_PowerDown,
#endif

#if	 KT0830EG
    KT_FMStandby,
#endif

#if	 QN8035
    QN8035_powerdown
#endif

};
bool (_code * _code fm_read_id[])(void)   =
{

#if	RDA5807
    rda5807_read_id,
#endif

#if	AR1015
    AR1000_Get_ID,
#endif

#if	 BK1080
    BK1080_Read_ID,
#endif

#if	 KT0830EG
    KT0830EG_ChipID,
#endif

#if	 QN8035
    QN8035_Read_ID
#endif
};
void (_code * _code fm_mute[])(u8)        =
{
#if	RDA5807
    rda5807_mute,
#endif

#if	AR1015
    AR1000_SetMute,
#endif

#if	BK1080
    BK1080_MUTE,
#endif

#if	KT0830EG
    KT_FMMute,
#endif

#if	 QN8035
    QN8035_MUTE
#endif

};
void (_code * _code fm_setch[])(u8)	  =
{

#if	RDA5807
    rda5807_setch,
#endif

#if	AR1015
    AR1015_setch,
#endif

#if	BK1080
    BK1080_setch,
#endif

#if	KT0830EG
    KT0830EG_setch,
#endif

#if	 QN8035
    QN8035_setch
#endif
};

/*----------------------------------------------------------------------------*/
/**@brief   FM模块初始化接口函数
   @param
   @return
   @note    void init_fm_rev(void)
*/
/*----------------------------------------------------------------------------*/
bool init_fm_rev(void)
{
    for (fm_addr = 0; fm_addr < (sizeof(fm_read_id)/2); fm_addr++)
    {
        if ((*fm_read_id[fm_addr])())
        {
            (* fm_init[fm_addr])();

            return TRUE;
        }
    }

    fm_addr = 0xff;
    return FALSE;
}


/*----------------------------------------------------------------------------*/
/**@brief   关闭FM模块电源
   @param
   @return
   @note    void fm_rev_powerdown(void
*/
/*----------------------------------------------------------------------------*/
void fm_rev_powerdown(void)
{
    (* fm_power_off[fm_addr])();
}


/*----------------------------------------------------------------------------*/
/**@brief   设置一个FM频点的接口函数
   @param   mode = 0: 使用frequency中的值，= 1:频点加1， = 2:频点减1
   @return  1：有台；0：无台
   @note    bool set_fre(u16 fre, u8 mode)
*/
/*----------------------------------------------------------------------------*/
bool set_fre(u8 mode)
{

    if (mode == FM_FRE_INC)
    {
        frequency++;
    }
    else if (mode == FM_FRE_DEC)
    {
        frequency--;
    }
    if (frequency > MAX_FRE)
        frequency = MIN_FRE;
    if (frequency < MIN_FRE)
        frequency = MAX_FRE;
    return (* fm_set_fre[fm_addr])(frequency);
}


/*----------------------------------------------------------------------------*/
/**@brief   FM模块Mute开关
   @param   flag：Mute使能位
   @return  无
   @note    void fm_module_mute(u8 flag)
*/
/*----------------------------------------------------------------------------*/
void fm_module_mute(u8 flag)
{
    (* fm_mute[fm_addr])(flag);
}

/*----------------------------------------------------------------------------*/
/**@brief   设置一个FM频点的接口函数
   @param   mode = 0: 使用frequency中的值，= 1:频点加1， = 2:频点减1
   @return  1：有台；0：无台
   @note    bool set_fre(u16 fre, u8 mode)
*/
/*----------------------------------------------------------------------------*/
void fm_set_ch(u8 db)
{
    (* fm_setch[fm_addr])(db);
}

#endif
