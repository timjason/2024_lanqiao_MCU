#include <STC15F2K60S2.H>
#include "iic.h"
#include "ds1302.h"

code unsigned char Seg_Table[] =
{
	0xc0, //0
	0xf9, //1
	0xa4, //2
	0xb0, //3
	0x99, //4
	0x92, //5
	0x82, //6
	0xf8, //7
	0x80, //8
	0x90, //9
	0x88, //A
	0x83, //b
	0xc6, //C
	0xa1, //d
	0x86, //E
	0x8e, //F
	0xbf, //-
	0x7f, //.
	0xff  //close
};
//

unsigned char count_t=0;//50ms timer1
unsigned char count_s=0;//200ms timer1
unsigned int freq=0;//timer0
int freq_nix=0;//freq nix
unsigned int freq_PF_nix=0;
unsigned char mode_1=1;//big mode
unsigned char mode_2=1;//small mode
unsigned int freq_P=2000;//max_freq
int freq_PF=100;//Ð£×¼ÆµÂÊ
unsigned int Max_freq_nix=0;//max_freq
unsigned char LED_stat=0xff;
unsigned int volt=0;

code unsigned char R_Time_addr[3]={0x81,0x83,0x85};//Read_ds1302_Time
code unsigned char W_Time_addr[3]={0x80,0x82,0x84};//Write_ds1302_Time
unsigned char Time_data[3]={0,0,0};//store_ds1302_time
unsigned char Max_Time_data[3]={0,0,0};//store_max_ds1302_time

void Delay(unsigned int t)		//@12.000MHz
{
	unsigned char i, j;
	while(t)
	{
		i = 12;
		j = 169;
		do
		{
			while (--j);
		} while (--i);
		t--;
	}
}
//

void Set_HC573(unsigned char n,dat)
{
	P0=dat;
	
	switch (n)
	{
		case 4:
			P2=(P2&0x1f)|0x80;
		break;
		
		case 5:
			P2=(P2&0x1f)|0xa0;
		break;
				
		case 6:
			P2=(P2&0x1f)|0xc0;
		break;
		
		case 7:
			P2=(P2&0x1f)|0xe0;
		break;
	}
	
	P2=(P2&0x1f)|0x00;
}
//

void Nix_Bit(unsigned char pos,dat)
{
	Set_HC573(6,0x01<<pos);
	Set_HC573(7,dat);
	Delay(1);
	Set_HC573(6,0x01<<pos);
	Set_HC573(7,0xff);
}
//

void Close_Nix_All()
{
	unsigned char i=0;
	for(i=0;i<7;i++)
		Nix_Bit(i,0xff);
}
//

