#include "mp3player.h"
#include "vs10xx.h"	 
#include "delay.h"
#include "led.h"
#include "key.h"
#include "lcd.h"		 
#include "malloc.h"
#include "text.h"
#include "string.h"
#include "exfuns.h"  
#include "ff.h"   
#include "usart.h"
#include "remote.h"
#include "stdlib.h"
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

_player_ctr_obj player_ctr = {
	30,				//显示器关闭延时30s
	LCD_ON,	//显示器默认打开
	PLAY_ORDER,	//默认顺序播放模式
	PLAY_STOP,			//默认停止播放
};


//显示曲目索引
//index:当前索引
//total:总文件数
void mp3_index_show(u16 index,u16 total)
{
	//显示当前曲目的索引,及总曲目数
	LCD_ShowxNum(30+0,230,index,3,16,0X80);		//索引
	LCD_ShowChar(30+24,230,'/',16,0);
	LCD_ShowxNum(30+32,230,total,3,16,0X80); 	//总曲目				  	  
}
//显示当前音量
void mp3_vol_show(u8 vol)
{			    
	LCD_ShowString(30+110,230,200,16,16,"VOL:");	  	  
	LCD_ShowxNum(30+142,230,vol,2,16,0X80); 	//显示音量	 
}
u16 f_kbps=0;//歌曲文件位率	
//显示播放时间,比特率 信息 
//lenth:歌曲总长度
void mp3_msg_show(u32 lenth)
{	
	static u16 playtime=0;//播放时间标记	     
 	u16 time=0;// 时间变量
	u16 temp=0;	  
	if(f_kbps==0xffff)//未更新过
	{
		playtime=0;
		f_kbps=VS_Get_HeadInfo();	   //获得比特率
	}	 	 
	time=VS_Get_DecodeTime(); //得到解码时间
	if(playtime==0)playtime=time;
	else if((time!=playtime)&&(time!=0))//1s时间到,更新显示数据
	{
		if(player_ctr.LCD_delay>0)	
		{
			player_ctr.LCD_delay--;	//延时减少
			LCD_LED = 1;	//LCD背光打开
		}
		else 
		{
			LCD_LED = 0;				//LCD背光关闭
			player_ctr.LCD_status = LCD_OFF;		//无操作显示自动关闭	
		}
		playtime=time;//更新时间 	 				    
		temp=VS_Get_HeadInfo(); //获得比特率	   				 
		if(temp!=f_kbps)
		{
			f_kbps=temp;//更新KBPS	  				     
		}			 
		//显示播放时间			 
		LCD_ShowxNum(30,210,time/60,2,16,0X80);		//分钟
		LCD_ShowChar(30+16,210,':',16,0);
		LCD_ShowxNum(30+24,210,time%60,2,16,0X80);	//秒钟		
 		LCD_ShowChar(30+40,210,'/',16,0); 	    	 
		//显示总时间
		if(f_kbps)time=(lenth/f_kbps)/125;//得到秒钟数   (文件长度(字节)/(1000/8)/比特率=持续秒钟数    	  
		else time=0;//非法位率	  
 		LCD_ShowxNum(30+48,210,time/60,2,16,0X80);	//分钟
		LCD_ShowChar(30+64,210,':',16,0);
		LCD_ShowxNum(30+72,210,time%60,2,16,0X80);	//秒钟	  		    
		//显示位率			   
   		LCD_ShowxNum(30+110,210,f_kbps,3,16,0X80); 	//显示位率	 
		LCD_ShowString(30+134,210,200,16,16,"Kbps");	  	  
		LED0=!LED0;		//DS0翻转
	}   		 
}			  		 
//得到path路径下,目标文件的总个数
//path:路径		    
//返回值:总有效文件数
u16 mp3_get_tnum(u8 *path)
{	  
	u8 res;
	u16 rval=0;
 	DIR tdir;	 		//临时目录
	FILINFO tfileinfo;	//临时文件信息		
	u8 *fn; 			 			   			     
    res=f_opendir(&tdir,(const TCHAR*)path); //打开目录
  	tfileinfo.lfsize=_MAX_LFN*2+1;						//长文件名最大长度
	tfileinfo.lfname=mymalloc(SRAMIN,tfileinfo.lfsize);	//为长文件缓存区分配内存
	if(res==FR_OK&&tfileinfo.lfname!=NULL)
	{
		while(1)//查询总的有效文件数
		{
	        res=f_readdir(&tdir,&tfileinfo);       		//读取目录下的一个文件
	        if(res!=FR_OK||tfileinfo.fname[0]==0)break;	//错误了/到末尾了,退出		  
     		fn=(u8*)(*tfileinfo.lfname?tfileinfo.lfname:tfileinfo.fname);			 
			res=f_typetell(fn);	//对比文件类型
			if((res&0XF0)==0X40)//取高四位,看看是不是音乐文件	
			{
				rval++;//有效文件数增加1
			}	    
		}  
	} 
	myfree(SRAMIN,tfileinfo.lfname);
	return rval;
}
//播放音乐
void mp3_play(void)
{
	u8 res;
 	DIR mp3dir;	 		//目录
	FILINFO mp3fileinfo;//文件信息
	u8 *fn;   			//长文件名
	u8 *pname;			//带路径的文件名
	u16 totmp3num; 		//音乐文件总数
	u16 curindex;		//图片当前索引
	u8 key;				//键值		  
 	u16 temp;
	u16 *mp3indextbl;	//音乐索引表 
 	while(f_opendir(&mp3dir,"0:/MUSIC"))//打开图片文件夹
 	{	    
		Show_Str(30,190,240,16,"MUSIC文件夹错误!",16,0);
		delay_ms(200);				  
		LCD_Fill(30,190,240,206,WHITE);//清除显示	     
		delay_ms(200);				  
	} 									  
	totmp3num=mp3_get_tnum("0:/MUSIC"); //得到总有效文件数
  while(totmp3num==NULL)//音乐文件总数为0		
 	{	    
		Show_Str(30,190,240,16,"没有音乐文件!",16,0);
		delay_ms(200);				  
		LCD_Fill(30,190,240,146,WHITE);//清除显示	     
		delay_ms(200);				  
	}										   
  mp3fileinfo.lfsize=_MAX_LFN*2+1;						//长文件名最大长度
	mp3fileinfo.lfname=mymalloc(SRAMIN,mp3fileinfo.lfsize);	//为长文件缓存区分配内存
 	pname=mymalloc(SRAMIN,mp3fileinfo.lfsize);				//为带路径的文件名分配内存
 	mp3indextbl=mymalloc(SRAMIN,2*totmp3num);				//申请2*totmp3num个字节的内存,用于存放音乐文件索引
 	while(mp3fileinfo.lfname==NULL||pname==NULL||mp3indextbl==NULL)//内存分配出错
 	{	    
		Show_Str(30,190,240,16,"内存分配失败!",16,0);
		delay_ms(200);				  
		LCD_Fill(30,190,240,146,WHITE);//清除显示	     
		delay_ms(200);				  
	}  	
	VS_HD_Reset();
	VS_Soft_Reset();
	mp3_vol_show((vsset.mvol-100)/5);	//音量限制在:100~250,显示的时候,按照公式(vol-100)/5,显示,也就是0~30   
	//记录索引
  res=f_opendir(&mp3dir,"0:/MUSIC"); //打开目录
	if(res==FR_OK)
	{
		curindex=0;//当前索引为0
		while(1)//全部查询一遍
		{
			temp=mp3dir.index;								//记录当前index
	        res=f_readdir(&mp3dir,&mp3fileinfo);       		//读取目录下的一个文件
	        if(res!=FR_OK||mp3fileinfo.fname[0]==0)break;	//错误了/到末尾了,退出		  
     		fn=(u8*)(*mp3fileinfo.lfname?mp3fileinfo.lfname:mp3fileinfo.fname);			 
			res=f_typetell(fn);	
			if((res&0XF0)==0X40)//取高四位,看看是不是音乐文件	
			{
				mp3indextbl[curindex]=temp;//记录索引
				curindex++;
			}	    
		} 
	}   
	curindex=0;											//从0开始显示
	res=f_opendir(&mp3dir,(const TCHAR*)"0:/MUSIC"); 	//打开目录
	while(res==FR_OK)//打开成功（一直在此函数里循环）
	{	
		dir_sdi(&mp3dir,mp3indextbl[curindex]);			//改变当前目录索引	   
    res=f_readdir(&mp3dir,&mp3fileinfo);       		//读取目录下的一个文件
    if(res!=FR_OK||mp3fileinfo.fname[0]==0)break;	//错误了/到末尾了,退出
   	fn=(u8*)(*mp3fileinfo.lfname?mp3fileinfo.lfname:mp3fileinfo.fname);			 
		strcpy((char*)pname,"0:/MUSIC/");				//复制路径(目录)
		strcat((char*)pname,(const char*)fn);  			//将文件名接在后面
 		LCD_Fill(30,190,240,190+16,WHITE);				//清除之前的显示
		Show_Str(30,190,240-30,16,fn,16,0);				//显示歌曲名字 
		mp3_index_show(curindex+1,totmp3num);
		key = mp3_play_song(pname); 				 		//播放这个MP3 
			
		if(key==0xFF)	break;	//产生了错误
		else 
		{
			if(player_ctr.play_mode==PLAY_RANDOM)	//随机播放模式
			{
				if((key==PLAY_UP)||(key==PLAY_DOWN)||(key==0))	
					curindex = rand()%totmp3num;		//随机播放
			}
			else 
			{
				if(key==PLAY_UP)	//上一曲
				{ 	
					if(curindex)	curindex--;
					else curindex = totmp3num-1;
				}
				else if((key==PLAY_DOWN)||(key==0))	//下一曲
				{
					curindex++;		   	
					if(curindex>=totmp3num)	curindex = 0;//到末尾的时候,自动从头开始
				}
			}
			
			if(key==PLAY)	//选歌
			{
				if((vsset.content<=totmp3num)&&(vsset.content>0))	//歌曲范围[1,歌曲数]
					curindex = vsset.content-1;	//选歌
			} 
		}
		
	} 											  
	myfree(SRAMIN,mp3fileinfo.lfname);	//释放内存			    
	myfree(SRAMIN,pname);				//释放内存			    
	myfree(SRAMIN,mp3indextbl);			//释放内存	 
}

