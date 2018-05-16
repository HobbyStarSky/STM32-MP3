#include "sys.h"
#include "delay.h"
#include "usart.h" 
#include "led.h" 		 	 
#include "lcd.h"  
#include "key.h"      
#include "malloc.h"
#include "sdio_sdcard.h"  
#include "w25qxx.h"    
#include "ff.h"  
#include "exfuns.h"
#include "fattester.h"
#include "text.h"
#include "mp3player.h"
#include "vs10xx.h"
#include "remote.h"
#include "piclib.h"	
#include "decoded.h"
 
/*********************************************************************************	 
	Music Player	

* Based on WarShip V3
*	Remote Control
* Picture Interface
* Multiple Play Mode
* Configure Sound Effects
* Display all Play Informations
* Need a SD_Card(Stored Musics and Pictures)

**********************************************************************************/

int main(void)
{	 
	u8 key = KEY_UP;
	u16 t;
	u8 res;
	
	delay_init();	    	 //延时函数初始化	  
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置中断优先级分组为组2：2位抢占优先级，2位响应优先级
	uart_init(115200);	 	//串口初始化为115200	
 	LED_Init();		  			//初始化与LED连接的硬件接口
	KEY_Init();					//初始化按键
	LCD_Init();			   		//初始化LCD   
	W25QXX_Init();				//初始化W25Q128
	VS_Init();	  				//初始化VS1053 
	Remote_Init();				//红外遥控初始化
 	my_mem_init(SRAMIN);		//初始化内部内存池
	piclib_init();					//初始化画图	
	
 	POINT_COLOR = WHITE;			//设置字体为白色 
	BACK_COLOR = BLACK;	//背景为黑色
	 
	LCD_Clear(BLACK);	//清屏
	LCD_ShowString(30,20,200,16,16,"System Initializing... ",1);
	delay_ms(500);
	
	LCD_ShowString(30,40,200,16,16,"SD Card Checking...",1);
	while(SD_Init())//检测不到SD卡
	{
		LCD_ShowString(30,60,200,16,16,"SD Card Error!",0);
		delay_ms(500);					
		LCD_ShowString(30,60,200,16,16,"Please Check! ",0);
		delay_ms(500);
		LED0=!LED0;//DS0闪烁
	}
	delay_ms(500);
	LCD_ShowString(30,60,200,16,16,"SD Card OK!     ",0);
	
 	exfuns_init();							//为fatfs相关变量申请内存	
	f_mount(fs[0],"0:",1); 					//挂载SD卡，初始化时标记的卷为"0"
	f_mount(fs[1],"1:",1); 					//挂载FLASH,初始化时标记的卷为"1"
	
	delay_ms(500);
	LCD_ShowString(30,80,200,16,16,"Font Checking...     ",1);			  
	while(font_init()) 				//检查字库
	{	    
		LCD_ShowString(30,100,200,16,16,"Font Error!        ",0);
		delay_ms(1000);	
		update_SDCard_font("0:/");
		LCD_ShowString(30,100,200,16,16,"Update Font...     ",0);
	}
	while(flac_init()) 				//检查FLAC
	{	    
		delay_ms(1000);	
		LCD_ShowString(30,100,200,16,16,"Update Flac...     ",0);
		update_flac("0:/");	//更新FLAC
	}
	delay_ms(500);
	LCD_ShowString(30,100,200,16,16,"Font and Flac OK!        ",0);
	
	delay_ms(500);
	LCD_ShowString(30,120,200,16,16,"Load Windows... ",1);
	
	delay_ms(500);
	res = Show_Decoded_Picture(0,"0:/"); //检测图片文件是否正常
	if(res) 
	{
		LCD_ShowString(30,140,300,16,16,"Picture Error! NOW update windows",1);
		delay_ms(1000);
		Load_Windows(160,1);	//更新桌面
	}
	else
	{
		LCD_ShowString(30,140,200,16,16,"update windows:WAKE_UP",1);
		for(t=0;t<300;t++)
		{
			key = KEY_Scan(0);
			if(key==WKUP_PRES)	
			{
				Load_Windows(160,1);	//更新桌面
				break;
			}
			delay_ms(10);
		}
		if(key==KEY_UP)	Load_Windows(160,0);	//加载桌面
	}
	
	delay_ms(500);
	LCD_ShowString(30,180,200,16,16,"Windows OK!    ",1);
	
	delay_ms(500);
	LCD_ShowString(30,200,200,16,16,"System Initialize Over! ",1);
	delay_ms(1500);
	Load_Windows(200,2);	//加载桌面
	
	Picture_Fill(100,100,48,16,"0:/");
	while(1)
	{
		POINT_COLOR = PURPLEBLUE;			//设置字体颜色
		mp3_play();		//播放音乐（按播放键开始播放）
	} 
}
















