/*--------------------------------------------------------------------------*/
/**@file     KT0830EG.c
   @brief    KT0830EG收音底层驱动
   @details
   @author
   @date   2011-9-8
   @note
*/
/*----------------------------------------------------------------------------*/
#include "config.h"
#if	(KT0830EG == 1)

#include "KT0830EG.h"
#include "iic.h"

extern bit iic_busy;

u8  idata mem_afc[3];					//Remembered afc values for previous, current and next stations
u16 idata mem_freq[3];				    //Remembered channel frequencies for previous, current and next stations
u8  idata mem_vol;						//Remembered volume before mute


#ifdef SEEK_WITH_SNR
u8 idata mem_snr[3];					//Remembered SNR values for previous, current and next stations
#endif

#ifdef ULTRA_LOW_Q
int CAP_OFFSET;
#endif

//extern bool hp_in_busy;
/*----------------------------------------------------------------------------*/
/**@brief   标准IIC总线向一个目标ID读取几个数据
   @param   address : 目标ID
   @param   *iic_dat     :  存档读取到的数据的buffer指针
   @param   n :  需要读取的数据的个数
   @return  无
   @note    void iic_readKT(u8 chip_id,u8 iic_addr,u8 *iic_dat,u8 n)
*/
/*----------------------------------------------------------------------------*/
void iic_readKT(u8 chip_id,u8 iic_addr,u8 *iic_dat,u8 n)
{
//    while(hp_in_busy);
    iic_busy = 1;
    iic_start();                	//I2C启动
    iic_sendbyte(chip_id & 0xfe);   //写命令
    iic_sendbyte(iic_addr);         //写地址
    iic_start();                	//I2C启动
    iic_sendbyte(chip_id);     	    //写地址
    for (;n>1;n--)
    {
        *iic_dat++ = iic_revbyte(0); //读数据
    }
    *iic_dat++ = iic_revbyte(1);
    iic_stop();                 	//I2C停止时序
    iic_busy = 0;
}


void KT0830EG_Write(u8 Register_Address, u16 Temp)
{
    u8 pTemp[2];
    pTemp[0] = ((u8 *)(&Temp))[0];
    pTemp[1] = ((u8 *)(&Temp))[1];
    iic_write(KTFMw_address, Register_Address, pTemp, 2);
}

u16 KT0830EG_Read(u8 Register_Address)
{
    u8 pTemp[2];
    u16 Temp;
    iic_readKT(KTFMr_address, Register_Address, pTemp, 2);
    ((u8 *)(&Temp))[0] = pTemp[0];
    ((u8 *)(&Temp))[1] = pTemp[1];
    return Temp;
}



/***************************************************************************
@Function: KT0830EG initialization
@Parameter: NON
@Return: 0->Fail,1->Success
@Note: u8 KT_FMInit(void)
****************************************************************************/
u8 KT_FMInit(void)
{
    u16 regx;
    u8 i;

    for (i=0;i<INIT_FAIL_TH;i++)
    {
        delay_10ms(50);
        regx = KT0830EG_Read(0x12);						//Read power-up indicator
        if ((regx & 0x8800) != 0x8800)
            continue;
        break;
    }

    if (INIT_FAIL_TH == i)
        return(0);

    KT0830EG_Write(0x0a, 0x0000);						//Turn On AFC function
    KT0830EG_Write(0x02, 0x2207);						//Write Frequency Space to 100KHz

    if ((KT0830EG_Read(0x1D) & 0x0080) == 0x0080)		//if bit7==1, KT0830EG run patch	 /////problem:12 pin reset
    {
        // RFAGC patch (V2.2 and V3.4)
        regx = KT0830EG_Read(0x22);
        KT0830EG_Write(0x22, regx | 0x1000);			//Configure RFAGC parameter of agc_filter_dis=1

        regx = KT0830EG_Read(0x21);
        KT0830EG_Write(0x21, (regx & 0xfff8)| 0x0003);	//Configure RFAGC parameter of peaklowth=30mV

        regx = KT0830EG_Read(0x09);
        KT0830EG_Write(0x09, (regx & 0x1fff)| 0x2000);	//Configure RFAGC parameter of peakhighth=90mV

        // LOLDO patch (V2.8)
        regx = KT0830EG_Read(0x0A);
        KT0830EG_Write(0x0A, regx | 0x0400);			//loldo_ctrl=highest

        // SDMCODE patch (V3.7)
        while ((KT0830EG_Read(0x10)&0x4000) != 0)		//Wait cali-done signal
        {
            delay_10ms(5);
        }
        KT0830EG_Write(0x10,0x5200);					//Recali sdmcode
        delay_10ms(1);
        regx = KT0830EG_Read(0x11);
        if ((regx&0x000f)>0x0A)
        {
            KT0830EG_Write(0x11,(regx&0xfff0)|0x000A);	//sdm_code patch
        }
    }

#ifdef AUDIO_LEVEL_BOOST
    regx=KT0830EG_Read(0x23);
    KT0830EG_Write(0x23, regx | 0x0008);				// AU_GAIN=1
#endif

    KT_FMAntTuneOn();

    return(1);
}