void Display_Nix()
{
//	Nix_Bit(0,Seg_Table[]);
//	Nix_Bit(1,Seg_Table[]);
//	Nix_Bit(2,Seg_Table[]);
//	Nix_Bit(3,Seg_Table[]);
//	Nix_Bit(4,Seg_Table[]);
//	Nix_Bit(5,Seg_Table[]);
//	Nix_Bit(6,Seg_Table[]);
//	Nix_Bit(7,Seg_Table[]);
	switch (mode_1)
	{
		case 1:
			if(freq_nix<freq_PF)
			{
				Nix_Bit(0,Seg_Table[15]);
				Nix_Bit(6,0xc7);
				Nix_Bit(7,0xc7);
			}
			else
			{
				Nix_Bit(0,Seg_Table[15]);
				Nix_Bit(1,Seg_Table[18]);
				Nix_Bit(2,Seg_Table[18]);
				if(freq_nix>9999)
					Nix_Bit(3,Seg_Table[freq_nix/10000]);
				if(freq_nix>999)
					Nix_Bit(4,Seg_Table[(freq_nix/1000)%10]);
				if(freq_nix>99)
					Nix_Bit(5,Seg_Table[(freq_nix/100)%10]);
				if(freq_nix>9)
					Nix_Bit(6,Seg_Table[(freq_nix/10)%10]);
				Nix_Bit(7,Seg_Table[freq_nix%10]);
			}
		break;
		//
		case 2:
			switch (mode_2)
			{
				case 1:
					Nix_Bit(0,0x8c);//P1
					Nix_Bit(1,Seg_Table[1]);
					Nix_Bit(2,Seg_Table[18]);
					Nix_Bit(3,Seg_Table[18]);
				
					Nix_Bit(4,Seg_Table[(freq_P/1000)%10]);
					Nix_Bit(5,Seg_Table[(freq_P/100)%10]);
					Nix_Bit(6,Seg_Table[(freq_P/10)%10]);
					Nix_Bit(7,Seg_Table[freq_P%10]);
				break;
				//
				case 2:
					Nix_Bit(0,0x8c);//P
					Nix_Bit(1,Seg_Table[2]);
					Nix_Bit(2,Seg_Table[18]);
					Nix_Bit(3,Seg_Table[18]);

					if(freq_PF>0)
					{
						Nix_Bit(4,Seg_Table[18]);
						Nix_Bit(5,Seg_Table[(freq_PF/100)%10]);
						Nix_Bit(6,Seg_Table[(freq_PF/10)%10]);
						Nix_Bit(7,Seg_Table[freq_PF%10]);
					}
				
					if(freq_PF<0)
					{
						freq_PF_nix=-freq_PF;
						Nix_Bit(4,Seg_Table[16]);
						Nix_Bit(5,Seg_Table[(freq_PF_nix/100)%10]);
						Nix_Bit(6,Seg_Table[(freq_PF_nix/10)%10]);
						Nix_Bit(7,Seg_Table[freq_PF_nix%10]);
					}
					
					if(freq_PF==0)
						Nix_Bit(7,Seg_Table[0]);
				break;
			}
		break;
		//
		case 3:
			Nix_Bit(7,Seg_Table[Time_data[0]%16]);
			Nix_Bit(6,Seg_Table[Time_data[0]/16]);
			Nix_Bit(5,Seg_Table[16]);
			Nix_Bit(4,Seg_Table[Time_data[1]%16]);
			Nix_Bit(3,Seg_Table[Time_data[1]/16]);
			Nix_Bit(2,Seg_Table[16]);
			Nix_Bit(1,Seg_Table[Time_data[2]%16]);
			Nix_Bit(0,Seg_Table[Time_data[2]/16]);
		break;
		//
		case 4:
			switch(mode_2)
			{
				case 1:
					Nix_Bit(7,Seg_Table[Max_freq_nix%10]);
					if(Max_freq_nix>9)
						Nix_Bit(6,Seg_Table[(Max_freq_nix/10)%10]);
					else
						Nix_Bit(6,Seg_Table[18]);
					if(Max_freq_nix>99)
						Nix_Bit(5,Seg_Table[(Max_freq_nix/100)%10]);
					else
						Nix_Bit(5,0xff);
					if(Max_freq_nix>999)
						Nix_Bit(4,Seg_Table[(Max_freq_nix/1000)%10]);
					else
						Nix_Bit(4,0xff);
					if(Max_freq_nix>9999)
						Nix_Bit(3,Seg_Table[Max_freq_nix/10000]);
					else
						Nix_Bit(3,0xff);
					Nix_Bit(2,0xff);
					Nix_Bit(1,Seg_Table[15]);
					Nix_Bit(0,0x89);
				break;
				//
				case 2:
					Nix_Bit(7,Seg_Table[Max_Time_data[0]%16]);
					Nix_Bit(6,Seg_Table[Max_Time_data[0]/16]);
					Nix_Bit(5,Seg_Table[Max_Time_data[1]%16]);
					Nix_Bit(4,Seg_Table[Max_Time_data[1]/16]);
					Nix_Bit(3,Seg_Table[Max_Time_data[2]%16]);
					Nix_Bit(2,Seg_Table[Max_Time_data[2]/16]);
					Nix_Bit(1,Seg_Table[10]);
					Nix_Bit(0,0x89);
				break;
			}
		break;
	}
	Close_Nix_All();
}
//

void Init_Timer()//NE555
{
	
	TMOD &= 0xF0;		
	TMOD |= 0x06;		
	TL0 = 0xFF;		
	TH0 = 0xFF;		
	TF0 = 0;		
	TR0 = 1;	

	
	TMOD &= 0x1F;		
	TL1 = (65535-50000)/256;		
	TH1 = (65535-50000)%256;		
	TF1 = 0;		
	TR1 = 1;			
	
	EA=1;
	ET0=1;
	ET1=1;
}
//

void Ser_T0() interrupt 1
{
	freq++;
}
//

