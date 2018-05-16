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

#define POWER 			1 	//������ʾ��
#define PLAY  			2		//��ͣ/��ʼ����
#define PLAY_UP			3		//��Ŀ��
#define PLAY_DOWN		4		//��Ŀ��
#define VOL_UP			5		//������
#define VOL_DOWN		6		//������
#define MODE				7		//�л�ģʽ
#define MODE_UP			8		//ģʽ��
#define MODE_DOWN		9		//ģʽ��
#define NUM_0				10	//��Ŀ0
#define NUM_1				11	//��Ŀ1
#define NUM_2				12	//��Ŀ2
#define NUM_3				13	//��Ŀ3
#define NUM_4				14	//��Ŀ4
#define NUM_5				15	//��Ŀ5
#define NUM_6				16	//��Ŀ6
#define NUM_7				17	//��Ŀ7
#define NUM_8				18	//��Ŀ8
#define NUM_9				19	//��Ŀ9
#define PLAY_MODE	  20	//����ģʽ
#define KEY_UP			0		//�����ް�������

#define PLAY_STOP		0xFF		//��ͣ
#define PLAY_START		0xAE		//��ʼ
#define LCD_ON	0xFD		//����ʾ��
#define LCD_OFF	0xAC		//�ر���ʾ��
#define PLAY_RANDOM	0xFB		//�������
#define PLAY_ORDER	0xFA		//˳�򲥷�
#define PLAY_SELECT	0xF9		//ѡ�񲥷�

typedef struct{
	
	u8 LCD_delay;
	u8 LCD_status;
	u8 play_mode;
	u8 play_status;
}_player_ctr_obj;

extern _player_ctr_obj player_ctr;

void mp3_play(void);
u16 mp3_get_tnum(u8 *path);
u8 mp3_play_song(u8 *pname);
void mp3_index_show(u16 index,u16 total);
void mp3_msg_show(u32 lenth);

u8 mp3_key(void);	//�������
u8 mp3_Control(u8 key); //���ſ���
void mp3_mode_show(u8 mode);	//��Чģʽ��ʾ
#endif












