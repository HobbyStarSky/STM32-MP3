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
	
	delay_init();	    	 //��ʱ������ʼ��	  
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//�����ж����ȼ�����Ϊ��2��2λ��ռ���ȼ���2λ��Ӧ���ȼ�
	uart_init(115200);	 	//���ڳ�ʼ��Ϊ115200	
 	LED_Init();		  			//��ʼ����LED���ӵ�Ӳ���ӿ�
	KEY_Init();					//��ʼ������
	LCD_Init();			   		//��ʼ��LCD   
	W25QXX_Init();				//��ʼ��W25Q128
	VS_Init();	  				//��ʼ��VS1053 
	Remote_Init();				//����ң�س�ʼ��
 	my_mem_init(SRAMIN);		//��ʼ���ڲ��ڴ��
	piclib_init();					//��ʼ����ͼ	
	
 	POINT_COLOR = WHITE;			//��������Ϊ��ɫ 
	BACK_COLOR = BLACK;	//����Ϊ��ɫ
	 
	LCD_Clear(BLACK);	//����
	LCD_ShowString(30,20,200,16,16,"System Initializing... ",1);
	delay_ms(500);
	
	LCD_ShowString(30,40,200,16,16,"SD Card Checking...",1);
	while(SD_Init())//��ⲻ��SD��
	{
		LCD_ShowString(30,60,200,16,16,"SD Card Error!",0);
		delay_ms(500);					
		LCD_ShowString(30,60,200,16,16,"Please Check! ",0);
		delay_ms(500);
		LED0=!LED0;//DS0��˸
	}
	delay_ms(500);
	LCD_ShowString(30,60,200,16,16,"SD Card OK!     ",0);
	
 	exfuns_init();							//Ϊfatfs��ر��������ڴ�	
	f_mount(fs[0],"0:",1); 					//����SD������ʼ��ʱ��ǵľ�Ϊ"0"
	f_mount(fs[1],"1:",1); 					//����FLASH,��ʼ��ʱ��ǵľ�Ϊ"1"
	
	delay_ms(500);
	LCD_ShowString(30,80,200,16,16,"Font Checking...     ",1);			  
	while(font_init()) 				//����ֿ�
	{	    
		LCD_ShowString(30,100,200,16,16,"Font Error!        ",0);
		delay_ms(1000);	
		update_SDCard_font("0:/");
		LCD_ShowString(30,100,200,16,16,"Update Font...     ",0);
	}
	while(flac_init()) 				//���FLAC
	{	    
		delay_ms(1000);	
		LCD_ShowString(30,100,200,16,16,"Update Flac...     ",0);
		update_flac("0:/");	//����FLAC
	}
	delay_ms(500);
	LCD_ShowString(30,100,200,16,16,"Font and Flac OK!        ",0);
	
	delay_ms(500);
	LCD_ShowString(30,120,200,16,16,"Load Windows... ",1);
	
	delay_ms(500);
	res = Show_Decoded_Picture(0,"0:/"); //���ͼƬ�ļ��Ƿ�����
	if(res) 
	{
		LCD_ShowString(30,140,300,16,16,"Picture Error! NOW update windows",1);
		delay_ms(1000);
		Load_Windows(160,1);	//��������
	}
	else
	{
		LCD_ShowString(30,140,200,16,16,"update windows:WAKE_UP",1);
		for(t=0;t<300;t++)
		{
			key = KEY_Scan(0);
			if(key==WKUP_PRES)	
			{
				Load_Windows(160,1);	//��������
				break;
			}
			delay_ms(10);
		}
		if(key==KEY_UP)	Load_Windows(160,0);	//��������
	}
	
	delay_ms(500);
	LCD_ShowString(30,180,200,16,16,"Windows OK!    ",1);
	
	delay_ms(500);
	LCD_ShowString(30,200,200,16,16,"System Initialize Over! ",1);
	delay_ms(1500);
	Load_Windows(200,2);	//��������
	
	Picture_Fill(100,100,48,16,"0:/");
	while(1)
	{
		POINT_COLOR = PURPLEBLUE;			//����������ɫ
		mp3_play();		//�������֣������ż���ʼ���ţ�
	} 
}
















