/*--------------------------------------------------------------------------*/
/**@file    fm_receive.c
   @brief   FM��������ģ��
   @details
   @author
   @date   2011-3-7
   @note
*/
/*----------------------------------------------------------------------------*/
#include "config.h"

#include "fm_rev.h"

#if (FM_MODULE == 1)
#include "disp.h"
#include "key.h"
#include "msgfor_hot.h"
#include "main.h"
#include "fm_api.h"
#include "dac.h"
#include "rtc_api.h"
#include "eq.h"

#if defined(FM_USE_QN8035)
extern void QND_TuneToCH(UINT16 ch);
#endif

extern bool input_number_en;
extern u8 main_menu, cur_menu, main_menu_conter;
extern u16 input_number;
extern xd_u8 work_mode;
extern u8 _idata last_work_mode;

extern u8 fm_addr;
extern u8 _code one_table[];
u16 frequency;                          ///<��ǰƵ��
u8 fre_channel; 			///<��̨��
u8 total_channel;///<��ǰƵ��
extern  bool vol_change_en,sys_mute_flag;
extern u8 eq_mode;
extern u8 _xdata decode_buffer[];


/*----------------------------------------------------------------------------*/
/**@brief ��ȡȫ����¼��Ƶ��
   @param 	��
   @return  Ƶ������
   @note  u8 get_total_mem_channel(void)
*/
/*----------------------------------------------------------------------------*/
u8 get_total_mem_channel(void)
{
    u8 i,j;
    u8 total;

    total = 0;
    for (i = 0; i < MEM_FM_LEN; i++)
    {
        j = read_info(MEM_CHANNL + i);
        total += one_table[j & 0x0f];
        total += one_table[(j >> 4) & 0x0f];
    }

    if (total > MAX_CHANNL)
        total = MAX_CHANNL;

    return total;
}
/*----------------------------------------------------------------------------*/
/**@brief ͨ��Ƶ����ȡƵ��
   @param 	channel��Ƶ��
   @return  ��Ч��Ƶ��ƫ����
   @note  u8 get_fre_via_channle(u8 channel)
*/
/*----------------------------------------------------------------------------*/
u8 get_fre_via_channle(u8 channel)
{
    u8 i,j, k;
    u8 total;

    total = 0;
    for (i = 0; i < MEM_FM_LEN; i++)
    {
        j = read_info(MEM_CHANNL + i);
        for (k = 0; k < 8; k++)
        {
            if (j & (BIT(k)))
            {
                total++;
                if (total == channel)
                {
                    return i * 8 + k;		 //fre = MIN_FRE + return val
                }
            }
        }

    }
    return 0xff;							//find none
}
/*----------------------------------------------------------------------------*/
/**@brief ����Ƶ��ƫ������ȡƵ��
   @param 	channel��Ƶ��
   @return  Ƶ��
   @note  u8 get_channel_via_fre(u8 fre)
*/
/*----------------------------------------------------------------------------*/
u8 get_channel_via_fre(u8 fre)
{
    u8 i,j, k;
    u8 total;

    total = 0;
    for (i = 0; i < MEM_FM_LEN; i++)
    {
        j = read_info(MEM_CHANNL + i);
        for (k = 0; k < 8; k++)
        {
            if (j & (BIT(k)))
            {
                total++;
                if (fre == (i * 8 + k))
                {
                    return total;		 //return fre index
                }
            }
        }
    }
    return fre_channel;						    //find none
}
/*----------------------------------------------------------------------------*/
/**@brief ����Ƶ��ƫ�������浽��Ӧ��Ƶ��λ������EEPROM
   @param 	fre��Ƶ��ƫ����
   @return  ��
   @note  void save_fm_point(u8 fre)
*/
/*----------------------------------------------------------------------------*/
void save_fm_point(u8 fre)
{
    u8 i,j, k;

    i = fre / 8;
    k = fre % 8;

    j = read_info(MEM_CHANNL + i) | BIT(k);
    write_info(MEM_CHANNL + i, j);
}
/*----------------------------------------------------------------------------*/
/**@brief ��EEPROM�������Ƶ����Ϣ
   @param 	��
   @return  ��
   @note  void clear_all_fm_point(void)
*/
/*----------------------------------------------------------------------------*/
void clear_all_fm_point(void)
{
    u8 i;

    for (i = MEM_CHANNL; i <= (MEM_CHANNL + MEM_FM_LEN); i++)
    {
        write_info(i ,0);
    }
}
/*----------------------------------------------------------------------------*/
/**@brief ����Ƶ��
   @param 	��
   @return  ��
   @note  u8 ch_save(void)
*/
/*----------------------------------------------------------------------------*/
u8 ch_save(void)
{
    u8 i;
    u16 check_frequency;

    for (i = 0; i < total_channel; i++)
    {
        check_frequency = get_fre_via_channle(i) + MIN_FRE;
        if (frequency == check_frequency)
        {
            return 1;
        }
    }
    save_fm_point(frequency - MIN_FRE);
    fre_channel = get_channel_via_fre(frequency - MIN_FRE);
    total_channel = get_total_mem_channel();
    disp_port(main_menu);

    return 0;
}

