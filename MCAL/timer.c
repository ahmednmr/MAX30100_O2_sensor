/*
 * timer.c
 *
 *  Created on: Apr 28, 2020
 *      Author: embeddedfab
 */

#include"timer.h"



void timer_init (void)
{
// 16 Mega
	TCCR0=0b00001011;
	TCNT0=0;
	OCR0=249; //125;
	TIMSK |=0x02;
	TIFR  |=0x02;




//	TCCR1A=0x00;
//	TCCR1B=0b00001001;
//	TCNT1=0;
//	OCR1A=7999;                //it should be 7999  but this value is best practise
//	TIMSK=0b00010000;
//	TIFR=0b00010000;


	sei();
}


void timer_set_callBack_fun(void (*User_fun)(void))
{
	ISR_timer0_fun=User_fun;
}


//
//ISR(TIMER1_COMPA_vect)
//{
//	ISR_timer0_fun();
//
//}




ISR(TIMER0_COMP_vect)
{
	ISR_timer0_fun();
}

