/*--------------------------------------------------------------------------*/
/**@file     bk1080.c
   @brief    BK1080收音底层驱动
   @details
   @author
   @date   2011-3-30
   @note
*/
/*----------------------------------------------------------------------------*/
#include "config.h"
#if	(BK1080 == 1)

#include "bk1080.h"

//u8 idata TmpData8[4] = {0,0,0,0};


/*BK1080 initialization value */
u16 _code HW_Reg[] =
{
    0x0008,
    0x1080,
#if XTAL_CLOCK	//reg2
    0x1201,	//for internal crystal clock
#else
    0x0201,	//for extern clock
#endif
    0x0000,
    0x40C0,
    0x0A1F,	  //reg5	RSSI[15:8] BAND[7:6] SPACE[5:4],Europe standar
    0x002E,
    0x02FF,
    0x5B11,
    0x0000,
    0x411E,
    0x0000,
    0xCE00,
    0x0000,
    0x0000,
    0x1000,		//reg15
#ifdef USE_12M_CRYSTAL
    0x16E3,
    0x8000,
#else
    0x0010,
    0x0000,
#endif    
    0x13FF,
    0x9852,
    0x0000,
    0x0000,
    0x0008,
    0x0000,
    0x51E1,
    0x38BC,
    0x2645,
    0x00E4,
    0x1CD8,
    0x3A50,
    0xEAF0,
    0x3000,	//reg31
    0x0200,
    0x0000,
};

void BEKEN_I2c_Read(u8 reg,u8 *pBuf,u8 num)
{
    reg=reg<<1;
    reg|=0x01;
    iic_readn(CHIP_DEV_ID,reg,pBuf,num);
//    delay_10ms(1);
}

void BEKEN_I2c_Write(u8 reg,u8 *pBuf,u8 num)
{
    reg=reg<<1;
    iic_write(CHIP_DEV_ID,reg,pBuf,num);
//    delay_10ms(1);
}
/**************************************************
@Function:  Chip_wire2_Initialize()
@Parameter: None
@Description: BK1080 I2C mode initialization
/**************************************************/
void Chip_wire2_Initialize(void)
{
    u8 index;
    u8 _xdata TmpData8_Init[68];
    delay_10ms(100);	//delay 1s,please revise delay time according to your MCU

    for (index = 0; index < 34; index++)
    {
        TmpData8_Init[index*2] = (HW_Reg[index] >> 8)&0xff;
        TmpData8_Init[index*2+1] = (HW_Reg[index])&0xff;
    }

    BEKEN_I2c_Write(0,TmpData8_Init,68);	//start from reg2,total 60 byte

    delay_10ms(25);			//delay 250ms
    TmpData8_Init[25*2+1] &= 0x7f;
    BEKEN_I2c_Write(25,&(TmpData8_Init[25*2]),2);
    TmpData8_Init[25*2+1] |= 0x80;
    BEKEN_I2c_Write(25,&(TmpData8_Init[25*2]),2);

    delay_10ms(6);			//delay 60ms
}


/**************************************************
Function: BK1080_SetFreq()
Parameter:
	curFreq: FM frequency *10 ,such as  1017,933,1077..

Description:
	set FM to a frequency
/**************************************************/
void  BK1080_SetFreq(u16 curFreq)
{
    u8 curChan;
    u8 temp[2];
//	BEKEN_I2c_Read(0x02,temp,2); //read reg3,with 2 bytes

//	temp[0] &= 0xfe;
    curChan = curFreq - MIN_FRE;

    temp[0] = 0x00;				//reset tune
    temp[1]= curChan;			//set reg3 CHAN<9:0>
    BEKEN_I2c_Write(0x03,temp,2);   //write reg3,with 2 bytes

    temp[0] = 0x80;
    BEKEN_I2c_Write(0x03,temp,2);
}