/*----------------------------------------------------------------------------*/
/**@brief  fm_scan ״̬��־
   @param  flag=1 ��־�˽�����̨״̬   flag=0��������̨״̬
   @return ��
   @note   void enter_fm_rev(void)
*/
/*----------------------------------------------------------------------------*/
void fm_scan_flag(bool flag)
{
    if (flag)
    {
        write_info(MEM_CHAN,fre_channel|0x80);
    }
    else
    {
        write_info(MEM_CHAN,fre_channel&0x7f);
    }
}

/*----------------------------------------------------------------------------*/
/**@brief  ��ȡFM�������Ϣ
   @param  ��
   @return ��
   @note   void enter_fm_rev(void)
*/
/*----------------------------------------------------------------------------*/
void enter_fm_rev(void)
{
    frequency = read_info(MEM_FRE);
    if (frequency > (1085-875))
    {
        frequency = MIN_FRE;
    }
    else
    {
        frequency += MIN_FRE;
    }

    total_channel = get_total_mem_channel();
    if (!total_channel)
    {
        total_channel = 1;
    }

    fre_channel=read_info(MEM_CHAN);
    if (fre_channel > 206)					//�ж�eeprom�Ƿ��һ���ã�����̨��Ϊ1;��̨��Ϊ1
    {
        fre_channel=1;
        total_channel=1;
    }
    else if (!fre_channel)
    {
        fre_channel=1;
    }

    fre_channel = get_channel_via_fre(frequency - MIN_FRE);
    if (0xff == fre_channel)
    {
        fre_channel = 1;
    }
    set_fre(FM_CUR_FRE);
}

/*----------------------------------------------------------------------------*/
/**@brief    ȫƵ������
   @param    mode: 0ȫƵ��, 1:��һ����ЧƵ�㣬 2:��һ����ЧƵ��
   @return   0:δ�ҵ�Ƶ�㣬1:�������,2:�˳�FMģʽ
   @note     u8 scan_fre(u8)
*/
/*----------------------------------------------------------------------------*/
u8 fm_scan(u8 mode)
{
    u8 i;
    u8 key;
    u8 result = 0;
    vol_change_en=0;
    input_number_en = 0;
    if (mode == 0)
    {
        write_info(MEM_CHAN,0);
        write_info(MEM_FRE,0);
        clear_all_fm_point();
        total_channel = 0;
        fre_channel = 0;
        frequency = MIN_FRE - 1;					//�Զ���������͵�Ƶ�㿪ʼ
    }
    fm_set_ch(4);

    for (i = 0;i <= (MAX_FRE-MIN_FRE);i++)
    {

        while (1)
        {
            key = app_get_msg();
            if (key == NO_MSG)
                break;

            switch (key)
            {
            case MSG_MUSIC_NEW_DEVICE_IN:							//�����豸����
                work_mode = MUSIC_MODE;
                result = 2;
                goto _fm_scan_ret;
				
#ifndef NOT_USE_LINE_IN_FUNC
            case MSG_AUX_IN :
                work_mode = AUX_MODE;
                result = 2;
                goto _fm_scan_ret;
#endif

#if USB_DEVICE_ENABLE
            case MSG_USB_PC_IN:
                work_mode = USB_DEVICE_MODE;
                result = 2;
                goto _fm_scan_ret;
#endif
	     case MSG_FM_PP:
	     case MSG_FM_NEXT_STATION:
	     case MSG_FM_PREV_STATION:
            case MSG_FM_SCAN_ALL:
                result = 0;
                goto _fm_scan_ret;
            //case MSG_ALM_ON:
             //   result = 3;
             //   goto _fm_scan_ret;

            default:
                ap_handle_hotkey(key);
                break;
            }
        }

        if (mode ==1)
        {
            frequency--;
        }
        else
        {
            frequency++;
        }
        fm_module_mute(1);
        disp_port(MENU_FM_DISP_FRE);
        if (set_fre(FM_CUR_FRE))							//�ҵ�һ��̨
        {
            fm_module_mute(0);
            if (mode == 0)								//�������е�̨
            {

                save_fm_point(frequency - MIN_FRE);
                fre_channel = get_channel_via_fre(frequency - MIN_FRE);
                total_channel = get_total_mem_channel();
                disp_port(MENU_FM_FIND_STATION);
				
#ifdef THREE_SEC_AT_VALID_STATION
                delay_10ms(300);
#else
                delay_10ms(100);
#endif
            }
            else
            {
                save_fm_point(frequency - MIN_FRE);
                fre_channel = get_channel_via_fre(frequency - MIN_FRE);
                total_channel = get_total_mem_channel();

                disp_port(MENU_FM_DISP_FRE);
                result = 1;
                goto _fm_scan_ret;
            }
        }

    }
_fm_scan_ret:
    fm_module_mute(0);
    input_number_en = 1;
    vol_change_en = 1;
    return result;
}

