/*--------------------------------------------------------------------------*/
/**@file     ar1019.c
   @brief    ar1019收音底层驱动
   @details
   @author
   @date   2011-6-20
   @note
*/
/*----------------------------------------------------------------------------*/

#include "config.h"

#include "AR1019.h"
#include "clock.h"
#include"uart.h"


#ifdef  AR1015

extern bool iic_busy;

#define INTERNAL_XO   // mark this line if you're using external reference clock
#ifdef INTERNAL_XO
//独立晶振
u16 _code  AR1000reg[18]={ //using  AR1000 XO function
	0xFFFB,		// R0 -- the first writable register .
	0x5B15,		// R1.
	0xD0B9,		// R2.
	0xA00a,		// R3, seekTHD = 16	 /////搜台门限
	0x0780,		// R4					 
	0x28AB,		// R5
	0x6400,		// R6
	0x1EE7,		// R7
	0x7141,		// R8
	0x007D,		// R9
	0x82C6,		// R10  disable wrap
	0x4E55,		// R11. <--- 
	0x970C,		// R12.
	0xB845,		// R13
	0xFE2D,		// R14 //0xFE,0x2D, // R14	 D9=1,single seek
	0x8097,		// R15
	0x04A1,		// R16
	0xDF6A		// R17
};    
#else
//外部提供频率
u16 _code  AR1000reg[18]={ // using External 32.768KHz Reference Clock
	0xFF7B,		// R0 -- the first writable register .  (disable xo_en)
	0x5B15,		// R1.
	0xD4B9,		// R2.//0xD0B9,
	0xA00a,		// R3   seekTHD = 16  ///////搜台门限
	0x0780,		// R4
	0x28AB,		// R5
	0x6400,		// R6
	0x1EE7,		// R7
	0x7141,		// R8
	0x007D,		// R9
	0x82C6,		// R10  disable wrap
	0x4F55,		// R11. <--- (disable xo_output)
	0x970C,		// R12.
	0xB845,		// R13
	0xFE2D,		// R14 //0xFE,0x2D, // R14	 D9=1,single seek
	0x8097,		// R15
	0x04A1,		// R16
	0xDF6A		// R17
};    
#endif

// Volume Control
// there are two different fields about volume control in AR1000F
//  Volume   :  D7  ~D10 in register R3
//  Volume2 :  D12~D15 in register R14
//  17 combinations of ( volume2 + volume)  are  recommended.
//  
//  
// code unsigned char AR1000vol[19]={ // volume control  (increasing) 070822
	// 0x0F,	// step 0
	// 0xCF,	// step 1
	// 0xDF,	// step 2
	// 0xFF,	// 3
	// 0xCB,	// 4
	// 0xDB,	// 5
	// 0xFB,	// 6
	// 0xFA,	// 7
	// 0xF9,	// 8
	// 0xF8,	// 9
	// 0xF7,	//10
	// 0xD6,	//11
	// 0xE6,	//12
	// 0xF6,	//13
	// 0xE3,	//14
	// 0xF3,	//15
	// 0xF2,	//16 
	// 0xF1,	//17
	// 0xF0	//18 <------ default setting
// };

//
//idata DATA_TYPE_S Reg_Data[18]; 
_xdata DATA_TYPE_S Reg_Data[18]; 

