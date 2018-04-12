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
#define RATE 2
#define SERVO_LEFT		(4*RATE)
#define SERVO_RIGHT		(23*RATE)
#define SERVO_MIDDLE	(15*RATE)
#define SEVOR_PIN 		P10
#define TIMER_DIV12_VALUE_50us			65536-65		//65*12/16000000 = 50 uS,  		// Timer divider = 12 

int timer_count=0;
int servo_angle=15*RATE;

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
	}
}
void Init_servo(){
	P10_PushPull_Mode;							//Servo control pin
	
	TIMER0_MODE0_ENABLE;                        //Timer 0 and Timer 1 mode configuration
	clr_T0M;
    TMOD |= 0x01;                           		//Timer0 is 16-bit mode
    TL0 = LOBYTE(TIMER_DIV12_VALUE_10ms); 		//Find  define in "Function_define.h" "TIMER VALUE"
    TH0 = HIBYTE(TIMER_DIV12_VALUE_10ms);
	set_ET0;                                    //enable Timer0 interrupt
	set_TR0;                                    //Timer0 run
}
void set_angle(int angle){
	if(angle<0){
		angle=0;
	}
	if(angle>180){
		angle=180;
	}
	servo_angle=SERVO_LEFT+((float)(SERVO_RIGHT-SERVO_LEFT))/180*angle;
}
/************************************************************************************************************
*   	Motor Control
************************************************************************************************************/
#define SPEED_MIN		0
#define SPEED_MAX		2000

void Motor_forward(int speed){
	PWM0H = HIBYTE(speed);		//L9110 PWM HIGH
	PWM0L = LOBYTE(speed);
	PWM1H = HIBYTE(0);			//L9110 PWM 0		
	PWM1L = LOBYTE(0);
	//PWM0_OUTPUT_INVERSE;
	set_LOAD;
}
void Motor_backward(int speed){
	PWM0H = HIBYTE(0);			//L9110 PWM 0
	PWM0L = LOBYTE(0);
	PWM1H = HIBYTE(speed);		//L9110 PWM HIGH		
	PWM1L = LOBYTE(speed);
	//PWM0_OUTPUT_INVERSE;
	set_LOAD;
}
void Motor_stop(){
	PWM0H = HIBYTE(0);			//L9110 PWM 0
	PWM0L = LOBYTE(0);
	PWM1H = HIBYTE(0);			//L9110 PWM 0		
	PWM1L = LOBYTE(0);
	//PWM0_OUTPUT_INVERSE;
	set_LOAD;
}
void Init_motor(){
	PWM0_P12_OUTPUT_ENABLE;
	PWM1_P11_OUTPUT_ENABLE;

	PWM_IMDEPENDENT_MODE;
	PWM_CLOCK_DIV_8;
	PWMPH = 0x07;
	PWMPL = 0xCF;
	
	set_PWMRUN;
}
//-----------------------------------------------------------------------------------

void main (void) 
{
	//set_PD;
	Set_All_GPIO_Quasi_Mode;					// Define in Function_define.h
	//DEBUG_LED=0;
	InitialUART0_Timer1(9600);
	InitialUART1_Timer3(115200);
	set_EA;                                     //enable interrupts
	set_EPI;
/*---------------------------------Main function-----------------------------------------------*/
	Init_servo();
	set_angle(90);
	Init_motor();
	while(1){
#if 1
		int i=0;
		for(i=0;i<=2000;i=i+100){
			Motor_forward(i);
			Delay_1ms(100);
			//Send_num(i);
		}
		for(i=180;i>=0;i--){
			set_angle(i);
			Delay_1ms(20);
		}
#endif
	}
}