//播放一曲指定的歌曲				     	   									    	 
//返回值:0,正常播放完成
//		 1,下一曲
//       2,上一曲
//       0XFF,出现错误了
u8 mp3_play_song(u8 *pname)
{	 
 	FIL* fmp3;
  u16 br;
	u8 key;
	u8 res,rval;	  
	u8 *databuf;	   		   
	u16 i=0; 	    
			   
	rval=0;	   
	fmp3=(FIL*)mymalloc(SRAMIN,sizeof(FIL));//申请内存
	databuf=(u8*)mymalloc(SRAMIN,4096);		//开辟4096字节的内存区域
	if(databuf==NULL||fmp3==NULL)rval=0XFF ;//内存申请失败.
	if(rval==0)
	{	  
	  VS_Restart_Play();  					//重启播放 
		VS_Set_All();        					//设置音量等信息 			 
		VS_Reset_DecodeTime();					//复位解码时间 	  
		res=f_typetell(pname);	 	 			//得到文件后缀	 			  	 						 
		if(res==0x4c)//如果是flac,加载patch
		{	
			VS_Load_PatchX();
		}  				 		   		 						  
		res=f_open(fmp3,(const TCHAR*)pname,FA_READ);//打开文件	 
 		if(res==0)	//打开成功.
		{ 
			VS_SPI_SpeedHigh();	//高速						   
			while(rval==0)	//播放完一首歌曲或切换歌曲是跳出循环
			{
				res=f_read(fmp3,databuf,4096,(UINT*)&br);//读出4096个字节  
				i=0;
				do	//主播放循环，发送4096字节后跳出循环
			  {  
					if(player_ctr.play_status==PLAY_START)
					{
						if(VS_Send_MusicData(databuf+i)==0)//给VS10XX发送音频数据
						{
							i+=32;
						}
					}
					
					key = mp3_key();	//扫描按键
					if(key)	//有按键按下
					{
						if((key==PLAY_UP)||(key==PLAY_DOWN))
						{
							rval = key;	//进行歌曲切换，跳出循环
							player_ctr.play_status = PLAY_START;	//切完歌默认开始播放
							i = 4096;
						}
						
						if((key==PLAY)&&(player_ctr.play_mode==PLAY_SELECT))	rval = key;		//选切歌
						
						mp3_Control(key);	//进行播放设置
					}							
					mp3_msg_show(fmp3->fsize);//显示信息	
						    	    
				}while(i<4096);//循环发送4096个字节 
				if(br!=4096||res!=0)
				{
					rval=0;
					break;//读完了.		  
				} 							 
			}
			f_close(fmp3);
		}else rval=0xFF;//出现错误	   	  
	}						     	 
	myfree(SRAMIN,databuf);	  	 		  	    
	myfree(SRAMIN,fmp3);
	return rval;	  	 		  	    
}