//IIC写data
void AR1000_I2C_Write_Data(char Reg)
{
/*	char ack;
	
again:	
	I2C_START 
	ack=Write_I2C_Byte(AR1000_I2C_WRITE_ADDR);
	ack|=Write_I2C_Byte(Reg);
	ack|=Write_I2C_Byte(Reg_Data[Reg].c[0]);
	ack|=Write_I2C_Byte(Reg_Data[Reg].c[1]);
	
	I2C_STOP
	if(ack)
		goto again;
*/
	iic_busy  = 1;

    iic_start();

    iic_sendbyte(AR1000_I2C_WRITE_ADDR);
    
    iic_sendbyte(Reg);   
    
    iic_sendbyte(Reg_Data[Reg].c[0]);

	iic_sendbyte(Reg_Data[Reg].c[1]);
         
    iic_stop();

	iic_busy = 0;

}
//IIC读data
unsigned int AR1000_I2C_Read_Data(char Reg)
{
/*	char ack;
	DATA_TYPE_S value;
	
again:	
	I2C_START 
	// set address of the register which we want to read 
	ack=Write_I2C_Byte(AR1000_I2C_WRITE_ADDR);
	ack|=Write_I2C_Byte(Reg);
	if(ack)
		goto again;
		
again2:		
	I2C_START
	// start to read
	ack=Write_I2C_Byte(AR1000_I2C_READ_ADDR);
	if(ack)
		goto again2;
		
	value.c[0]= Read_I2C_Byte(); // read 8-bit data
	value.c[1]= Read_I2C_Byte(); // read another 8-bit data
	I2C_STOP
	
	return value.i;
	*/
	DATA_TYPE_S value;

    iic_busy  = 1; 
	
    iic_start();

    iic_sendbyte(AR1000_I2C_WRITE_ADDR);
    
    iic_sendbyte(Reg); 

    iic_start();

	iic_sendbyte(AR1000_I2C_READ_ADDR);

	value.c[0]= iic_revbyte(0); // read 8-bit data
	value.c[1]= iic_revbyte(1); // read another 8-bit data
	    
    iic_stop();

	iic_busy = 0;

	return value.i;

}
//初始化
void AR1000_Intialization(void)
{
 
	 char Cnt1; // init R1, R2, ....R17 then R0
     unsigned int status;
  u8 cnt=0;

	Reg_Data[0].i=AR1000reg[0]&0xFFFE;
	AR1000_I2C_Write_Data(0);
	
  	for(Cnt1=1;Cnt1<18;Cnt1++)
  	{
  		Reg_Data[Cnt1].i=AR1000reg[Cnt1]; 
		    #ifdef  FM_INDEPENDENT_ADJ_VOL
		 Reg_Data[1].BIT.B1 = OFF;
			#endif
			Reg_Data[3].i=0xa008; 

  		AR1000_I2C_Write_Data(Cnt1);

	}
	
	Reg_Data[0].i=AR1000reg[0];
	AR1000_I2C_Write_Data(0);
	
	//Power-On Calibration begins
	// then wait for STC flag
	// maybe you need to delay for a while
	// delay ( 100 ms )
	status = AR1000_I2C_Read_Data(ADDR_STATUS);
	status &=MASK_STC; // check STC flag 
	while( status == 0)
	{
		// maybe you can delay for a while
		// delay ( 100 ms )
				delay_10ms(1);
                   if(++cnt>100)
			break;
		status = AR1000_I2C_Read_Data(ADDR_STATUS);
		status &=MASK_STC; // check STC flag 
	}
	//Power-On Calibration Ends

	//SetAR1000_volume(19);	// max. output is recommended !! it is already set in the default register setting
						// you dont need to set it again
}
//关FM
void AR1000_PowerOff(void)
{
    AR1000_MUTE_ON   // Set Muto ON before SCAN
    AR1000_I2C_Write_Data(1);

	AR1000_TUNE_OFF  //clear TUNE
    AR1000_I2C_Write_Data(2);

    AR1000_SEEK_OFF;	//clear SEEK 
	AR1000_I2C_Write_Data(3); 

	Reg_Data[0].BIT.B0 = 0;
	AR1000_I2C_Write_Data(0);
}
//手动调台
void AR1000_I2C_TUNE(unsigned int FreqKHz)// unsigned int band, unsigned char space)
{
  unsigned int status;
  unsigned int flag;
  unsigned int CHAN =0x0000; //actually it's 9-bit
  u8 cnt=0;
  AR1000_MUTE_ON   // Set Muto ON before TUNE
  AR1000_I2C_Write_Data(1);

  AR1000_SEEK_OFF;	//clear SEEK 
  AR1000_I2C_Write_Data(3);

  AR1000_TUNE_OFF  //clear TUNE
  AR1000_I2C_Write_Data(2);
  
  CHAN = FreqKHz-690;
  Reg_Data[2].i &=0xfe00;
  Reg_Data[2].i|=CHAN;
  AR1000_TUNE_ON
  AR1000_I2C_Write_Data(2);
 
  // TUNE  begins
  // then wait for STC flag
  
  // maybe you need to delay for a while
  // delay ( 100 ms )
  status = AR1000_I2C_Read_Data(ADDR_STATUS);
  flag = status & MASK_STC; // check STC flag 
  while( flag == 0)
  {
		// maybe you can delay for a while
		// delay ( 100 ms )
		delay_10ms(1);
                   if(++cnt>200)
			break;
		status = AR1000_I2C_Read_Data(ADDR_STATUS);
		flag = status & MASK_STC; // check STC flag 
  }
  //TUNE Ends 
  
  AR1000_MUTE_OFF    //  <---   we dont mute off now, bcz this function may be called by other functions 
  AR1000_I2C_Write_Data(1);
}



