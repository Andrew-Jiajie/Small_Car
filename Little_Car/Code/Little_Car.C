/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* Copyright(c) 2017 Nuvoton Technology Corp. All rights reserved.                                         */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/

//***********************************************************************************************************
//  Website: http://www.nuvoton.com
//  E-Mail : MicroC-8bit@nuvoton.com
//  Date   : Jan/21/2017
//***********************************************************************************************************

//***********************************************************************************************************
//  File Function: N76E003 GPIO demo code
//***********************************************************************************************************
#include "N76E003.h"
#include "SFR_Macro.h"
#include "Function_define.h"
#include "Common.h"
#include "Delay.h"
#include "stdio.h"
#include "string.h"
#include "stdlib.h"


#define HIGH 1
#define LOW 0

#define WAITING 2
#define PLAYING 1
#define STOP 0
#define ON 1
#define OFF 0
#define MINIT 	60
#define SECOND 1
#define SW_PRESS 1
#define SW_RELEASE 2
#define SW_NONE 0


/*------------------------------------------------
The main C function.  Program execution starts
here after stack initialization.
------------------------------------------------*/

#define setbit(x,y) x|=(1<<y) //将X的第Y位置1
#define testbit(x,y) x&(1<<y) //测试X的第Y位置
#define clrbit(x,y) x&=!(1<<y) //将X的第Y位清0

#if 1
uchar button_trig_state=LOW;
uchar Button_state=HIGH;
void PinInterrupt_ISR (void) interrupt 7
{
	if (testbit(PIF,4))	//SWITCH PIN
	{
		clrbit(PIF,4);
		//Delay_100us(1);
		if(button_trig_state==LOW && P14==LOW){
			Enable_BIT4_RasingEdge_Trig;
			button_trig_state=HIGH;
			Button_state=LOW;
			Send_Data_To_UART0(Button_state);
			Send_Data_To_UART0('\n');
			Send_Data_To_UART0(Button_state);
			Send_Data_To_UART0('\n');
		}else if(button_trig_state==HIGH && P14==HIGH){
			Enable_BIT4_FallEdge_Trig;
			button_trig_state=LOW;
			Button_state=HIGH;
			Send_Data_To_UART0(Button_state);
			Send_Data_To_UART0('\n');
			Send_Data_To_UART0(Button_state);
			Send_Data_To_UART0('\n');
		}
	}
}
#endif

int Send_num(int num){
	char CharToSend[10];
	uchar i=0,j=9;
	uchar negative=0;
	if(num<0){
		negative=1;
		num=-num;
	}
	if(num==0)
		CharToSend[j--]='0';
	while(num>0){
		uchar aa=num%10;
		CharToSend[j--]='0'+aa;
		num=num/10;
	}
	if(negative==1){
		CharToSend[j--]='-';
	}
	for(i=j+1; i<10; i++){
		Send_Data_To_UART1(CharToSend[i]);
	}
}
/************************************************************************************************************
*   	Servo Control part
************************************************************************************************************/
//9C3F=40000;
//MIN=1000=0X03E8;
//MIDDLE=3000=0X0BB8
//MAX=5000=0X1388;
#define RATE 2
#define SERVO_LEFT		1000
#define SERVO_RIGHT		5000
#define SERVO_MIDDLE	3000
#define SEVOR_PIN 		P10
#define TIMER_DIV12_VALUE_50us			65536-65		//65*12/16000000 = 50 uS,  		// Timer divider = 12 

int timer_count=0;
int servo_angle=15*RATE;
#if 0
void Servo_Timer0_ISR (void) interrupt 1          //interrupt address is 0x000B
{
	clr_TF0;
	clr_TR0;                              		  //Stop Timer0
    TL0 = LOBYTE(TIMER_DIV12_VALUE_50us); 		//Find  define in "Function_define.h" "TIMER VALUE"
    TH0 = HIBYTE(TIMER_DIV12_VALUE_50us);
	set_TR0;                              		  //Start Timer0
	if(timer_count<=servo_angle){
		SEVOR_PIN=1;
	}else{
		SEVOR_PIN=0;
	}
	timer_count++;
	if(timer_count>=200*RATE){
		timer_count=0;
		Send_Data_To_UART0(Button_state);
		Send_Data_To_UART0('\n');
	}
}
#endif
void Init_servo(){

#if 0
	P10_PushPull_Mode;							//Servo control pin
	
	TIMER0_MODE0_ENABLE;                        //Timer 0 and Timer 1 mode configuration
	clr_T0M;
    TMOD |= 0x01;                           		//Timer0 is 16-bit mode
    TL0 = LOBYTE(TIMER_DIV12_VALUE_10ms); 		//Find  define in "Function_define.h" "TIMER VALUE"
    TH0 = HIBYTE(TIMER_DIV12_VALUE_10ms);
	set_ET0;                                    //enable Timer0 interrupt
	set_TR0;                                    //Timer0 run
#endif
}

int set_angle(int angle){
	angle+=90;	//angle range -90~90
	if(angle<0){
		angle=0;
	}
	if(angle>180){
		angle=180;
	}
	servo_angle=SERVO_LEFT+((float)(SERVO_RIGHT-SERVO_LEFT))/180*angle;
	PWM2H = HIBYTE(servo_angle);			//L9110 PWM 0
	PWM2L = LOBYTE(servo_angle);
	set_LOAD;
	return angle-90;
}
/************************************************************************************************************
*   	Motor Control
************************************************************************************************************/
#define SPEED_MIN		3999
#define SPEED_MAX		39999