/*--------------------------------按键处理------------------------------*/
//扫描板载按键及遥控器按键
//按键不支持连按
//返回键码，0：表示无按键按下
u8 mp3_key(void)
{
	static u8 key_up = 1;
	u8 key;
	
	key = KEY_Scan(0);	//扫描按键
	if(!key)	key = Remote_Scan();	//若无板载按键按下则检测遥控按键
	else return key;	//板载按键直接返回按键值
	
	if(key&&key_up)
	{
		switch(key)
		{			   
			case 162:	key = POWER;	break;	    
			case 98: key = MODE_UP;	break;	    
			case 2: key = PLAY;	break;			 
			case 226: key = MODE;	break;		  
			case 194: key = PLAY_DOWN;	break;		   
			case 34: key = PLAY_UP;	break;	  
			case 224: key = VOL_DOWN;	break;		  
			case 168: key = MODE_DOWN;	break;	
			case 82: key = PLAY_MODE;	break;
			case 144: key = VOL_UP;	break;		    
			case 104: key = NUM_1;	break;		  
			case 152: key = NUM_2;	break;	   
			case 176: key = NUM_3;	break;	    
			case 48: key = NUM_4;	break;		    
			case 24: key = NUM_5;	break;		    
			case 122: key = NUM_6;	break;	  
			case 16: key = NUM_7;	break;			   					
			case 56: key = NUM_8;	break;	 
			case 90: key = NUM_9;	break;
			case 66: key = NUM_0;	break;
			default : key = KEY_UP;	//无按键按下
		}	
		key_up = 0; //按键按下
		return key;
	}
	else if(!key)	key_up = 1;	//按键松开
	return 0;
}


