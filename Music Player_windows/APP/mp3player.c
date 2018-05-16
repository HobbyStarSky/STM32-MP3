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
#include "decoded.h"
#include "string.h"


_player_ctr_obj player_ctr = {
	30,				//��ʾ���ر���ʱ30s
	LCD_ON,	//��ʾ��Ĭ�ϴ�
	PLAY_ORDER,	//Ĭ��˳�򲥷�ģʽ
	PLAY_CLOSE,			//Ĭ�Ϲر�
	0,				//Ĭ�ϲ���Ŀ¼0
};


//��ʾ��Ŀ����
//index:��ǰ����
//total:���ļ���
void mp3_index_show(u16 index,u16 total)
{
	//��ʾ��ǰ��Ŀ������,������Ŀ��
	Picture_Fill(132,184,24,16,"0:/");	//ˢ��ͼƬ����
	LCD_ShowxNum(132,184,index,3,16,0X81);		//����
	LCD_ShowChar(132+24,184,'/',16,1);
	LCD_ShowxNum(132+32,184,total,3,16,0X81); 	//����Ŀ				  	  
}
//��ʾ��ǰ����
void mp3_vol_show(u8 vol)
{			    
	Picture_Fill(136+32,240,16,16,"0:/");	//ˢ��ͼƬ����
	LCD_ShowString(136,240,200,16,16,"VOL:",1);	  	  
	LCD_ShowxNum(136+32,240,vol,2,16,0X81); 	//��ʾ����	 
}
u16 f_kbps=0;//�����ļ�λ��	
//��ʾ����ʱ��,������ ��Ϣ 
//lenth:�����ܳ���
void mp3_msg_show(u32 lenth)
{	
	static u16 playtime=0;//����ʱ����	     
 	u16 time=0, now_time;// ʱ�����
	u16 temp=0;	  
	if(f_kbps==0xffff)//δ���¹�
	{
		playtime=0;
		f_kbps=VS_Get_HeadInfo();	   //��ñ�����
	}	 	 
	time=VS_Get_DecodeTime(); //�õ�����ʱ��
	if(playtime==0)playtime=time;
	else if((time!=playtime)&&(time!=0))//1sʱ�䵽,������ʾ����
	{
		if(player_ctr.LCD_delay>0)	
		{
			player_ctr.LCD_delay--;	//��ʱ����
			LCD_LED = 1;	//LCD�����
		}
		else 
		{
			LCD_LED = 0;				//LCD����ر�
			player_ctr.LCD_status = LCD_OFF;		//�޲�����ʾ�Զ��ر�	
		}
		playtime=time;//����ʱ�� 	 				    
		temp=VS_Get_HeadInfo(); //��ñ�����	   				 
		if(temp!=f_kbps)
		{
			f_kbps=temp;//����KBPS	  				     
		}			 
		
		//��ʾ����ʱ��		
		Picture_Fill(200,400,40,16,"0:/");	//ˢ��ͼƬ����
		LCD_ShowxNum(200,400,time/60,2,16,0X81);		//����
		LCD_ShowChar(200+16,400,':',16,1);
		LCD_ShowxNum(200+24,400,time%60,2,16,0X81);	//����		
 		LCD_ShowChar(200+40,400,'/',16,1); 	
		now_time = time;
		//��ʾ��ʱ��
		if(f_kbps)time=(lenth/f_kbps)/125;//�õ�������   (�ļ�����(�ֽ�)/(1000/8)/������=����������    	  
		else time=0;//�Ƿ�λ��	 
		Picture_Fill(200+48,400,40,16,"0:/");	//ˢ��ͼƬ����
 		LCD_ShowxNum(200+48,400,time/60,2,16,0X81);	//����
		LCD_ShowChar(200+64,400,':',16,1);
		LCD_ShowxNum(200+72,400,time%60,2,16,0X81);	//����	  		    
	  	  
		mp3_progress_show(now_time,time);	//��ʾ������
		
		LED0=!LED0;		//DS0��ת
	}   		 
}			  		 
//�õ�path·����,Ŀ���ļ����ܸ���
//path:·��		    
//����ֵ:����Ч�ļ���
u16 mp3_get_tnum(u8 *path)
{	  
	u8 res;
	u16 rval=0;
 	DIR tdir;	 		//��ʱĿ¼
	FILINFO tfileinfo;	//��ʱ�ļ���Ϣ		
	u8 *fn; 			 			   			     
    res=f_opendir(&tdir,(const TCHAR*)path); //��Ŀ¼
  	tfileinfo.lfsize=_MAX_LFN*2+1;						//���ļ�����󳤶�
	tfileinfo.lfname=mymalloc(SRAMIN,tfileinfo.lfsize);	//Ϊ���ļ������������ڴ�
	if(res==FR_OK&&tfileinfo.lfname!=NULL)
	{
		while(1)//��ѯ�ܵ���Ч�ļ���
		{
	        res=f_readdir(&tdir,&tfileinfo);       		//��ȡĿ¼�µ�һ���ļ�
	        if(res!=FR_OK||tfileinfo.fname[0]==0)break;	//������/��ĩβ��,�˳�		  
     		fn=(u8*)(*tfileinfo.lfname?tfileinfo.lfname:tfileinfo.fname);			 
			res=f_typetell(fn);	//�Ա��ļ�����
			if((res&0XF0)==0X40)//ȡ����λ,�����ǲ��������ļ�	
			{
				rval++;//��Ч�ļ�������1
			}	    
		}  
	} 
	myfree(SRAMIN,tfileinfo.lfname);
	return rval;
}
//��������
void mp3_play(void)
{
	u8 res;
 	DIR mp3dir;	 		//Ŀ¼
	FILINFO mp3fileinfo;//�ļ���Ϣ
	u8 *fn;   			//���ļ���
	u8 *pname;			//��·�����ļ���
	u16 totmp3num; 		//�����ļ�����
	u16 curindex;		//ͼƬ��ǰ����
	u8 key;				//��ֵ		  
 	u16 temp;
	u16 *mp3indextbl;	//���������� 
	
 	while(f_opendir(&mp3dir,"0:/MUSIC"))//��ͼƬ�ļ���
 	{	    
		Show_Str(30,190,240,16,"MUSIC�ļ��д���!",16,1);
		delay_ms(200);				  
		LCD_Fill(30,190,240,206,WHITE);//�����ʾ	     
		delay_ms(200);				  
	} 									  
	totmp3num=mp3_get_tnum("0:/MUSIC"); //�õ�����Ч�ļ���
  while(totmp3num==NULL)//�����ļ�����Ϊ0		
 	{	    
		Show_Str(30,190,240,16,"û�������ļ�!",16,1);
		delay_ms(200);				  
		LCD_Fill(30,190,240,146,WHITE);//�����ʾ	     
		delay_ms(200);				  
	}						

	LCD_ShowString(88,100,200,16,24,"Music Player",1);	//��ʾ����������
	
  mp3fileinfo.lfsize=_MAX_LFN*2+1;						//���ļ�����󳤶�
	mp3fileinfo.lfname=mymalloc(SRAMIN,mp3fileinfo.lfsize);	//Ϊ���ļ������������ڴ�
 	pname=mymalloc(SRAMIN,mp3fileinfo.lfsize);				//Ϊ��·�����ļ��������ڴ�
 	mp3indextbl=mymalloc(SRAMIN,2*totmp3num);				//����2*totmp3num���ֽڵ��ڴ�,���ڴ�������ļ�����
 	while(mp3fileinfo.lfname==NULL||pname==NULL||mp3indextbl==NULL)//�ڴ�������
 	{	    
		Show_Str(30,190,240,16,"�ڴ����ʧ��!",16,1);
		delay_ms(200);				  
		LCD_Fill(30,190,240,146,WHITE);//�����ʾ	     
		delay_ms(200);				  
	}  	
	VS_HD_Reset();
	VS_Soft_Reset();
	mp3_vol_show((vsset.mvol-100)/5);	//����������:100~250,��ʾ��ʱ��,���չ�ʽ(vol-100)/5,��ʾ,Ҳ����0~30   
	//��¼����
  res=f_opendir(&mp3dir,"0:/MUSIC"); //��Ŀ¼
	if(res==FR_OK)
	{
		curindex=0;//��ǰ����Ϊ0
		while(1)//ȫ����ѯһ��
		{
			temp=mp3dir.index;								//��¼��ǰindex
			res=f_readdir(&mp3dir,&mp3fileinfo);       		//��ȡĿ¼�µ�һ���ļ�
			if(res!=FR_OK||mp3fileinfo.fname[0]==0)	break;	//������/��ĩβ��,�˳�		  
			fn=(u8*)(*mp3fileinfo.lfname?mp3fileinfo.lfname:mp3fileinfo.fname);			 
			res=f_typetell(fn);	
			if((res&0XF0)==0X40)//ȡ����λ,�����ǲ��������ļ�	
			{
				mp3indextbl[curindex]=temp;//��¼����
				curindex++;
			}	    
		} 
	}   
	curindex=0;											//��0��ʼ��ʾ
	res=f_opendir(&mp3dir,(const TCHAR*)"0:/MUSIC"); 	//��Ŀ¼
	while(res==FR_OK)//�򿪳ɹ���һֱ�ڴ˺�����ѭ����
	{	
		dir_sdi(&mp3dir,mp3indextbl[curindex]);			//�ı䵱ǰĿ¼����	   
    res=f_readdir(&mp3dir,&mp3fileinfo);       		//��ȡĿ¼�µ�һ���ļ�
    if(res!=FR_OK||mp3fileinfo.fname[0]==0)break;	//������/��ĩβ��,�˳�
   	fn=(u8*)(*mp3fileinfo.lfname?mp3fileinfo.lfname:mp3fileinfo.fname);			 
		strcpy((char*)pname,"0:/MUSIC/");				//����·��(Ŀ¼)
		strcat((char*)pname,(const char*)fn);  			//���ļ������ں���
		
 		Picture_Fill(0,204,320,16,"0:/");	//ˢ��ͼƬ�������޷�ȷ����һ�׸��������ʰ��������ˢ�£�
		Show_Str(160-4*strlen(fn),204,320,16,fn,16,1);				//��ʾ��������
		
		mp3_index_show(curindex+1,totmp3num);
		key = mp3_play_song(pname); 				 		//�������MP3 
			
		if(key==0xFF)	break;	//�����˴���
		else 
		{
			if(player_ctr.play_mode==PLAY_RANDOM)	//�������ģʽ
			{
				if((key==PLAY_UP)||(key==PLAY_DOWN)||(key==0))	
					curindex = rand()%totmp3num;		//�������
			}
			else 
			{
				if(key==PLAY_UP)	//��һ��
				{ 	
					if(curindex)	curindex--;
					else curindex = totmp3num-1;
				}
				else if((key==PLAY_DOWN)||(key==0))	//��һ��
				{
					curindex++;		   	
					if(curindex>=totmp3num)	curindex = 0;//��ĩβ��ʱ��,�Զ���ͷ��ʼ
				}
			}
			
			if(key==PLAY)	//ѡ��
			{
				if((vsset.content<=totmp3num)&&(vsset.content>0))	//������Χ[1,������]
					curindex = vsset.content-1;	//ѡ��
				else 
				{
					vsset.content = curindex+1;	//��Ч�и�
					mp3_mode_show(0,5);	//��ʾʵ���и���Ŀ
				}
			} 
		}
		
	} 											  
	myfree(SRAMIN,mp3fileinfo.lfname);	//�ͷ��ڴ�			    
	myfree(SRAMIN,pname);				//�ͷ��ڴ�			    
	myfree(SRAMIN,mp3indextbl);			//�ͷ��ڴ�	 
}