int Motor_move(int speed){			//speed range -10~10
	int speed_load=0;
	if(speed>10){
		speed=10;
	}
	if(speed<-10){
		speed=-10;
	}
	//speed=speed*200;
	if(speed==0){
		PWM0H = HIBYTE(0);		//L9110 PWM HIGH
		PWM0L = LOBYTE(0);
		PWM1H = HIBYTE(0);			//L9110 PWM 0		
		PWM1L = LOBYTE(0);
	}
	if(speed>0){
		speed_load=(SPEED_MAX-SPEED_MIN)/10*speed+SPEED_MIN;
		PWM0H = HIBYTE(speed_load);		//L9110 PWM HIGH
		PWM0L = LOBYTE(speed_load);
		PWM1H = HIBYTE(0);			//L9110 PWM 0		
		PWM1L = LOBYTE(0);
	}
	if(speed<0){
		speed=-speed;
		speed_load=(SPEED_MAX-SPEED_MIN)/10*speed+SPEED_MIN;
		PWM0H = HIBYTE(0);			//L9110 PWM 0
		PWM0L = LOBYTE(0);
		PWM1H = HIBYTE(speed_load);		//L9110 PWM HIGH		
		PWM1L = LOBYTE(speed_load);
	}
	//PWM0_OUTPUT_INVERSE;
	set_LOAD;
	return speed;
}
void Init_motor(){
	PWM0_P12_OUTPUT_ENABLE;
	PWM1_P11_OUTPUT_ENABLE;

	PWM_IMDEPENDENT_MODE;
	
	PWM2_P10_OUTPUT_ENABLE;
	PWM_CLOCK_DIV_8;
	PWMPH = 0x9C;							//Setting PWM period
	PWMPL = 0x3F;
	
	PWM0H = 0x0B;							//PWM0 high duty = 1/2 PWM period
	PWM0L = 0xB8;	

	set_LOAD;
	set_PWMRUN;
}

//-----------------------------------------------------------------------------------
#define STR_LIMIT 	50
char Tmp_Buf[STR_LIMIT];
char Str_Buf[STR_LIMIT];
int Write_Num=0;
int Read_Num=0;
int Str_Ready=0;
int Data_Ready=0;

#if 0
uchar get_string()
{
    int result_num=0;
	int timeout=100;

	while(!RI && timeout--){
		Delay_100us(1);		//delay 25ms
	}
    while (RI) {
      Str_Buf[result_num++]=SBUF;
	  RI = 0;
	  if(result_num>=STR_LIMIT){
	  	break;
	  }
	  timeout=5000;
	  while(!RI && timeout--);
    }
	Str_Buf[result_num]=0;
    return result_num;
}
#else
int get_string(){
	int str_count=0;
	while(Tmp_Buf[Read_Num]){
		Str_Buf[str_count]=Tmp_Buf[Read_Num];
		Tmp_Buf[Read_Num]=0;
		str_count++;
		Read_Num++;
		if(Read_Num>=STR_LIMIT){
			Read_Num=0;
		}
		if(Str_Buf[str_count-1]=='\n'){
			break;
		}
	}
	Str_Buf[str_count]=0;
	return str_count;
}

void SerialPort0_ISR(void) interrupt 4 
{
    if (RI==1) 
    {                                       /* if reception occur */
        clr_RI;                             /* clear reception flag for next reception */
        Tmp_Buf[Write_Num] = SBUF;
		Write_Num++;
		if(Write_Num>=STR_LIMIT){
			Write_Num=0;
		}
    }
}
#endif

void StrTrim(char*pStr)  
{  
    char *pTmp = pStr;  
      
    while (*pStr != 0)   
    {  
        if (*pStr != ' ')  
        {  
            *pTmp++ = *pStr;  
        }  
        ++pStr;  
    }  
    *pTmp = 0;  
} 



void main (void) 
{
	//set_PD;
	Set_All_GPIO_Quasi_Mode;					// Define in Function_define.h
	//DEBUG_LED=0;
	InitialUART0_Timer1(115200);
	InitialUART1_Timer3(115200);
/*---------------------------------------------------------------*/
	P14_Input_Mode;
#if 1
	set_P0S_4;
	Enable_BIT4_FallEdge_Trig;
	button_trig_state=LOW;
	Enable_INT_Port1; 
#endif
/*---------------------------------------------------------------*/
	set_EA;                                     //enable interrupts
	set_EPI;
	set_ES;
/*---------------------------------Main function-----------------------------------------------*/
	Init_motor();
	Init_servo();
	set_angle(0);
	Motor_move(0);
	while(1){
		int count = get_string();
		//printf("get string\n");
		if(count>0){
			int speed=0, angle=0;
			char speed_str[8],angle_str[8];
			char * dot_pos;
			//printf("get:%s",Str_Buf);
			count = strlen(Str_Buf);
			dot_pos=strchr(Str_Buf,',');
			if(dot_pos!=NULL){
				memset(speed_str,0,sizeof(speed_str));
				memset(angle_str,0,sizeof(angle_str));
				strncpy(speed_str,Str_Buf,dot_pos-Str_Buf);
				strncpy(angle_str,dot_pos+1,count-(dot_pos-Str_Buf)-1);
				StrTrim(speed_str);
				StrTrim(angle_str);
				speed=atoi(speed_str);
				angle=atoi(angle_str);
				if(speed>=-10 && speed<=10)
					speed=Motor_move(speed);
				else
					speed=200;
				if(angle>=-90 && angle<=90)
					angle=set_angle(angle);
				else
					angle=400;
				//printf("set:%d,%d\n",speed,angle);
			}
			
		}
	}
}