/**************************************************
Function: BK1080_ValidStop()

Parameter:
	freq:FM frequency *10 ,such as  1017,933,1077...
//	start_freq:lowest frequency,in China,start_freq=875
Description:
	check a frequency is a valid.
Return:
	1:ok
	0:false
/**************************************************/
bool BK1080_ValidStop(u16 freq)//,u16 start_freq)
{
    static u16 last_tuned_freq=0;
    static u16 g_last_freq_deviation_value=0;
    u8 temp[4];
//	bit stc_flag = 0;
//	u8 loopnum;
    u16 cur_freq_deviation;

    BK1080_SetFreq(freq);
    delay_10ms(7);		//延时时间>=30ms

///////////////////////////////////////////////////////////////////
//new added 2009-05-30

    BEKEN_I2c_Read(0x07,temp,2);	//start from reg 0x7,with 2bytes

    ((u8 *)(&cur_freq_deviation))[0] = temp[0];
    ((u8 *)(&cur_freq_deviation))[1] = temp[1];

    cur_freq_deviation = cur_freq_deviation>>4;

    BEKEN_I2c_Read(0x0a,&(temp[2]),2);

/////////////////////////////////////////////////////////////////////
    if (temp[2]&0x10)    			//check AFCRL bit12
    {
        last_tuned_freq=freq;			//save last tuned freqency
        g_last_freq_deviation_value=cur_freq_deviation;
        return 0;
    }

    if (temp[3]<15) //RSSI<10   //搜台较少可减少该值，假台较多可增大该值
    {
        last_tuned_freq=freq;			//save last tuned freqency
        g_last_freq_deviation_value=cur_freq_deviation;
        return 0;
    }

    if ( (temp[1]&0x0f) <2) //SNR<2 //搜台较少可减少该值，假台较多可增大该值
    {
        last_tuned_freq=freq;			//save last tuned freqency
        g_last_freq_deviation_value=cur_freq_deviation;
        return 0;
    }
/////////////////////////////////////////////////////////////////////////
//add frequency devation check
    if ( (cur_freq_deviation>=0xfa)&&(cur_freq_deviation<=(0xfff-0xfa)))	 //0x64 //搜台较少可减少该值，假台较多可增大该值
    {
        last_tuned_freq=freq;			//save last tuned freqency
        g_last_freq_deviation_value=cur_freq_deviation;

        return 0;
    }

///////////////////////////////////////////////////////////////////////////
//new added 2009-05-30
    if ( (freq>875)&&( (freq-last_tuned_freq)==1) )//start_freq)&&( (freq-last_tuned_freq)==1) )
    {
        if (g_last_freq_deviation_value&0x800)
        {
            last_tuned_freq=freq;		//save last tuned freqency
            g_last_freq_deviation_value=cur_freq_deviation;
            return 0;
        }

        if (g_last_freq_deviation_value<150)	 //搜台较少可减少该值，假台较多可增大该值
        {
            last_tuned_freq=freq;		//save last tuned freqency
            g_last_freq_deviation_value=cur_freq_deviation;
            return 0;
        }
    }

    if ( (freq>=875)&&( (last_tuned_freq - freq)==1) )//start_freq)&&( (last_tuned_freq - freq)==1) )
    {
        if ( (g_last_freq_deviation_value&0x800)==0)
        {
            last_tuned_freq=freq;		//save last tuned freqency
            g_last_freq_deviation_value=cur_freq_deviation;
            return 0;
        }

        if (g_last_freq_deviation_value>(0xfff-150) ) //搜台较少可减少该值，假台较多可增大该值
        {
            last_tuned_freq=freq;		//save last tuned freqency
            g_last_freq_deviation_value=cur_freq_deviation;
            return 0;
        }
    }

    last_tuned_freq=freq;				//save last tuned freqency
    g_last_freq_deviation_value=cur_freq_deviation;

    return 1;
}


/*********************************************************
@Function: Powerdown BK1080
@Parameter: non
@Return: non
@Note: void BK1080_PowerDown(void)
*********************************************************/
void BK1080_PowerDown(void)
{
    u8 temp[2];
    temp[0] =0x02;			  //write 0x0241 into reg2
    temp[1] =0x41;
    BEKEN_I2c_Write(0x02,temp,2);

    delay_10ms(5);				  //delay 50ms
}

/***********************************************************
@Function:
@Parameter:
@Return:
@Note: 	bool BK1080_Read_ID(void)
************************************************************/
bool BK1080_Read_ID(void)
{
    u16 xdata bk_id;
    u8 temp[2];
    BEKEN_I2c_Read(0x01,temp,2); 	//read reg3,with 2 bytes
    ((u8 *)(&bk_id))[0] = temp[0];
    ((u8 *)(&bk_id))[1] = temp[1];
    if (0x1080 == bk_id)
    {
        return TRUE;
    }
    return FALSE;
}

void BK1080_MUTE(u8 flag)
{
    u8 _idata TmpData8[2];

    BEKEN_I2c_Read(2,TmpData8,2);
    if (flag)
        TmpData8[0] |= 0x40;//mute
    else
        TmpData8[0] &= 0xbf;

    BEKEN_I2c_Write(2,TmpData8, 2);
}

void BK1080_setch(u8 db)
{
    db=db;
}
#endif