/*----------------------------------------------------------------------------*/
/**@brief  FM������
   @param  ��
   @return ��
   @note   void fm_rev( void )
*/
/*----------------------------------------------------------------------------*/
void fm_rev( void )
{
    u8 key;
	
    ext_amp_mute(EXT_AMP_UNMUTE);	

    while (1)
    {
#if defined(EXCHANGE_AUX_CHANNEL)
        dac_out_select(DAC_AMUX1, 0);
#else
        dac_out_select(DAC_AMUX0, 0);
#endif
        key = app_get_msg();

        switch (key)
        {
        case MSG_CHANGE_WORK_MODE:
            return;

        case MSG_MUSIC_NEW_DEVICE_IN:							//�����豸����
            work_mode = MUSIC_MODE;
            return;

        case MSG_FM_SCAN_ALL:								//play �����Զ�ȫƵ����

    	     flashled(LED_SPARK_FAST);
            key = fm_scan(0);	
	     flashled(LED_ON);
            if (key == 3)
            {
                put_msg_lifo(MSG_ALM_ON);
                break;
            }
            if (key == 2)
                return;
            put_msg_lifo(MSG_FM_NEXT_STATION);
            break;

        case  MSG_FM_SCAN_ALL_DOWN :
            key = fm_scan(1);
            if (key == 3)
            {
                put_msg_lifo(MSG_ALM_ON);
                break;
            }
            if (key == 2)
                return;
            break;

        case  MSG_FM_SCAN_ALL_UP:
            key =fm_scan(2);
            if (key == 3)
            {
                put_msg_lifo(MSG_ALM_ON);
                break;
            }
            if (key == 2)
                return;
            break;
#ifdef FM_PLAY_KEY_FOR_PAUSE
        case MSG_MUSIC_PP:									//play �̰�����
		sys_mute_flag=~sys_mute_flag;
        	dac_mute_control(sys_mute_flag,1);					//��������ʱ���Զ�UNMUTE
            	disp_port(MENU_FM_MAIN);
            break;
#endif
       // case MSG_MUTE:
        //    break;

       // case MSG_MUSIC_FR:									//������һ��̨
        //    flush_all_msg();
        //    break;

      //  case MSG_MUSIC_FF:									//������һ��̨
      //      flush_all_msg();
      //      break;

        case MSG_FM_PREV_STEP:
            set_fre(FM_FRE_DEC);
            fre_channel = get_channel_via_fre(frequency - MIN_FRE);						//���Ҹ�Ƶ���Ƿ��м����
            disp_port(MENU_FM_MAIN);
            write_info(MEM_FRE, frequency-MIN_FRE);
            write_info(MEM_CHAN, fre_channel);
            break;

        case MSG_FM_NEXT_STEP:
            set_fre(FM_FRE_INC);
            fre_channel = get_channel_via_fre(frequency - MIN_FRE);						//���Ҹ�Ƶ���Ƿ��м����
            disp_port(MENU_FM_MAIN);
            write_info(MEM_FRE, frequency-MIN_FRE);
            write_info(MEM_CHAN, fre_channel);
            break;

        case MSG_FM_PREV_STATION:
            if (total_channel == 0)
                break;
            fre_channel -= 2;
        case MSG_FM_NEXT_STATION:
            if (total_channel == 0)
                break;
            fre_channel++;

            if ((fre_channel == 0) || (fre_channel == 0xff))
            {
                fre_channel = total_channel;
            }
            else if (fre_channel > total_channel)
            {
                fre_channel = 1;
            }
            frequency = get_fre_via_channle(fre_channel) + MIN_FRE;				//����̨����Ƶ��
            main_vol_set(0, CHANGE_VOL_NO_MEM);
            set_fre(FM_CUR_FRE);
            main_vol_set(0, SET_USE_CURRENT_VOL);
            disp_port(MENU_FM_CHANNEL);
            write_info(MEM_FRE, frequency-MIN_FRE);
            write_info(MEM_CHAN, fre_channel);
            break;

        case MSG_CH_SET:
            if (cur_menu == MENU_INPUT_NUMBER)			//��������ģʽ
            {
                if (input_number <= MAX_CHANNL)							//�������̨��
                {
                    key = get_fre_via_channle(input_number);
                    if (key != 0xff)
                    {
                        frequency = key + MIN_FRE;
                        fre_channel = input_number;
                        main_vol_set(0, CHANGE_VOL_NO_MEM);
                        set_fre(FM_CUR_FRE);
                        main_vol_set(0, SET_USE_CURRENT_VOL);
                        disp_port(MENU_FM_CHANNEL);
                        //break;
                    }

                }
                else if ((input_number >= MIN_FRE) && (input_number <= MAX_FRE)) //�������Ƶ��
                {
                    frequency = input_number;
                    fre_channel = get_channel_via_fre(frequency - MIN_FRE);
                    main_vol_set(0, CHANGE_VOL_NO_MEM);
                    set_fre(FM_CUR_FRE);
                    main_vol_set(0, SET_USE_CURRENT_VOL);
                }
            }
            write_info(MEM_FRE, frequency-MIN_FRE);
            write_info(MEM_CHAN, fre_channel);
            disp_port(main_menu);

            break;

        case MSG_CH_SAVE:
            ch_save();
            break;


        case MSG_HALF_SECOND:

#if defined(USE_BAT_MANAGEMENT)
		bmt_hdlr();
#endif

            set_brightness_fade_out();
            if (main_menu_conter < SUB_MENU_TIME)
            {
                main_menu_conter++;
            }
            else if (cur_menu != main_menu)
            {
                put_msg_lifo(MSG_CH_SET);				//��ת��CH_SET��Ϣ����
            }
			
	     if(RECODE_WORKING == encode_status)
            {
                 //   cur_menu = MENU_RECWORKING;
		   disp_port(MENU_RECWORKING); 
					
            }			
            break;


#if RTC_ENABLE
        case MSG_ALM_ON:
            write_next_alm_sec();
            work_mode = RTC_MODE;
            put_msg_lifo(MSG_CHANGE_WORK_MODE);
            break;
#endif
        default :
            ap_handle_hotkey(key);
            break;
        }
    }
}