void AR1000_SetMute(u8 mute)
{
	if(mute>0)
		AR1000_MUTE_ON   // Set Muto ON before TUNE  
	else
		AR1000_MUTE_OFF
	AR1000_I2C_Write_Data(1);
}
/*
自动搜台
*/
bool AR1000_ValidStop(u16 fre)
{	// use the native seek capibility of AR1000 to accomplish  scan function 
  unsigned int status;
  unsigned int flag=0;
  u8 cnt=0;

//  sys_clock_pll();

  //AR1000_MUTE_ON   // Set Muto ON before SCAN
  //AR1000_I2C_Write_Data(1);
//Reg_Data[3].i=0xa008; 
 //AR1000_I2C_Write_Data(3);

  // Setting before seek
  Reg_Data[17].i = (Reg_Data[17].i & SEEK_MASK)|SEEK_SETTING;
  AR1000_I2C_Write_Data(17);
  // 

  AR1000_TUNE_OFF  //clear TUNE, just make sure tune if OFF
  Reg_Data[2].i &=0xfe00;
  Reg_Data[2].i|=(fre-690-1); // set 87.5 MHz as starting point for this scan  
  AR1000_I2C_Write_Data(2);
  
	AR1000_SEEK_OFF;	//clear SEEK 
	AR1000_I2C_Write_Data(3);      
 
//	sys_clock_rc();

	AR1000_SEEK_ON	 
	Reg_Data[3].BIT.B13=SPACE100K;  // set SPACE 
	Reg_Data[3].BIT.B15=SEEKUP; // Seek up or down
	Reg_Data[3].i = (Reg_Data[3].i & 0xE7FF) | JWBAND;  // Set BAND
	AR1000_I2C_Write_Data(3); // set  and seek
   

	status = AR1000_I2C_Read_Data(ADDR_STATUS);
	flag = status & MASK_STC; // check STC flag 
	while( flag == 0)
	{
		// maybe you can delay for a while
		// delay ( 100 ms )
			delay_10ms(1);
                   if(++cnt>200)
			break;
		status = AR1000_I2C_Read_Data(ADDR_STATUS);
		flag = status & MASK_STC; // check STC flag 
	}
	// Seek Ends 
	// check SF if seek fail ?   0 means successful seek
	flag = status & MASK_SF;
  
  // Restore setting after seek
  Reg_Data[17].i = AR1000reg[17];
  AR1000_I2C_Write_Data(17);
  
//AR1000_MUTE_OFF      
// AR1000_I2C_Write_Data(1);

   if(flag==0)
     return 1;
   else
     return 0;
}
//
bool set_fre_AR1000(u16 fre, u8 mode)
{
	if(mode)
	{
		AR1000_I2C_TUNE(fre);
		return 0;	
	}
	else
	{
		return  AR1000_ValidStop(fre);
	}
}
//iic读取ID号
bool  AR1000_Get_ID(void)
{
	u16 ChipID = 0xFFFF;
		// Get IC version
	ChipID = AR1000_I2C_Read_Data(ADDR_CHIPID);
	//deg_str(" ar1000_get_id  \r\n ");

	//printf_u16(ChipID,'i');

	if( ChipID != 0x1010 ) {
		return 0;
	}
	return 1;
}
u8 _code  AR1000vol[17]={ // volume control
	 0x0F,	// step 0
	 0x10,	// step 1
	 0x20,	// step 2
	 0x30,	// 3
	 0x40,	// 4
	 0x50,	// 5
	 0x60,	// 6
	 0x70,	// 7
	 0x80,	// 8
	 0x90,	// 9
	 0xA0,	//10
	 0xB0,	//11
	 0xC0,	//12
	 0xD0,	//13
	 0xE0,	//14
	 0xF0	//15 <------ default setting, MAX volume level.
};

// set volume
void AR1000_SetVolume (u8 vol)
{
	u16 dataRead;	
	//printf(" vol=%d ",(u16)vol);

	// AR1018_ReadByte(3);
	 dataRead =AR1000_I2C_Read_Data(3) ;//(uint16)(dataM<<8|dataL);

	// AR1018_WriteByte(3, (dataRead&0xF87F)|(u16)((AR1000vol[vol]&0x0F)<<7));
		 Reg_Data[3].i=(dataRead&0xF87F)|(u16)((AR1000vol[vol]&0x0F)<<7);
	AR1000_I2C_Write_Data(3);
	// AR1018_ReadByte(14);
	 dataRead =AR1000_I2C_Read_Data(14) ;// dataRead = (uint16)(dataM<<8|dataL);
	// AR1018_WriteByte(14, (dataRead&0x0FFF)|(u16)((AR1000vol[vol]&0xF0)<<8));
			 Reg_Data[14].i=(dataRead&0x0FFF)|(u16)((AR1000vol[vol]&0xF0)<<8);
	//printf(" Reg_Data[14]=%x ",(u16)Reg_Data[14].i);

	 	AR1000_I2C_Write_Data(14);

}
void AR1015_setch(u8 db)
{
	db=db;
}
#endif