/************************************************************************
@Function: The chip will tune the antenna automatically
		   whenever TUNE function is performed
@Parameter:	NON
@Return: NON
@Note: void KT_FMAntTuneOn(void)
*************************************************************************/
void KT_FMAntTuneOn(void)
{
    u16 regx;
    regx = KT0830EG_Read(0x23);
    KT0830EG_Write(0x23, regx | 0x0004);	//antenna tuning enable
    regx = KT0830EG_Read(0x1D);
    KT0830EG_Write(0x1D, (regx & 0xff7f)|((3-INDUCTOR_Q)<<5));	//cali loop disable =0
}

/******************************************************************
@Function: 	Mute the chip and Tune to Frequency
@Paramteer: Frequency:87.5MHz-->Frequency=8750;
@Return:
@Note: void KT_FMTune(u16 Frequency)
*******************************************************************/
void KT_FMTune(u16 Frequency)
{
    u16 idata channel;

    channel = (Frequency-6400)/5;
//	KT_FMMute(1);
    KT0830EG_Write(0x03, 0x8800 | channel);		//set tune bit to 1
    delay_10ms(5);
}

/*****************************************************************************
@Function: Read RSSI From KT0830EG
@Parameter: RSSI:range from -100 to -6, unit is dbm
@Return: non
@Note: void KT_FMReadRSSI(char *RSSI)
*****************************************************************************/
void KT_FMReadRSSI(char *RSSI)
{
    u16 regx;
    regx = KT0830EG_Read(0x12);
    *RSSI = -(100 - ((( regx >> 3 ) & 0x001F ) * 3 ));
}

#ifdef SEEK_WITH_SNR
/*****************************************************************************
@Function: Read SRN From KT0830EG
@Parameter: NON
@Return:
@Note: u8 KT_FMGetSNR(void)
*****************************************************************************/
u8 KT_FMGetSNR(void)
{
    u16 regx;
    regx = KT0830EG_Read(0x1F);
    return (regx & 0x00FF);
}
#endif

/*****************************************************************************
@Function: returned value's range is from -127 to 127, unit is KHz
@Parameter: NON
@Return: non
@Note: void KT_FMReadRSSI(char *RSSI)
*****************************************************************************/
u8 KT_FMGetAFC(void)
{
    u8 afc_delta;
    u16 reg14;
    reg14 = KT0830EG_Read(0x14);								// read AFC_DELTAF
    afc_delta = reg14 & 0x003f;
    if (afc_delta & 0x20)
    {
        afc_delta = afc_delta-32;
    }
    else
    {
        afc_delta = afc_delta+32;
    }
    return (afc_delta);
}