//����һ��ָ���ĸ���				     	   									    	 
//����ֵ:0,�����������
//		 1,��һ��
//       2,��һ��
//       0XFF,���ִ�����
u8 mp3_play_song(u8 *pname)
{	 
 	FIL* fmp3;
  u16 br;
	u8 key;
	u8 res,rval;	  
	u8 *databuf;	   		   
	u16 i=0; 
	static u8 j = 0;
			   
	rval=0;	   
	fmp3=(FIL*)mymalloc(SRAMIN,sizeof(FIL));//�����ڴ�
	databuf=(u8*)mymalloc(SRAMIN,4096);		//����4096�ֽڵ��ڴ�����
	if(databuf==NULL||fmp3==NULL)rval=0XFF ;//�ڴ�����ʧ��.
	if(rval==0)
	{	  
	  VS_Restart_Play();  					//�������� 
		VS_Set_All();        					//������������Ϣ 			 
		VS_Reset_DecodeTime();					//��λ����ʱ�� 	  
		res=f_typetell(pname);	 	 			//�õ��ļ���׺	 			  	 						 
		if(res==0x4c)//�����flac,����patch
		{	
			VS_Load_PatchX();
		}  				 		   		 						  
		res=f_open(fmp3,(const TCHAR*)pname,FA_READ);//���ļ�	 
 		if(res==0)	//�򿪳ɹ�.
		{ 
			VS_SPI_SpeedHigh();	//����						   
			while(rval==0)	//������һ�׸������л�����������ѭ��
			{
				res=f_read(fmp3,databuf,4096,(UINT*)&br);//����4096���ֽ�  
				i=0;
				do	//������ѭ��������4096�ֽں�����ѭ��
			  {  
					if(player_ctr.play_status==PLAY_START)
					{
						if(VS_Send_MusicData(databuf+i)==0)//��VS10XX������Ƶ����
						{
							i+=32;
						}
					}
					else if(player_ctr.play_status==PLAY_STOP)	//��ͣ״̬�����м�ʱ
					{
						if(player_ctr.LCD_delay>0)	
						{
							j++;
							if(j==100)	//��ʱ1s
							{
								j = 0;
								player_ctr.LCD_delay--;	//��ʱ����
							}
							if(LCD_LED==0) LCD_LED = 1;	//LCD�����
							delay_ms(10);
						}
						else 
						{
							if(LCD_LED==1) LCD_LED = 0;				//LCD����ر�
							if(j>0)	j = 0;	//��ʼ����ʱ
							player_ctr.LCD_status = LCD_OFF;		//�޲�����ʾ�Զ��ر�	
						}
					}
					
					key = mp3_key();	//ɨ�谴��
					if(key)	//�а�������
					{
						if(((key==PLAY_UP)||(key==PLAY_DOWN))&&(player_ctr.play_status!=PLAY_CLOSE))
						{
							rval = key;	//���и����л�������ѭ��
							player_ctr.play_status = PLAY_START;	//�����Ĭ�Ͽ�ʼ����
							i = 4096;
						}
						
						if((key==PLAY)&&(player_ctr.play_mode==PLAY_SELECT))	rval = key;		//ѡ�и�
						
						mp3_Control(key);	//���в�������
					}							
					mp3_msg_show(fmp3->fsize);//��ʾ��Ϣ	
						    	    
				}while(i<4096);//ѭ������4096���ֽ� 
				if(br!=4096||res!=0)
				{
					rval=0;
					break;//������.		  
				} 							 
			}
			f_close(fmp3);
		}else rval=0xFF;//���ִ���	   	  
	}						     	 
	myfree(SRAMIN,databuf);	  	 		  	    
	myfree(SRAMIN,fmp3);
	return rval;	  	 		  	    
}

