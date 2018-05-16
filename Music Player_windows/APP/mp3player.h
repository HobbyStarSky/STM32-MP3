#ifndef __MP3PLAYER_H
#define __MP3PLAYER_H
#include "sys.h"
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK战舰STM32开发板V3
//MP3播放驱动 代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//创建日期:2015/1/20
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2009-2019
//All rights reserved								  						    								  
//////////////////////////////////////////////////////////////////////////////////

#define POWER 			0x01 	//开关显示屏
#define PLAY  			0x02	//暂停/开始播放
#define PLAY_UP			0x03	//曲目加
#define PLAY_DOWN		0x04	//曲目减
#define VOL_UP			0x05	//音量加
#define VOL_DOWN		0x06	//音量减
#define MODE				0x07	//切换模式
#define MODE_UP			0x08	//模式加
#define MODE_DOWN		0x09	//模式减
#define NUM_0				0x0A	//曲目0
#define NUM_1				0x0B	//曲目1
#define NUM_2				0x0C	//曲目2
#define NUM_3				0x0D	//曲目3
#define NUM_4				0x0E	//曲目4
#define NUM_5				0x0F	//曲目5
#define NUM_6				0x10	//曲目6
#define NUM_7				0x11	//曲目7
#define NUM_8				0x12	//曲目8
#define NUM_9				0x13	//曲目9
#define PLAY_MODE	  0x14	//播放模式
#define KEY_UP			0x00	//撤销无按键按下

#define PLAY_STOP		0xFF		//暂停
#define PLAY_START	0xAE		//开始
#define PLAY_CLOSE	0x9E		//关闭
#define LCD_ON			0xFD		//打开显示器
#define LCD_OFF			0xAC		//关闭显示器
#define PLAY_RANDOM	0xFB		//随机播放
#define PLAY_ORDER	0xFA		//顺序播放
#define PLAY_SELECT	0xF9		//选择播放

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

u8 mp3_key(void);	//按键检测
u8 mp3_Control(u8 key); //播放控制
void mp3_mode_show(u8 key,u8 mode);	//音效模式显示
void mp3_progress_show(u16 now_time,u16 all_time);	//显示进度条
#endif












