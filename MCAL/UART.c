/*
 * UART.c
 *
 *  Created on: Oct 11, 2019
 *      Author: Nmr
 */

#include "UART.h"





void init_UART(void)
{
	unsigned int Value_of_UBBR = 0;


//	    UCSRA= (1<<U2X);
		UCSRB = (1 << RXEN) | (1 << TXEN)|(1<<RXCIE);
       UCSRC = (1 << URSEL) | (3 << UCSZ0);
				UBRRL = 103;//51;
				UBRRH =0;
}


void UART_SendChar(char data)
{
	while (!( UCSRA & (1<<UDRE))) ;  /* Wait for empty transmit buffer */

	UDR = data;                     /* Put data into buffer, sends the data */
}


char UART_getChar(void)
{
	while (!(UCSRA & (1<<RXC)));   /* Wait until reveived flag set */

	return UDR;                   /* Return data in received buffer */
}


void UART_SEND_string(char *array)
{
	int i=0;
    while(array[i]!='\0')
    {

    	UART_SendChar(array[i]);
    	i++;
    }

}



void UART_SEND_number(unsigned int number)
{
   char buffer[20];

   utoa(number,buffer,10);
   UART_SEND_string(buffer);


}


void UART_SEND_number_signed(signed int number)
{
   char buffer[20];

   itoa(number,buffer,10);
   UART_SEND_string(buffer);


}




void UART_SEND_number_binary(int number)
{
    char c=0,k=0;

    for (c = 8; c > 0; c--)
     {
       k = number >> (c-1);

       if (k & 1)
    	   UART_SendChar('1');
       else
    	   UART_SendChar('0');

     }


}




void UART_SEND_number_binary_16_bits(int number)
{
    char c=0,k=0;

    for (c = 16; c > 0; c--)
     {
       k = number >> (c-1);

       if (k & 1)
    	   UART_SendChar('1');
       else
    	   UART_SendChar('0');

     }


}


void Set_Call_Back_fun(void (*PTR)(void))
{

ISR_UART_RX_fun=PTR;


}

ISR (USART_RXC_vect)
{

	ISR_UART_RX_fun();


}




