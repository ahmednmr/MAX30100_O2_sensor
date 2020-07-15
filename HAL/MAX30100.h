/*
 * MAX30100.h
 *
 *  Created on: Jun 22, 2020
 *      Author: embeddedfab
 */

#ifndef HAL_MAX30100_H_
#define HAL_MAX30100_H_

#include"../MCAL/i2c.h"
#include"../MCAL/UART.h"
#include"../ServiceLayer/std_types.h"
#include <stdio.h>
#include <math.h>
#include<util/delay.h>


#define CALCULATE_EVERY_N_BEATS    3
#define Acceptable_value 1000
#define ACCEPTABLE_HEART_RATE_READING 300

#define MAX30100_WRITE_ADDRESS		0xAE
#define MAX30100_READ_ADDRESS		0xAF

//STATUS
#define	Interrupt_Status_REG		0x00	//	R

#define	Interrupt_Enable_REG		0x01    //	R/W
#define ENB_A_FULL                 (1<<7)
#define ENB_TemP_RDY  			   (1<<6)
#define ENB_HR_RDY    			   (1<<5)
#define ENB_SO2_RDY   			   (1<<4)

//FIFO
#define	FIFO_Write_Pointer_REG		0x02    //	R/W
#define	Over_Flow_Counter_REG		0x03    //	R/W
#define	FIFO_Read_Pointer_REG		0x04    //	R/W
#define	FIFO_Data_Register_REG		0x05    //	R/W

//CONFIGURATION
#define	Mode_Configuration_REG		0x06    //	R/W
#define	SPO2_Configuration_REG		0x07    //	R/W
#define	LED_Configuration_REG		0x09    //	R/W

//TEMPERATURE
#define	Temp_Integer_REG			0x16    //	R/W
#define	Temp_Fraction_REG			0x17    //	R/W

//PART ID
#define	Revision_ID_REG				0xFE    //	R
#define	Part_ID_REG					0xFF    //	R/W

typedef struct
{
	unsigned char Heart_rate;
	unsigned char SPO2;

}MAX30100_reading;


void MAX30100_init(void);
unsigned char MAX30100_get_reading(MAX30100_reading * my_reading);
unsigned int find_Max_MIN_in_Array(unsigned int* arr,unsigned int size);
void Write_data_to_REG(unsigned char REG,unsigned char DATA);
unsigned char Read_data_From_REG(unsigned char REG);
float DCRemover_step(float x);
float DCRemover_step_red(float x);
float lpf_step(float x);

unsigned int find_Max_IR(unsigned int* arr);
unsigned int dcRemoval(unsigned int* x,float alpha);
void DRAW_RESULT(void);

unsigned char SpO2Calculator_update(float irACValue, float redACValue, unsigned char beatDetected);
unsigned char Cal_SPO2(unsigned int IR_DC_value, unsigned int RED_DC_value,long long MAX_IR ,long long MAX_RED );
unsigned char Look_up_table(unsigned char Ratio_of_ratio);




#endif /* HAL_MAX30100_H_ */
