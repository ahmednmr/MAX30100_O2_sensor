/*
 * operation.c
 *
 *  Created on: 5 Aug 2019
 *      Author: root
 */

#include"../main.h"

ret_state operation_fun(void)
{

	EF_void_UART_SendString("Operation state\r\n");
	_delay_ms(2000);


return ret_terminate;
}
