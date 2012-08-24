/*--------------------------------------------------------------------------*/
/**@file     CL6017G.c
   @brief    CL6017G收音底层驱动
   @details
   @author
   @date   2011-9-8
   @note
*/
/***************************************************************************/
#include "config.h"
#if	(CL6017G == 1)

#include "CL6017.h"

#define FM_CL6017G_WRITE      0x20
#define FM_CL6017G_READ       0x21

#define FM_CL6017G_RSSILEV        160 	//160~190  越大，信号弱的电台就会被屏蔽	 170
#define FM_CL6017G_ENVELOPELEV    630  	//480~640  越小，信号弱的电台就会被屏蔽   608
#define FM_CL6017G_FDLEV          40   	//25~50    基准的频偏，越小越严格 	     40

u8 xdata CL6017G_ControlData[47];


void CL6017G_READ_REG(u8 *pBuf,u8 size)
{
    iic_readn(FM_CL6017G_READ,0xff,pBuf,size);
}

void I2C_WriteCL6017GReg(u8 *pBuf,u8 num)
{
    iic_write(FM_CL6017G_WRITE,0xff,pBuf,num);
}

/*******************************************************
Function: Chip_wire2_Initialize()
Parameter:	None
Description: CL6017 I2C mode initialization
*****************************************************/
void init_CL6017G(void)
{
    //read reg
    CL6017G_ControlData[0]  = 0x64;     // index + 22 ---- reg correct index
    CL6017G_ControlData[1]  = 0x69;
    CL6017G_ControlData[2]  = 0x78;
    CL6017G_ControlData[3]  = 0x10;       //0x10 -->32.768K,0X11-->12M,0X15-->24M,
    CL6017G_ControlData[4]  = 0x02;
    CL6017G_ControlData[5]  = 0xff;
    CL6017G_ControlData[6]  = 0x1f;     //R28_bit<5:4>=01 0x2f
    CL6017G_ControlData[7]  = 0xb2;
    CL6017G_ControlData[8]  = 0x9b;
    CL6017G_ControlData[9]  = 0x05;
    CL6017G_ControlData[10] = 0x00;
    CL6017G_ControlData[11] = 0x90;
    CL6017G_ControlData[12] = 0x00;
    CL6017G_ControlData[13] = 0x9a;    //0xaa //modi by alpha110119
    CL6017G_ControlData[14] = 0xb2;    //0XBC
    CL6017G_ControlData[15] = 0x15;
    CL6017G_ControlData[16] = 0x96;
    CL6017G_ControlData[17] = 0x80;
    CL6017G_ControlData[18] = 0x00;
    CL6017G_ControlData[19] = 0x00;
    CL6017G_ControlData[20] = 0x00;
    CL6017G_ControlData[21] = 0x00;
    CL6017G_ControlData[22] = 0x00;
    CL6017G_ControlData[23] = 0x00;
    CL6017G_ControlData[24] = 0x60;
    CL6017G_ControlData[25] = 0x02;
    CL6017G_ControlData[26] = 0xfe;
    CL6017G_ControlData[27] = 0x02;
    CL6017G_ControlData[28] = 0xa0;
    CL6017G_ControlData[29] = 0x00;
    CL6017G_ControlData[30] = 0x00;
    CL6017G_ControlData[31] = 0x02;
    CL6017G_ControlData[32] = 0x80;
    CL6017G_ControlData[33] = 0x00;
    CL6017G_ControlData[34] = 0x60;
    CL6017G_ControlData[35] = 0x25;
    CL6017G_ControlData[36] = 0x90;
    CL6017G_ControlData[37] = 0xff;		//0x0f //modi by alpha110119
    CL6017G_ControlData[38] = 0x5f;
    CL6017G_ControlData[39] = 0xbc;		//0xbc //modi by alpha110119 24 --0xbe 32768 --0xbc
    CL6017G_ControlData[40] = 0xa3;
    CL6017G_ControlData[41] = 0x28;
    CL6017G_ControlData[42] = 0x27;	    //0x27
    CL6017G_ControlData[43] = 0x80;
    CL6017G_ControlData[44] = 0x10; 	//R66=00010000 0x02
    CL6017G_ControlData[45] = 0x80;
    CL6017G_ControlData[46] = 0xc0;		//R68_bit4=1 0xc0

    I2C_WriteCL6017GReg(CL6017G_ControlData, 47);	  //modi by alpha110224
    delay_10ms(1);
    // power up and initial operation
    CL6017G_ControlData[5] &= 0xfe; 	//power up reg27
    I2C_WriteCL6017GReg(CL6017G_ControlData, 6);

    delay_10ms(80); 	//must wait 800ms for crystal oscillate stable

    CL6017G_ControlData[0] &= 0xdf;   //mute off reg22
    I2C_WriteCL6017GReg(CL6017G_ControlData, 1);
}

/**************************************************
Function: DisMuteCL6017()
Parameter: flag:1 mute,0 dismute

Description: Dismute CL6017
/**************************************************/
void cl6017g_mute(u8 flag)
{
    if (flag)
    {
        CL6017G_ControlData[0] |= 0x20;	//mute
    }
    else
    {
        CL6017G_ControlData[0] &= 0xdf;
    }

    I2C_WriteCL6017GReg(CL6017G_ControlData, 1);
}