//播放控制
//可通过指定按键直接实现曲目上下切换、音量增减
//通过模式按键切换效果设置，再通过上下按键设置对应效果（mode=0,1,2分别为高音效、低音效及空间效果）
//电源键可以“开/关”显示器
//播放键可以“暂停/开始”播放
//可以进行选歌、顺序及随机播放，其中选歌后需要按播放键播放
u8 mp3_Control(u8 key)
{
	static u8 mode = 0, play_mode;
	static u8 key_last = NUM_0;	//默认按键NUM_0
	
	switch(key)
	{
		case VOL_UP:	//音量增加
			if(vsset.mvol<200)
			{
				vsset.mvol += 5;
				VS_Set_Vol(vsset.mvol);	
			}else vsset.mvol = 200;
			mp3_vol_show((vsset.mvol-100)/5);	//音量限制在:100~200,显示的时候,按照公式(vol-100)/5,显示,也就是0~20   
			break;
		case VOL_DOWN:	//音量减
			if(vsset.mvol>100)
			{
				vsset.mvol -= 5;
				VS_Set_Vol(vsset.mvol);	
			}else vsset.mvol = 100;
			mp3_vol_show((vsset.mvol-100)/5);	//音量限制在:100~200,显示的时候,按照公式(vol-100)/5,显示,也就是0~20   
			break;
		case MODE:	//模式切换
			mode++;
			if(mode>=3)	mode = 0;
			mp3_mode_show(mode);	//显示模式信息
		  break;
		case MODE_UP:	//模式加
			if(mode==0)	
			{
				if(vsset.treble<15)	vsset.treble++;
				VS_Set_Bass(vsset.bflimit,vsset.bass,vsset.tflimit,vsset.treble);  //设置高音效
			}
			else if(mode==1)
			{
				if(vsset.bass<15)	vsset.bass++;
				VS_Set_Bass(vsset.bflimit,vsset.bass,vsset.tflimit,vsset.treble);  //设置低音效
			}
			else if(mode==2)
			{
				if(vsset.effect<3)	vsset.effect++;
				VS_Set_Effect(vsset.effect);	//设置空间效果
			}
			mp3_mode_show(mode);	//显示模式信息
			break;
		case MODE_DOWN:	//模式加
			if(mode==0)	
			{
				if(vsset.treble>0)	vsset.treble--;
				VS_Set_Bass(vsset.bflimit,vsset.bass,vsset.tflimit,vsset.treble);  //设置高低音效
			}
			else if(mode==1)
			{
				if(vsset.bass>0)	vsset.bass--;
				VS_Set_Bass(vsset.bflimit,vsset.bass,vsset.tflimit,vsset.treble);  //设置高低音效
			}
			else if(mode==2)
			{
				if(vsset.effect>0)	vsset.effect--;
				VS_Set_Effect(vsset.effect);	//设置空间效果
			}
			mp3_mode_show(mode);	//显示模式信息
			break;
		case POWER:	//显示器开关
			if(player_ctr.LCD_status==LCD_ON) player_ctr.LCD_status = LCD_OFF;	//显示器关闭
			else player_ctr.LCD_status = LCD_ON;	//显示器打开
			break;
		case PLAY_MODE:	//播放模式
			if(player_ctr.play_mode==PLAY_ORDER)	player_ctr.play_mode = PLAY_RANDOM;	//切换到随机模式
			else if(player_ctr.play_mode==PLAY_RANDOM) player_ctr.play_mode = PLAY_ORDER;	//切换到顺序模式
			mp3_mode_show(mode);	//显示模式信息
			break;
		case PLAY:	//播放模式
			if(player_ctr.play_status==PLAY_START)	player_ctr.play_status = PLAY_STOP;	//切换到随机模式
			else player_ctr.play_status = PLAY_START;	//切换到顺序模式
			if(player_ctr.play_mode==PLAY_SELECT)	//选切歌开始
			{
				player_ctr.play_mode = play_mode;	//选切歌完成，切换到之前的模式
				player_ctr.play_status = PLAY_START;	//开始播放
			}
			mp3_mode_show(mode);	//显示模式信息
			break;
		default:		//数字按键
			if((key>=NUM_0)&&(key<=NUM_9))
			{
				if(player_ctr.play_mode!=PLAY_SELECT)	play_mode = player_ctr.play_mode;	//记录选切歌之前的播放模式
				player_ctr.play_mode = PLAY_SELECT;	//选切歌模式
				
				mode = 3;	//切换到模式3
				vsset.content = (key_last-NUM_0)*10+key-NUM_0;
				key_last = key;	//记录上一次数字按键值
				mp3_mode_show(mode);	//显示模式信息
			}
	}
	
	if(player_ctr.LCD_status==LCD_ON)	player_ctr.LCD_delay = 30;	//显示器打开状态下30s内有操作则重新计时
	else player_ctr.LCD_delay = 0;	//直接关闭显示器（在显示歌曲播放信息函数里进行关闭操作）
	
	return mode;
}