/*----------------------------------------------------------------------------*/
/**@brief FMģʽ��ں���
   @param 	��
   @return  ��
   @note  void fm_radio(void)
*/
/*----------------------------------------------------------------------------*/
void fm_radio(void)
{

#ifdef USE_AMP_MODE_SELECT
	D_CLASS_AMP_GPIO_INIT(); 
	D_CLASS_AMP_LOW();
#endif
//    printf("enter fm\n");
    enable_key_tone_flag(1);

    flashled(LED_ON);
#ifndef INDEPENDENT_FM_OSC
    P05_source_select(P05_SEL_OSC1);
#endif
    amux_dsp_eq();

    SYSTEM_CLK_DIV2();
    //if(1)
    if (init_fm_rev())
    {
    	
    }
    else					// no fm module
    {
#ifdef NO_FM_CHIP_GOTO_MP3
        work_mode = IDLE_MODE;
#else
        work_mode++;
#endif
        return;
    }

    input_number_en = 1;
    vol_change_en=1;
    main_menu = MENU_FM_MAIN;
    key_table_sel(1);
    flush_all_msg();
    enter_fm_rev();
    disp_port(MENU_FM_MAIN);
	SYSTEM_CLK_DIV4();
    set_max_vol(MAX_ANOLOG_VOL,MAX_DIGITAL_VOL);///�����������
	//dac_out_select(DAC_AMUX1, 0);
    encode_channel = REC_FM;
    encode_vol = 3;
    set_fre(FM_CUR_FRE);
#if defined(FM_USE_QN8035)
    QND_TuneToCH(frequency*10);
#endif
    fm_rev();
    main_vol_set(0, CHANGE_VOL_NO_MEM);
    fm_rev_powerdown();
#ifdef FM_PLAY_KEY_FOR_PAUSE
    sys_mute_flag=0;
    dac_mute_control(0,1);
#endif
    P05_source_select(P05_SEL_IO);
#ifdef USE_AMP_MODE_SELECT
	D_CLASS_AMP_HIGH();
#endif	
    break_encode();
}

#endif
