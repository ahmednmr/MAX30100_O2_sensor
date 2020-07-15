/*
 * operation.c
 *
 *  Created on: 5 Aug 2019
 *      Author: root
 */

#include"../main.h"


 unsigned char First_Reading=TRUE,Status=0,temp_value;
//unsigned char t=0;
unsigned long int CO2_reading_numbers,CO2_AVG,TVOC_reading_numbers,TVOC_AVG;




ret_state operation_fun(void)
{



//
//	if(timer_check(update_screen_timer_ID))
//	{
//		timer_reset(update_screen_timer_ID);
//
//		EF_void_LCD_Clear_Screen();
//		EF_void_LCD_print((unsigned char*)"CO2");
//
//	}
//
//
//
//
//
//
//	if(timer_check(server_update_timer_ID))
//	{
//
//		timer_reset(server_update_timer_ID);
//
//
//		PORTD |=(1<<PD5);               //buzzer on
//		_delay_ms(200);
//		PORTD &=~(1<<PD5);              //buzzer off
//
//
//
//	}


	return ret_operation;
}
