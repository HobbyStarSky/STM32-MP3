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
 
/************************************************
 ALIENTEKս��STM32������ʵ��39
 FATFS�ļ�ϵͳʵ�� 
 ����֧�֣�www.openedv.com
 �Ա����̣�http://eboard.taobao.com 
 ��ע΢�Ź���ƽ̨΢�źţ�"����ԭ��"����ѻ�ȡSTM32���ϡ�
 ������������ӿƼ����޹�˾  
 ���ߣ�����ԭ�� @ALIENTEK
************************************************/



int main(void)
{	 
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
 	POINT_COLOR=RED;			//��������Ϊ��ɫ  
	 
	while(SD_Init())//��ⲻ��SD��
	{
		LCD_ShowString(30,150,200,16,16,"SD Card Error!");
		delay_ms(500);					
		LCD_ShowString(30,150,200,16,16,"Please Check! ");
		delay_ms(500);
		LED0=!LED0;//DS0��˸
	}
 	exfuns_init();							//Ϊfatfs��ر��������ڴ�	
	f_mount(fs[0],"0:",1); 					//����SD������ʼ��ʱ��ǵľ�Ϊ"0"
	f_mount(fs[1],"1:",1); 					//����FLASH,��ʼ��ʱ��ǵľ�Ϊ"1"
	
	LCD_Fill(30,150,240,150+16,WHITE);		//�����ʾ			  
	while(font_init()) 				//����ֿ�
	{	    
		LCD_ShowString(30,50,200,16,16,"Font Error!");
		delay_ms(200);				  	     
	}
	while(flac_init()) 				//���FLAC
	{	    
		LCD_ShowString(30,50,200,16,16,"Flac Error!");
		delay_ms(1000);				  	
		update_flac("0:/");	//����FLAC
	}
	LCD_Fill(30,50,240,66,WHITE);//�����ʾ
 	Show_Str(30,50,200,16,"ս�� STM32������",16,0);				    	 
	Show_Str(30,70,200,16,"���ֲ�����ʵ��",16,0);				    	 
	Show_Str(30,90,200,16,"����ԭ��@ALIENTEK",16,0);				    	 
	Show_Str(30,110,200,16,"2016��5��24��",16,0);
	Show_Str(30,130,200,16,"KEY0:NEXT   KEY2:PREV",16,0);
	Show_Str(30,150,200,16,"KEY_UP:VOL+ KEY1:VOL-",16,0);
	
	while(1)
	{
		mp3_play();		//�������֣������ż���ʼ���ţ�
	} 
}
