void Ser_T1() interrupt 3
{
	TL1 = (65535-50000)/256;		
	TH1 = (65535-50000)%256;	
	count_t++;
	if(count_t%4==0)
	{
		count_s++;
		if(mode_1==1&&count_s%2==0)
		{
			Display_Nix();
			if(LED_stat&0x01==1)
			{
				LED_stat&=0xfe;
				Display_Nix();
			}
			else
			{
				LED_stat|=0x01;
				Display_Nix();
			}
		}
		if(mode_1!=1)
		{
			LED_stat|=0x01;
			Display_Nix();
		}
		
		if(freq_nix<0)
		{
			LED_stat&=0xfd;
			Display_Nix();
		}
		else if(freq_nix>freq_P&&count_s%2==0)
		{
			if((LED_stat&0x02)>>1==1)
			{
				Display_Nix();
				LED_stat&=0xfd;
			}
			else
			{
				LED_stat|=0x02;
				Display_Nix();
			}
		}
		else
		{
			LED_stat|=0x02;
			Display_Nix();
		}
	}

	Set_HC573(4,LED_stat);

	if(count_t==20)
	{
		count_t=0;
		freq_nix=freq-freq_PF;
		
		if(Max_freq_nix<freq_nix)
		{
			Max_freq_nix=freq_nix;

			Max_Time_data[0]=Time_data[0];
			Max_Time_data[1]=Time_data[1];
			Max_Time_data[2]=Time_data[2];
		
		}
		freq=0;

	}
}
//

void Set_DS1302_Time()
{
	unsigned char i=0;
	Write_Ds1302_Byte(0x8e,0x00);
	
	for(i=0;i<3;i++)
	{
		Write_Ds1302_Byte(W_Time_addr[i],Time_data[i]);
	}
	
	Write_Ds1302_Byte(0x8e,0x80);
}
//

void Read_DS1302_Time()
{
	unsigned char i=0;
	for(i=0;i<3;i++)
	{
		Time_data[i]=Read_Ds1302_Byte(R_Time_addr[i]);
	}
	Display_Nix();
}
//

void Init_sys()
{
	Set_HC573(4,0xff);
	Set_HC573(5,0x00);
	Close_Nix_All();
	Init_Timer();
	Set_DS1302_Time();
}
//

void Scan_Key()
{
	P44=0;
	P42=P34=P35=1;
	P30=P31=P32=P33=1;
	if(P32==0)
	{
		Delay(20);
		while(P32==0);
		Display_Nix();
		//S5
		if(mode_2==1)
			mode_2=2;
		else if(mode_2==2)
			mode_2=1;
		
	}
	if(P33==0)
	{
		Delay(20);
		while(P33==0);
		Display_Nix();
		//S4
		if(mode_1<4)
		{
			mode_1++;
			mode_2=1;
		}
		else if(mode_1==4)
		{
			mode_1=1;
			mode_2=1;
		}
	}
	
	P42=0;
	P44=P34=P35=1;
	P30=P31=P32=P33=1;
	if(P32==0)
	{
		Delay(20);
		while(P32==0);
		Display_Nix();
		//S9
		if(mode_1==2&&mode_2==1)
		{
			if(freq_P==1000)
				freq_P=1000;
			else
				freq_P-=1000;
		}
		if(mode_1==2&&mode_2==2)
		{
			if(freq_PF==-900)
				freq_PF=-900;
			else
				freq_PF-=100;
		}
	}
	if(P33==0)
	{
		Delay(20);
		while(P33==0);
		Display_Nix();
		//S8
		if(mode_1==2&&mode_2==1)
		{
			if(freq_P==9000)
				freq_P=9000;
			else
				freq_P+=1000;
		}
		if(mode_1==2&&mode_2==2)
		{
			if(freq_PF==900)
				freq_PF=900;
			else
				freq_PF+=100;
		}
	}
}
//

void Set_PCF8591_DAC()
{
	I2CStart();
	I2CSendByte(0x90);
	I2CWaitAck();
	I2CSendByte(0x43);
	I2CWaitAck();
	if(freq_nix<0)
	{
		I2CSendByte(0);
		I2CWaitAck();
	}
	else if(freq_nix<500)
	{
		I2CSendByte(51);
		I2CWaitAck();
	}
	else if(freq_nix>500)
	{
		I2CSendByte(204/(freq_P-500)+51);
		I2CWaitAck();
	}
	I2CStop();
	Display_Nix();
}
//

void main()
{
	Init_sys();
	while(1)
	{
		Scan_Key();
		Display_Nix();
		Read_DS1302_Time();
		Set_PCF8591_DAC();
	}
}