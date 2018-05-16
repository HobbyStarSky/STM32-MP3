#ifndef __MP3PLAYER_H
#define __MP3PLAYER_H
#include "sys.h"
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEKս��STM32������V3
//MP3�������� ����	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��������:2015/1/20
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2009-2019
//All rights reserved								  						    								  
//////////////////////////////////////////////////////////////////////////////////

#define POWER 			0x01 	//������ʾ��
#define PLAY  			0x02	//��ͣ/��ʼ����
#define PLAY_UP			0x03	//��Ŀ��
#define PLAY_DOWN		0x04	//��Ŀ��
#define VOL_UP			0x05	//������
#define VOL_DOWN		0x06	//������
#define MODE				0x07	//�л�ģʽ
#define MODE_UP			0x08	//ģʽ��
#define MODE_DOWN		0x09	//ģʽ��
#define NUM_0				0x0A	//��Ŀ0
#define NUM_1				0x0B	//��Ŀ1
#define NUM_2				0x0C	//��Ŀ2
#define NUM_3				0x0D	//��Ŀ3
#define NUM_4				0x0E	//��Ŀ4
#define NUM_5				0x0F	//��Ŀ5
#define NUM_6				0x10	//��Ŀ6
#define NUM_7				0x11	//��Ŀ7
#define NUM_8				0x12	//��Ŀ8
#define NUM_9				0x13	//��Ŀ9
#define PLAY_MODE	  0x14	//����ģʽ
#define KEY_UP			0x00	//�����ް�������

#define PLAY_STOP		0xFF		//��ͣ
#define PLAY_START	0xAE		//��ʼ
#define PLAY_CLOSE	0x9E		//�ر�
#define LCD_ON			0xFD		//����ʾ��
#define LCD_OFF			0xAC		//�ر���ʾ��
#define PLAY_RANDOM	0xFB		//�������
#define PLAY_ORDER	0xFA		//˳�򲥷�
#define PLAY_SELECT	0xF9		//ѡ�񲥷�

typedef struct{
	
	u8 LCD_delay;
	u8 LCD_status;
	u8 play_mode;
	u8 play_status;
	u16 play_content;
}_player_ctr_obj;

extern _player_ctr_obj player_ctr;

void mp3_play(void);
u16 mp3_get_tnum(u8 *path);
u8 mp3_play_song(u8 *pname);
void mp3_index_show(u16 index,u16 total);
void mp3_msg_show(u32 lenth);

u8 mp3_key(void);	//�������
u8 mp3_Control(u8 key); //���ſ���
void mp3_mode_show(u8 key,u8 mode);	//��Чģʽ��ʾ
void mp3_progress_show(u16 now_time,u16 all_time);	//��ʾ������
#endif












