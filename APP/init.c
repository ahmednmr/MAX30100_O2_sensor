/*
 * init.c
 *
 *  Created on: 5 Aug 2019
 *      Author: root
 */
#include"../main.h"

unsigned char value,First_stable_reading,heart_rate;
volatile unsigned int check;
MAX30100_reading my_sensor_reading;
unsigned char number_of_plulses_to_show_reading;

ret_state init_fun(void)
{

	//	sei();

	//	DDRD |=(1<<PD5);
	//	PORTD &=~(1<<PD5);
	EF_void_LCD_init();
	init_UART();
	//	spechial_timer_init();
	UART_SEND_string("\r\ninit MAX30100\r\n");
	MAX30100_init();

	EF_void_LCD_Clear_Screen();
	EF_void_LCD_print((unsigned char*)"done init LCD");

	UART_SEND_string("\r\ndone init\r\n");


	while(1)
	{


		value=MAX30100_get_reading(&my_sensor_reading);
		number_of_plulses_to_show_reading++;



		if(my_sensor_reading.Heart_rate==First_stable_reading)
		{
			heart_rate=my_sensor_reading.Heart_rate;
		}

		if(number_of_plulses_to_show_reading>6)
		{
			EF_void_LCD_Clear_Screen();
			EF_void_LCD_goto(1,1);
			EF_void_LCD_print((unsigned char*)"Heart Rate:");
			EF_void_LCD_goto(2,1);
			EF_void_LCD_print((unsigned char*)"SPO2      :");



			EF_void_LCD_goto(1,13);
			EF_void_LCD_print_Number((signed int)heart_rate);

			EF_void_LCD_goto(2,13);
			EF_void_LCD_print_Number((signed int)my_sensor_reading.SPO2);
			//			my_sensor_reading.SPO2=0;
			//				my_sensor_reading.Heart_rate=0;
			number_of_plulses_to_show_reading=0;
		}

		First_stable_reading=my_sensor_reading.Heart_rate;
	}



	return ret_operation;
}