/**************************************************
Function: void CL6017G_SetFre(unsigned int unFreq)
Parameter:
Description:
Note: 875~1080 unit:100kHz
**************************************************/
void CL6017G_SetFre(u16 unFreq)
{
    unsigned int ulFreq;	//<=1080*25=27000=0x6978
//	ulFreq= (unsigned long) unFreq * 100;  //modi by alpha 100827
//	ulFreq >>= 2;
    ulFreq= unFreq * 25;

    // set frequency
    CL6017G_ControlData[2] = (u8) (ulFreq % 256);

//	CL6017G_ControlData[1] = (u8) (ulFreq >> 8);//reg23
    CL6017G_ControlData[1] = ((u8 *)(&ulFreq))[0];  	//reg23

    CL6017G_ControlData[4] &= 0xf3;		//tune=0,seek=0 reg26
    I2C_WriteCL6017GReg(CL6017G_ControlData, 5);

    CL6017G_ControlData[4] |= 0x08;		//tune=1, begin tune operaton reg26
    I2C_WriteCL6017GReg(CL6017G_ControlData,5);
    delay_10ms(5);
}

/**********************************************************************
@Function: CL6017_SetFreq()
@Parameter: curFreq: FM frequency *10 ,such as  1017,933,1077..
@Description: set FM to a frequency
@Note: bool CL6017G_ValidStop(unsigned int unFreq)//875~1080 unit:100kHz
************************************************************************/
bool  CL6017G_ValidStop(u16 unFreq)
{
    bool   bStcFlag = 0;
    _xdata u16 unLoop = 0;
    int nRssi;
    _xdata u16 unEnvelope;
    _xdata u16 unFd;
    u8 idata Temp[22];

    CL6017G_SetFre(unFreq);

    delay_10ms(9);//delay_10ms(2);
    CL6017G_READ_REG(Temp, 8);  //modi by alpha100827 old is 22
    bStcFlag = (bool)(Temp[3] & 0x10);

    if (bStcFlag)  //modi by alpha100827
    {
        nRssi = Temp[5];

        nRssi -= 7;

        if (nRssi < 0)
        {
            nRssi = 0;
        }
        unEnvelope = ((u16)Temp[6] << 2) + (Temp[7] >> 6);

//		unFd[0] = temp[4] + (((u16)temp[3] & 0x01) << 8); 	//reg4/reg3 bit0
        ((u8 *)(&unFd))[0] = (Temp[3] & 0x01);
        ((u8 *)(&unFd))[1]	= Temp[4];


        if (unFd >500)
        {
            unFd = 512 - unFd;
        }

        if ((nRssi >= FM_CL6017G_RSSILEV) && (unEnvelope <= FM_CL6017G_ENVELOPELEV) && (unFd <= FM_CL6017G_FDLEV))
        {
            return 1;
        }
    }
    return 0;
}


bool cl6017g_read_id(void)
{
    u8 xdata TempID[2] = { 0x00 };
    CL6017G_ControlData[0] = 0x64;    // index + 22 ---- reg correct index
    CL6017G_ControlData[1] = 0x69;
    CL6017G_ControlData[2] = 0x78;
    CL6017G_ControlData[3] = 0x15;    //0x10 -->32.768K,0X11-->12M,0X15-->24M,
    CL6017G_ControlData[4] = 0x02;
    CL6017G_ControlData[5] = 0xff;
    CL6017G_ControlData[6] = 0x1f;    //R28_bit<5:4>=01 0x2f
    CL6017G_ControlData[7] = 0xb2;
    CL6017G_ControlData[8] = 0x9b;
    CL6017G_ControlData[9] = 0x05;
    CL6017G_ControlData[10] = 0x00;
    CL6017G_ControlData[11] = 0x90;
    CL6017G_ControlData[12] = 0x00;
    CL6017G_ControlData[13] = 0xaa;
    CL6017G_ControlData[14] = 0xb2;	  //0XBC
    CL6017G_ControlData[15] = 0x15;
    CL6017G_ControlData[16] = 0x96;
    CL6017G_ControlData[17] = 0x80;
    I2C_WriteCL6017GReg(CL6017G_ControlData,18);
    CL6017G_ControlData[5] &= 0xfe;   //POWER ON
    I2C_WriteCL6017GReg(CL6017G_ControlData,6);
    delay_10ms(15);  			      //delay 150ms
    CL6017G_READ_REG(TempID,2);	      //ID=TEMP[0]
    CL6017G_ControlData[5] |= 0x01;   //POWER DOWN
    I2C_WriteCL6017GReg(CL6017G_ControlData,6);
    if (0x0a == TempID[0])
    {
        return TRUE;
    }

    return FALSE;
}

/*************************************************************
@function: PowerOFF CL6017
@parameter: non
@return: non
@note: void CL6017G_PowerOff(void)
**************************************************************/
void CL6017G_PowerOff(void)
{
    CL6017G_ControlData[0] |= 0x20;		//mute on
    CL6017G_ControlData[4] &= 0xf3;		//tune=0,seek=0
    I2C_WriteCL6017GReg(CL6017G_ControlData,5);

    CL6017G_ControlData[5] |= 0x01; 	//set DISABLE=1 to power down
    I2C_WriteCL6017GReg(CL6017G_ControlData,6);
    delay_10ms(5);
}

void CL6017_setch(u8 db)
{
    db=db;
}
#endif