/*--------------------------------��������------------------------------*/
//ɨ����ذ�����ң��������
//������֧������
//���ؼ��룬0����ʾ�ް�������
u8 mp3_key(void)
{
	static u8 key_up = 1;
	u8 key;
	
	key = KEY_Scan(0);	//ɨ�谴��(���ذ������Ӧ���ⰴ��������ͬ�����ڴ���������)
	if(!key)	key = Remote_Scan();	//���ް��ذ�����������ң�ذ���
	else return key;	//���ذ���ֱ�ӷ��ذ���ֵ
	
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
			default : key = KEY_UP;	//�ް�������
		}	
		key_up = 0; //��������
		return key;
	}
	else if(!key)	key_up = 1;	//�����ɿ�
	return KEY_UP;
}


//���ſ���
//��ͨ��ָ������ֱ��ʵ����Ŀ�����л�����������
//ͨ��ģʽ�����л�Ч�����ã���ͨ�����°������ö�ӦЧ����mode=0,1,2�ֱ�Ϊ����Ч������Ч���ռ�Ч����
//��Դ�����ԡ���/�ء���ʾ��
//���ż����ԡ���ͣ/��ʼ������
//���Խ���ѡ�衢˳��������ţ�����ѡ�����Ҫ�����ż�����
u8 mp3_Control(u8 key)
{
	static u8 mode = 0, play_mode;
	static u8 key_last[2] = {NUM_0,NUM_0};	//Ĭ�ϰ���NUM_0
	
	switch(key)
	{
		case VOL_UP:	//��������
			if(vsset.mvol<200)
			{
				vsset.mvol += 5;
				VS_Set_Vol(vsset.mvol);	
			}else vsset.mvol = 200;
			mp3_vol_show((vsset.mvol-100)/5);	//����������:100~200,��ʾ��ʱ��,���չ�ʽ(vol-100)/5,��ʾ,Ҳ����0~20   
			break;
		case VOL_DOWN:	//������
			if(vsset.mvol>100)
			{
				vsset.mvol -= 5;
				VS_Set_Vol(vsset.mvol);	
			}else vsset.mvol = 100;
			mp3_vol_show((vsset.mvol-100)/5);	//����������:100~200,��ʾ��ʱ��,���չ�ʽ(vol-100)/5,��ʾ,Ҳ����0~20   
			break;
		case MODE:	//ģʽ�л�
			mode++;
			if(mode>4)	mode = 0;
			mp3_mode_show(0,mode);	//��ʾģʽ��Ϣ
		  break;
		case MODE_UP:	//ģʽ��
			if(mode==0)	
			{
				if(vsset.treble<15)	vsset.treble++;
				VS_Set_Bass(vsset.bflimit,vsset.bass,vsset.tflimit,vsset.treble);  //���ø���Ч
			}
			else if(mode==1)
			{
				if(vsset.bass<15)	vsset.bass++;
				VS_Set_Bass(vsset.bflimit,vsset.bass,vsset.tflimit,vsset.treble);  //���õ���Ч
			}
			else if(mode==2)
			{
				if(vsset.effect<3)	vsset.effect++;
				VS_Set_Effect(vsset.effect);	//���ÿռ�Ч��
			}
			else if(mode==4)
			{
				vsset.speakersw = 1;	//������
				VS_SPK_Set(vsset.speakersw);	//���ư�������״̬����
			}
			mp3_mode_show(0,mode);	//��ʾģʽ��Ϣ
			break;
		case MODE_DOWN:	//ģʽ��
			if(mode==0)	
			{
				if(vsset.treble>0)	vsset.treble--;
				VS_Set_Bass(vsset.bflimit,vsset.bass,vsset.tflimit,vsset.treble);  //���øߵ���Ч
			}
			else if(mode==1)
			{
				if(vsset.bass>0)	vsset.bass--;
				VS_Set_Bass(vsset.bflimit,vsset.bass,vsset.tflimit,vsset.treble);  //���øߵ���Ч
			}
			else if(mode==2)
			{
				if(vsset.effect>0)	vsset.effect--;
				VS_Set_Effect(vsset.effect);	//���ÿռ�Ч��
			}
			else if(mode==4)
			{
				vsset.speakersw = 0;	//�ر�����
				VS_SPK_Set(vsset.speakersw);	//���ư������ȹر� 
			}
			mp3_mode_show(0,mode);	//��ʾģʽ��Ϣ
			break;
		case POWER:	//��ʾ������
			if(player_ctr.LCD_status==LCD_ON) player_ctr.LCD_status = LCD_OFF;	//��ʾ���ر�
			else player_ctr.LCD_status = LCD_ON;	//��ʾ����
			break;
		case PLAY_MODE:	//����ģʽ
			if(player_ctr.play_mode==PLAY_ORDER)	player_ctr.play_mode = PLAY_RANDOM;	//�л������ģʽ
			else if(player_ctr.play_mode==PLAY_RANDOM) player_ctr.play_mode = PLAY_ORDER;	//�л���˳��ģʽ
			mp3_mode_show(1,mode);	//��ʾģʽ��Ϣ
			break;
		case PLAY:	//����ģʽ
			if(player_ctr.play_status==PLAY_START)	player_ctr.play_status = PLAY_STOP;	//�л������ģʽ
			else player_ctr.play_status = PLAY_START;	//�л���˳��ģʽ
			if(player_ctr.play_mode==PLAY_SELECT)	//ѡ�и迪ʼ
			{
				player_ctr.play_mode = play_mode;	//ѡ�и���ɣ��л���֮ǰ��ģʽ
				player_ctr.play_status = PLAY_START;	//��ʼ����
				mp3_mode_show(0,5);	//��ʾ�и���Ŀ
			}
			else mp3_mode_show(2,mode);	//��ʾģʽ��Ϣ
			break;
		default:		//���ְ���
			if((key>=NUM_0)&&(key<=NUM_9))
			{
				if(player_ctr.play_mode!=PLAY_SELECT)	play_mode = player_ctr.play_mode;	//��¼ѡ�и�֮ǰ�Ĳ���ģʽ
				player_ctr.play_mode = PLAY_SELECT;	//ѡ�и�ģʽ
				
				mode = 3;	//�л���ģʽ3
				player_ctr.play_content = (key_last[1]-NUM_0)*100+(key_last[0]-NUM_0)*10+key-NUM_0;
				if(player_ctr.play_content>255)	vsset.content = 255;	//����ѡ�赽255
				else vsset.content = player_ctr.play_content;
				key_last[1] = key_last[0];	
				key_last[0] = key;	//��¼��һ�����ְ���ֵ
				mp3_mode_show(0,mode);	//��ʾģʽ��Ϣ
			}
	}
	
	if(player_ctr.LCD_status==LCD_ON)	player_ctr.LCD_delay = 30;	//��ʾ����״̬��30s���в��������¼�ʱ
	else player_ctr.LCD_delay = 0;	//ֱ�ӹر���ʾ��������ʾ����������Ϣ��������йرղ�����
	
	return mode;
}