/*************************************************************************
@Function: check AFC_DELTA only
@Parameter:	Frequency:8700-10800
@Return: 0-->False Station 1-->Good Station
@Note: u8 KT0830EG_ValidStop(u16 Frequen)
**************************************************************************/
bit KT0830EG_ValidStop(u16 Frequen)
{
//	KT_FMTune(Frequency);
    u16 Frequency;
    u16 nextfreq;
    u8  afc[3];								//AFC value for previous, current and next channels
    u16 freq[3];							//frequency values for previous, current and next channels

#ifdef SEEK_WITH_SNR
    u8 snr[3];								//SNR for previous, current and next channels
    u8 snr2,snr3;
#endif

    char i,j;

    Frequency = Frequen * 10;

    afc[0]=0;
    afc[1]=0;
    afc[2]=0;				//initialize
    freq[0]=0;
    freq[1]=0;
    freq[2]=0;			//initialize

#ifdef SEEK_WITH_SNR
    snr[0]=0;
    snr[1]=0;
    snr[2]=0;				//initialize
#endif

    nextfreq=Frequency - CHANNEL_STEP;

    //Get AFC values of previous, current and next channels
    for (i=0;i<3;i++)
    {
        //get AFC values for the last station's information
        for (j=0;j<3;j++)
        {
            if (mem_freq[j] == nextfreq)
            {
                afc[i] = mem_afc[j];
                freq[i] = mem_freq[j];

#ifdef SEEK_WITH_SNR
                snr[i] = mem_snr[j];
#endif
            }
        }

        //get new AFC value if the station information is not investigated in the last run
        if (!freq[i])
        {
            KT_FMTune(nextfreq);
            afc[i] = KT_FMGetAFC();
#ifdef SEEK_WITH_SNR
            snr[i] = KT_FMGetSNR();
#endif
            freq[i] = nextfreq;
        }
        nextfreq += CHANNEL_STEP;
    }

    //Record AFC values
    mem_afc[0]=afc[0];
    mem_afc[1]=afc[1];
    mem_afc[2]=afc[2];
    mem_freq[0]=freq[0];
    mem_freq[1]=freq[1];
    mem_freq[2]=freq[2];

#ifdef SEEK_WITH_SNR
    mem_snr[0]=snr[0];
    mem_snr[1]=snr[1];
    mem_snr[2]=snr[2];
#endif

    //Determine the validation of current station
    if ((afc[0]<afc[1]) && (afc[1]<afc[2]) && (afc[0]<(32-AFCTH_PREV))
            && (afc[1]>(32-AFCTH)) && (afc[1]<(AFCTH+32)) && (afc[2]>(AFCTH_NEXT+32)))
    {

#ifdef SEEK_WITH_SNR
        KT_FMTune(Frequency);
        snr2 = KT_FMGetSNR();
        if ((snr[1] >= SNR_TH) && (snr2 >= SNR_TH))
        {
            return(1);
        }
        if ((snr[1] < SNR_TH) && (snr2 < SNR_TH))
        {
            return(0);
        }
        delay_10ms(5);
        snr3 = KT_FMGetSNR();
        if (snr3 >= SNR_TH)
        {
            return(1);
        }
        else
        {
            return(0);
        }

#endif

    }
    return(0);
}

/****************************************************************************
@Function: KT0830EG set MUTE when SEEK
@Parameter: flag:1 mute,0 dismute
@Return: non
@Note: void KT_FMMute(u8 flag)
****************************************************************************/
void KT_FMMute(u8 flag)
{
    u16 idata regx;
    regx = KT0830EG_Read(0x04);
    if (flag)
    {
        KT0830EG_Write(0x04, regx & 0xFFF0 );		//Write volume to 0
    }
    else
    {
        KT0830EG_Write(0x04, regx & 0xfff0 | 0x000e);
    }
}

/***************************************************************************
@Function: wake up the FT0830EG
@Parameter: non
@Return: 0->Fail 1->Success
@Note: u8 KT_FMWakeUp(void)
****************************************************************************/
void KT_FMWakeUp(void)
{
    u16 xdata regx;
    /*对于32.768KHz的共晶振与独立晶振，在应用IDE调试时，须加上，在SOCKY座上不用*/

    regx=KT0830EG_Read(0x0f);
    KT0830EG_Write(0x0F, regx | 0x4000);
    delay_10ms(10);

    KT0830EG_Write(0x0F, 0x8A00);				//Write Standby bit to 0

    KT_FMInit();
    KT_FMMute(0);
}

/****************************************************************************
@Function: LET KT0830EG working in IDLE
@Parameter: non
@Return: non
@Note: void KT_FMStandby(void)
****************************************************************************/
void KT_FMStandby(void)
{
    KT0830EG_Write(0x0F, 0x9A00);      			//Write Standby bit to 1
    delay_10ms(2);
}

/**************************************************************************
@Function: This function should be called just after power-up (within 50ms)
		   when crystal is used or before reference clock is applied
@Parameter:	non
@Return: 0->fail,1->success
@Note: bool KT0830EG_ChipID(void)
***************************************************************************/
bool KT0830EG_ChipID(void)
{
    u16 xdata regx;
    u8 xdata i;
    for (i=0;i<INIT_FAIL_TH;i++)
    {
        delay_10ms(1);
        regx = KT0830EG_Read(0x00);			//Read Manufactory ID
        if (regx != 0xb002)
            continue;
        break;
    }
    if (i == INIT_FAIL_TH)
    {
        return(0);
    }
    if (0xb002 == regx)
    {
        return TRUE;
    }
    return FALSE;
}

void KT0830EG_setch(u8 db)
{
    db=db;
}
#endif