//模式设置显示
//高低音效、空间效果设置
//播放模式设置
//选歌设置
//播放状态显示
void mp3_mode_show(u8 mode)
{
	if(mode==0)	
	{
		Show_Str(30,250,200,16,"高音效果：  dB",16,0);
		LCD_ShowxNum(30+72,250,vsset.treble,2,16,0X80);	//增强效果	
	}
	else if(mode==1)	
	{
		Show_Str(30,250,200,16,"低音效果：  dB",16,0);	
		LCD_ShowxNum(30+72,250,vsset.bass,2,16,0X80);	//增强效果
	}		
	else if(mode==2)	
	{
		Show_Str(30,250,200,16,"空间效果：  dB",16,0);
		LCD_ShowxNum(30+72,250,vsset.effect,2,16,0X80);	//增强效果
	}
	else
	{
		Show_Str(30,250,200,16,"选歌：        ",16,0);
		LCD_ShowxNum(30+40,250,vsset.content,2,16,0X80);	//歌曲索引
	}
	
	if(player_ctr.play_mode==PLAY_RANDOM)
		Show_Str(30+120,250,200,16,"播放模式：随机",16,0);
	else
		Show_Str(30+120,250,200,16,"播放模式：顺序",16,0);
	
	if(player_ctr.play_status==PLAY_START)
		Show_Str(30,270,200,16,"正在播放",16,0);
	else
		Show_Str(30,270,200,16,"暂停播放",16,0);
}





