//ģʽ������ʾ
//�ߵ���Ч���ռ�Ч������
//����ģʽ����
//ѡ������
//����״̬��ʾ
void mp3_mode_show(u8 key, u8 mode)
{
	if(key==0)
	{
		Picture_Fill(108,312,112,16,"0:/");	//ˢ��ͼƬ����
		if(mode==0)	
		{
			Show_Str(108,312,200,16,"����Ч����  dB",16,1);
			LCD_ShowxNum(108+72,312,vsset.treble,2,16,0X81);	//��ǿЧ��	
		}
		else if(mode==1)	
		{
			Show_Str(108,312,200,16,"����Ч����  dB",16,1);	
			LCD_ShowxNum(108+72,312,vsset.bass,2,16,0X81);	//��ǿЧ��
		}		
		else if(mode==2)	
		{
			Show_Str(108,312,200,16,"�ռ�Ч����  ",16,1);
			LCD_ShowxNum(108+72,312,vsset.effect,2,16,0X81);	//��ǿЧ��
		}
		else if(mode==3)
		{
			Show_Str(108,312,200,16,"ѡ�裺        ",16,1);
			LCD_ShowxNum(108+40,312,player_ctr.play_content,3,16,0X81);	//����������ʾ(�����)
		}
		else if(mode==4)
		{
			Show_Str(108,312,200,16,"���ȣ�        ",16,1);
			if(vsset.speakersw)	Show_Str(108+40,312,200,16,"��",16,1);	//���ȴ�
			else Show_Str(108+40,312,200,16,"�ر�",16,1);	//���ȹر�
		}
		else if(mode==5)
		{
			Show_Str(108,312,200,16,"ѡ�裺        ",16,1);
			LCD_ShowxNum(108+40,312,vsset.content,3,16,0X81);	//����������ʾ��ʵ�ʵģ�
		}
	}
	else if(key==1)
	{
		Picture_Fill(116,276,88,16,"0:/");	//ˢ��ͼƬ����
		if(player_ctr.play_mode==PLAY_RANDOM)
			Show_Str(116,276,200,16,"MODE��RANDOM",16,1);
		else
			Show_Str(116,276,200,16,"MODE��OEDER",16,1);
	}
	else if(key==2)
	{
		Picture_Fill(128,148,64,16,"0:/");	//ˢ��ͼƬ����
		if(player_ctr.play_status==PLAY_START)
			Show_Str(128,148,200,16,"���ڲ���",16,1);
		else
			Show_Str(128,148,200,16,"��ͣ����",16,1);
	}
}

//��ʾ������
void mp3_progress_show(u16 now_time,u16 all_time)
{
	static u8 flag = 0;	//���ڵ�һ����ʾ
	u32 temp;
	
	temp = 260*now_time/all_time;
	
	if((now_time==1)||(flag==0)) LCD_Fill(30,420,290,423,LIGHTBLUE);	//�������������
	LCD_Fill(30,420,30+temp,423,POINT_COLOR);	//����Ѳ��Ž���
	if(flag==0)
	{
		mp3_mode_show(0,0);	//������ʾ��������Ϣ
		mp3_mode_show(1,0);
		mp3_mode_show(2,0);
	}
	flag = 1;
}	


















