/*
 * MAX30100.c
 *
 *  Created on: Jun 22, 2020
 *      Author: embeddedfab
 */
#include"MAX30100.h"


const unsigned char spO2LUT[43] = {100,100,100,100,99,99,99,99,99,99,98,98,98,98,98,97,97,97,97,97,97,96,96,96,96,96,96,95,95,95,95,95,95,94,94,94,94,94,93,93,93,93,93};



//unsigned char IR_MOST[220]={0},IR_LEAST[220]={0},RED_MOST[220]={0},RED_LEAST[220]={0};
unsigned int IR_data[20]={0},RED_data[20]={0};
//signed int y_out[20]={0};
float filteredPulseValue_arr[50]={0};
float last_peek_value=0;
unsigned int filteredPulseValue_index=0;

float filteredPulseValue_arr_red[50]={0};
float last_filtered_value_red;
unsigned int filteredPulseValue_index_red=0;
unsigned char number_of_seconds=0;
unsigned int number_of_reading_per_second=0;

long long red_reading=0,ir_reading=0;
unsigned int  red_avg=0,ir_avg;

float max;
BOOLEAN peek_may_be_detected=FALSE;
BOOLEAN  peek_detected=FALSE;
unsigned char values_in_slope_carve_down=0;
unsigned char values_in_slope_carve_up=0;
long long first_peak=0,second_peak=0,peek=0,number_of_reading;


float arr_number_of_pulses[40]={0};
unsigned char arr_number_of_pulses_idx=0;
unsigned char pulses_count=0;
long long AVG_pulses_Value=0;
float final_filtered_value=0;

float arr_number_of_pulses_red[40]={0};
unsigned char arr_number_of_pulses_idx_red=0;
unsigned char pulses_count_red=0;
long long AVG_pulses_Value_red=0;
float final_filtered_value_red=0;

float redACValue_peek=0;
float irACValue_peek=0;


unsigned int arr_number_of_pulses_between_peeks[40]={0};
unsigned char arr_number_of_pulses_between_peeks_idx=0;
unsigned char peeks_count=0;
long long AVG_Peek_Value=0;

unsigned int IR_DC_value,RED_DC_value,MAX_RED,MAX_IR;
BOOLEAN first_ditect=TRUE;
BOOLEAN enable_ditect=TRUE;
BOOLEAN pulses_count_full_idx=FALSE;
BOOLEAN pulses_count_full_idx_red=FALSE;
BOOLEAN peeks_pulses_count_full_idx=FALSE;

BOOLEAN wait_for_pulse=TRUE;

void MAX30100_init(void)
{
	unsigned char status;


	TWI_Init();



	Write_data_to_REG(Interrupt_Enable_REG,ENB_A_FULL|ENB_TemP_RDY|ENB_HR_RDY|ENB_HR_RDY);
	Write_data_to_REG(Mode_Configuration_REG,0b00001011);
	Write_data_to_REG(SPO2_Configuration_REG,0b01000011);
	Write_data_to_REG(LED_Configuration_REG,0b10010011);

	_delay_ms(10);



	status=Read_data_From_REG(Interrupt_Status_REG);
	_delay_ms(10);
	UART_SEND_string("\r\nStatus in binary =  ");
	UART_SEND_number_binary(status);


	status=Read_data_From_REG(Temp_Integer_REG);
	_delay_ms(10);
	UART_SEND_string("\r\nTemp in Decimal =  ");
	UART_SEND_number(status);


	status=Read_data_From_REG(Mode_Configuration_REG);
	_delay_ms(10);
	UART_SEND_string("\r\nConfig in binary =  ");
	UART_SEND_number_binary(status);

	status=Read_data_From_REG(LED_Configuration_REG);
	_delay_ms(10);
	UART_SEND_string("\r\nLED Config in binary =  ");
	UART_SEND_number_binary(status);



	Write_data_to_REG(Interrupt_Enable_REG,0b11110000);

	Write_data_to_REG(Mode_Configuration_REG,0b00000011);
	Write_data_to_REG(FIFO_Write_Pointer_REG,0b00000000);
	Write_data_to_REG(Over_Flow_Counter_REG,0b00000000);
	Write_data_to_REG(FIFO_Read_Pointer_REG,0b00000000);


}
unsigned char MAX30100_get_reading(MAX30100_reading * my_reading)
{
	unsigned char status=0,data=0,stable_reading=0,SPO2=0,Ratio_of_Ratio=0,New_Beat=FALSE,num_available_samples=0,number_of_samples=0;
	unsigned int Reading=0,write_ptr=0,read_ptr=0,heart_rate=0,count_to_desplay_peek=0;
	unsigned int IR=0,max_IR=0,i=0,byte_number=0,num_samples_between_peeks=0,last_byte_number=0;
	static unsigned int k=0; ///for number of samples
	unsigned int min_red=0,min_ir=0;
	float redACValue=0;
	float irACValue=0;
	float filteredPulseValue=0;



	_delay_ms(100);
	//
	while(!(status&0b00010000))
	{
		status=Read_data_From_REG(Interrupt_Status_REG);
	}

	write_ptr=Read_data_From_REG(FIFO_Write_Pointer_REG);
	read_ptr=Read_data_From_REG(FIFO_Read_Pointer_REG);


	//	num_available_samples=(( 16 + write_ptr - read_ptr ) % 16);


	if(write_ptr>read_ptr)
	{
		num_available_samples=(write_ptr)-read_ptr;

	}
	else
	{
		num_available_samples=(write_ptr+16)-read_ptr;

	}



	TWI_Start();
	TWI_SendAddress(MAX30100_WRITE_ADDRESS);
	TWI_SendAddress(FIFO_Data_Register_REG);
	TWI_Start();
	TWI_SendAddress(MAX30100_READ_ADDRESS);

	for(i=0;i<(num_available_samples-1);i++)
	{

		TWI_ReceiveData_ACK(&data);
		IR_data[k] = data<<8;
		TWI_ReceiveData_ACK(&data);
		IR_data[k] |= data;

		TWI_ReceiveData_ACK(&data);
		RED_data[k]=data<<8;
		TWI_ReceiveData_ACK(&data);
		RED_data[k] |= data;
		k++;
	}

	if(num_available_samples!=0)
	{
		TWI_ReceiveData_ACK(&data);
		IR_data[k] = data<<8;
		TWI_ReceiveData_ACK(&data);
		IR_data[k] |= data;
		TWI_ReceiveData_ACK(&data);
		RED_data[k]=data<<8;
		TWI_ReceiveData_NACK(&data);
		RED_data[k] |= data;
		k++;
	}
	TWI_Stop();


	if(IR_data[0]>Acceptable_value)
	{
		min_ir=0;min_red=0;


		for(i=0;i<num_available_samples;i++)
		{

			red_reading+=RED_data[i];
			ir_reading+=IR_data[i];


		}

		red_reading/=num_available_samples;
		ir_reading/=num_available_samples;

		red_avg=(unsigned int)red_reading;
		ir_avg=(unsigned int)ir_reading;

		for(i=0;i<num_available_samples;i++)
		{

			if(min_ir<IR_data[i])
			{
				min_ir=IR_data[i];
			}
			if(min_red<RED_data[i])
			{
				min_red=RED_data[i];
			}

			irACValue = DCRemover_step(IR_data[i]);
			filteredPulseValue_arr[filteredPulseValue_index] = lpf_step(-irACValue);


			arr_number_of_pulses[arr_number_of_pulses_idx]=filteredPulseValue_arr[filteredPulseValue_index];
			arr_number_of_pulses_idx++;
			if(arr_number_of_pulses_idx>=9)
			{
				arr_number_of_pulses_idx=0;
				pulses_count=10;
				pulses_count_full_idx=TRUE;

			}
			else if(pulses_count_full_idx==FALSE)
			{
				pulses_count=arr_number_of_pulses_idx;

			}

			for(unsigned char j=0; j<pulses_count; j++)
			{
				AVG_pulses_Value+=arr_number_of_pulses[j];

			}
			AVG_pulses_Value/=pulses_count;




			final_filtered_value  =   filteredPulseValue_arr[filteredPulseValue_index]-AVG_pulses_Value;


			if((final_filtered_value>100)&&(wait_for_pulse==TRUE))
			{

				if(final_filtered_value>last_peek_value)
				{
					last_peek_value=final_filtered_value;
					peek=number_of_reading;
					irACValue_peek=IR_data[i];
					redACValue_peek=RED_data[i];
				}
				else if(final_filtered_value<last_peek_value)
				{
					//										UART_SEND_number_signed((signed int)300);
					//					UART_SEND_string("Beat");
					last_peek_value=0;
					wait_for_pulse=FALSE;
					first_peak=second_peak;
					second_peak=peek;

					num_samples_between_peeks =(unsigned int)second_peak-(unsigned int)first_peak;
					arr_number_of_pulses_between_peeks[arr_number_of_pulses_between_peeks_idx]=num_samples_between_peeks;

					arr_number_of_pulses_between_peeks_idx++;
					if(arr_number_of_pulses_between_peeks_idx>=4)
					{
						arr_number_of_pulses_between_peeks_idx=0;
						peeks_count=5;
						peeks_pulses_count_full_idx=TRUE;

					}
					else if(peeks_pulses_count_full_idx==FALSE)
					{
						peeks_count=arr_number_of_pulses_between_peeks_idx+1;
					}
					for(unsigned char j=0; j<peeks_count; j++)
					{
						AVG_Peek_Value+=arr_number_of_pulses_between_peeks[j];
					}
					AVG_Peek_Value/=peeks_count;

					//					UART_SEND_string("\r\n num_samples_between_peeks=  ");
					//					UART_SEND_number((unsigned int)AVG_Peek_Value);

					heart_rate=3000/AVG_Peek_Value;

					if(heart_rate<ACCEPTABLE_HEART_RATE_READING)
					{
						//					UART_SEND_string("\r\n heart_rate= ");
						//					UART_SEND_number((unsigned int)heart_rate);

						my_reading->Heart_rate=heart_rate;
					}

					irACValue_peek-=min_ir;
					redACValue_peek-=min_red;


//					my_reading->SPO2 = SpO2Calculator_update( irACValue_peek, redACValue_peek, TRUE);
					my_reading->SPO2 =Cal_SPO2(irACValue_peek, redACValue_peek,ir_avg ,red_avg );

				}

			}
			else
			{
				//								UART_SEND_number_signed((signed int)10);
				if(final_filtered_value<100)
				{
					wait_for_pulse=TRUE;
				}
			}



			//						UART_SEND_string("\r\n");






			filteredPulseValue_index++;
			number_of_reading++;

		}

	}


	else
	{
		my_reading->SPO2=0;
		my_reading->Heart_rate=0;
	}
	filteredPulseValue_index=0;
	k=0;

	return Reading;
}




unsigned int find_Max_MIN_in_Array(unsigned int* arr,unsigned int size)
{
	unsigned int i=0,max=0;
	signed int value=0;

	for(i=0;i<size;i++)
	{
		value=arr[i];
		if((value>max)&&(value>0))
		{
			max=value;
		}
	}


	return max;

}






void Write_data_to_REG(unsigned char REG,unsigned char DATA)
{
	TWI_Start();
	TWI_SendAddress(MAX30100_WRITE_ADDRESS);
	TWI_SendAddress(REG);
	TWI_SendData(DATA);
	TWI_Stop();

}


unsigned char Read_data_From_REG(unsigned char REG)
{
	unsigned char Reg_Reading;

	TWI_Start();
	TWI_SendAddress(MAX30100_WRITE_ADDRESS);
	TWI_SendAddress(REG);
	TWI_Start();
	TWI_SendAddress(MAX30100_READ_ADDRESS);
	TWI_ReceiveData_NACK(&Reg_Reading);
	TWI_Stop();

	return Reg_Reading;

}



float DCRemover_step(float x)
{
	float alpha=0.95;
	static float dcw=0;

	float olddcw = dcw;

	dcw = (float)x + alpha * olddcw;

	return dcw - olddcw;
}


float DCRemover_step_red(float x)
{
	float alpha=0.95;
	static float dcw_red=0;

	float olddcw_red = dcw_red;

	dcw_red = (float)x + alpha * olddcw_red;

	return dcw_red - olddcw_red;
}


float lpf_step(float x) //class II
{          static float v[2];
v[0]=0.0;


v[0] = v[1];
v[1] = (2.452372752527856026e-1 * x) + (0.50952544949442879485 * v[0]);
return
		(v[0] + v[1]);
}





unsigned int dcRemoval(unsigned int* x,float alpha)
{
	unsigned int t=0,DC_value=0;
	long long avg=0;

	for(t=0;t<100;t++)
	{
		avg+=x[t];


	}

	DC_value=avg/100;
	for(t=0;t<100;t++)
	{
		x[t]=x[t]-DC_value;
	}




	return DC_value;
}

void DRAW_RESULT(void)
{
	unsigned int i=0;

	for(i=0;i<filteredPulseValue_index;i++)
	{

		//		UART_SEND_number((unsigned int)i);
		//		UART_SEND_string("   ");

		UART_SEND_number_signed((signed int)filteredPulseValue_arr[i]);
		//		UART_SEND_string("   ");


		//		UART_SEND_number((unsigned int)RED_data[i]);
		UART_SEND_string("\r\n");
	}


}


unsigned char SpO2Calculator_update(float irACValue, float redACValue, unsigned char beatDetected)
{
	static float irACValueSqSum=0;
	static   float redACValueSqSum=0;
	static   unsigned char beatsDetectedNum=0;
	static    uint32_t samplesRecorded=0;
	unsigned char spO2=0;
	float acSqRatio=0;
	uint8_t index=0;



	irACValueSqSum += irACValue * irACValue;
	redACValueSqSum += redACValue * redACValue;
	++samplesRecorded;

	if (beatDetected) {
		++beatsDetectedNum;
		if (beatsDetectedNum == CALCULATE_EVERY_N_BEATS) {
			acSqRatio = 100.0 * log(redACValueSqSum/samplesRecorded) / log(irACValueSqSum/samplesRecorded);

			index = 0;

			if (acSqRatio > 66) {
				index = (uint8_t)acSqRatio - 66;
			} else if (acSqRatio > 50) {
				index = (uint8_t)acSqRatio - 50;
			}
			samplesRecorded = 0;
			redACValueSqSum = 0;
			irACValueSqSum = 0;
			beatsDetectedNum = 0;
			spO2 = 0;

			spO2 = spO2LUT[index];

			UART_SEND_string("\r\n SPO2 = ");
			UART_SEND_number((unsigned int)spO2);
			//			UART_SEND_string("\r\n");
			return spO2;
		}
	}
	return 0;
}



unsigned char Cal_SPO2(unsigned int IR_DC_value, unsigned int RED_DC_value,long long MAX_IR ,long long MAX_RED )
{
	float Ratio_red=0,Ratio_ir=0,Ratio_ratio=0;
	unsigned char spo_2=0;
static unsigned char last_spo2=0;

	//	IR_DC_value/=2;
	//	RED_DC_value/=2;


	Ratio_red=(float)RED_DC_value/(float)MAX_RED;
	Ratio_ir=(float)IR_DC_value/(float)MAX_IR;

	//	UART_SEND_string("\r\n IR_DC_value = ");
	//	UART_SEND_number((unsigned int)IR_DC_value);
	//
	//	UART_SEND_string("\r\n RED_DC_value = ");
	//	UART_SEND_number((unsigned int)RED_DC_value);
	//
	//	UART_SEND_string("\r\n MAX_IR = ");
	//	UART_SEND_number((unsigned int)MAX_IR);
	//
	//	UART_SEND_string("\r\n MAX_RED = ");
	//	UART_SEND_number((unsigned int)MAX_RED);
	//


	//	Ratio_red*=10000;
	//	Ratio_ir*=10000;
	Ratio_ratio=Ratio_red/Ratio_ir;
//		Ratio_ratio*=100;

	//	UART_SEND_string("\r\n Ratio_ir = ");
	//	UART_SEND_number((unsigned int)Ratio_ir);
	//
	//	UART_SEND_string("\r\n Ratio_red = ");
	//	UART_SEND_number((unsigned int)Ratio_red);

	Ratio_ratio *=25;
	spo_2 =110-Ratio_ratio;

	UART_SEND_string("\r\n spo_2 = ");
	UART_SEND_number((unsigned int)spo_2);


if((spo_2<100)&&(spo_2!=0))
{
	last_spo2=spo_2;
	return (unsigned char)spo_2;
}

UART_SEND_string("\r\n last_spo2 = ");
UART_SEND_number((unsigned int)last_spo2);

return last_spo2;

}



unsigned char Look_up_table(unsigned char Ratio_of_ratio)
{
	static unsigned char SPO2_value=0;

	if(Ratio_of_ratio<40)             						{ SPO2_value=100; }
	else if ((Ratio_of_ratio>40)&&(Ratio_of_ratio<47))    { SPO2_value=99;  }
	else if((Ratio_of_ratio>47)&&(Ratio_of_ratio<54))  				  { SPO2_value=98;  }
	else if ((Ratio_of_ratio>54)&&(Ratio_of_ratio<60))   			  { SPO2_value=97;  }
	else if ((Ratio_of_ratio>60)&&(Ratio_of_ratio<70))   			 { SPO2_value=96;  }
	else if ((Ratio_of_ratio>70)&&(Ratio_of_ratio<80))  				 { SPO2_value=95;  }
	else if ((Ratio_of_ratio>80)&&(Ratio_of_ratio<84))     { SPO2_value=94;  }
	else if ((Ratio_of_ratio>84)&&(Ratio_of_ratio<88))     { SPO2_value=93;  }
	else if ((Ratio_of_ratio>88)&&(Ratio_of_ratio<92))    { SPO2_value=92;  }
	else if ((Ratio_of_ratio>92)&&(Ratio_of_ratio<96))     { SPO2_value=91;  }
	else if ((Ratio_of_ratio>96)&&(Ratio_of_ratio<100))    { SPO2_value=90;  }
	else if ((Ratio_of_ratio>100)&&(Ratio_of_ratio<120))     { SPO2_value=86;  }
	else if ((Ratio_of_ratio>120)&&(Ratio_of_ratio<140))   { SPO2_value=80;  }
	else if ((Ratio_of_ratio>140)&&(Ratio_of_ratio<160))    { SPO2_value=70;  }
	else                                 { SPO2_value=0;  }


	return SPO2_value;
}


//prev = dcRemoval(signal, prev.w, 0.9);



//w_t = x[t]+(alpha*old_w_t) ;
//y_out[t]=w_t-old_w_t;
//
//old_w_t=w_t;
////		y_out[t]=x[t]-x[0];



/*
 *
				if(filteredPulseValue_arr[filteredPulseValue_index]>last_filtered_value)
				{

					values_in_slope_carve_up++;
					values_in_slope_carve_down=0;

					if(values_in_slope_carve_up>4)
					{
						peek_may_be_detected=TRUE;
						//					UART_SEND_number((unsigned int)9000);

					}

					peek=filteredPulseValue_index;

				}

				else if((peek_may_be_detected==TRUE)&&(filteredPulseValue_arr[filteredPulseValue_index]<last_filtered_value))
				{

					values_in_slope_carve_up=0;
					values_in_slope_carve_down++;

					if(values_in_slope_carve_down>4)
					{
						peek_detected=TRUE;
					}
					//				UART_SEND_number((unsigned int)8000);

				}

				if(peek_detected==TRUE)
				{
					max=0;
					values_in_slope_carve_up=0;
					values_in_slope_carve_down=0;
					peek_may_be_detected=FALSE;
					peek_detected=FALSE;


					first_peak=second_peak;
					second_peak=peek;


					//				UART_SEND_string("     detect peek at ");
					//				UART_SEND_number((unsigned int)second_peak);

				}
 */


/*
	if(0)//(filteredPulseValue_index>=288)
	{

		//						UART_SEND_string("\r\n\r\n filteredPulseValue_index= ");
		//							UART_SEND_number((unsigned int)filteredPulseValue_index);


		//						DRAW_RESULT();


		for(i=0;i<filteredPulseValue_index;i++)
		{





			if(filteredPulseValue_arr[i]>Acceptable_value)
			{

				UART_SEND_number_signed((signed int)filteredPulseValue_arr[i]);
				UART_SEND_string("      ");
				count_to_desplay_peek++;
				if((filteredPulseValue_arr[i]>filteredPulseValue_arr[i-9])&&(filteredPulseValue_arr[i]>filteredPulseValue_arr[i-8])&&(filteredPulseValue_arr[i]>filteredPulseValue_arr[i-7])&&(filteredPulseValue_arr[i]>filteredPulseValue_arr[i-6])&&(filteredPulseValue_arr[i]>filteredPulseValue_arr[i-5])&&(filteredPulseValue_arr[i]>filteredPulseValue_arr[i-4])&&(filteredPulseValue_arr[i]>filteredPulseValue_arr[i-3])&&(filteredPulseValue_arr[i]>filteredPulseValue_arr[i-2])&&(filteredPulseValue_arr[i]>filteredPulseValue_arr[i-1])&&(filteredPulseValue_arr[i]>filteredPulseValue_arr[i+1])&&(filteredPulseValue_arr[i]>filteredPulseValue_arr[i+2]&&(filteredPulseValue_arr[i]>filteredPulseValue_arr[i+3])&&(filteredPulseValue_arr[i]>filteredPulseValue_arr[i+4])&&(filteredPulseValue_arr[i]>filteredPulseValue_arr[i+5])&&(filteredPulseValue_arr[i]>filteredPulseValue_arr[i+6])&&(filteredPulseValue_arr[i]>filteredPulseValue_arr[i+7])&&(filteredPulseValue_arr[i]>filteredPulseValue_arr[i+8])&&(filteredPulseValue_arr[i]>filteredPulseValue_arr[i+9])))
				{
					//					peek=i;



					UART_SEND_number((unsigned int)9000);


					first_peak=second_peak;
					second_peak=peek;


					//					UART_SEND_string("\r\n\r\n\r\nPEEK   ");

					if(first_peak!=0)
					{
						if(second_peak>first_peak)
						{
							num_samples_between_peeks=second_peak-first_peak;


						}
						else
						{
							num_samples_between_peeks=(second_peak+filteredPulseValue_index)-first_peak;
						}

						UART_SEND_string("\r\n\r\n num_samples_between_peeks= ");
						UART_SEND_number((unsigned int)num_samples_between_peeks);



					}



					//					stable_reading++;
					//
					//					if(stable_reading==1)
					//					{
					//						last_byte_number=i;
					//					}
					//					else if(stable_reading==2)
					//					{
					//						byte_number=i;
					//						num_samples_between_peeks=byte_number-last_byte_number;
					//						stable_reading=0;
					//
					//						heart_rate=(60000/(num_samples_between_peeks*20));
					//
					//						if((heart_rate>30)&&(heart_rate<200))
					//						{
					//							UART_SEND_string("\r\n\r\n Heart Rate = ");
					//							UART_SEND_number((unsigned int)heart_rate);
					//							my_reading->Heart_rate=(unsigned char)heart_rate;
					//						}
					//						New_Beat=TRUE;
					//					}

					count_to_desplay_peek=0;

				}
				else
				{

					UART_SEND_number((unsigned int)8000);
				}

				UART_SEND_string("\r\n");
			}










		}


		filteredPulseValue_index=0;
		Write_data_to_REG(Mode_Configuration_REG,0b00001011);

	}













						arr_number_of_pulses_between_peeks[arr_number_of_pulses_between_peeks_idx]=num_samples_between_peeks;


						arr_number_of_pulses_between_peeks_idx++;
						if(arr_number_of_pulses_between_peeks_idx>=9)
						{
							arr_number_of_pulses_between_peeks_idx=0;
							peeks_count=10;
							peeks_pulses_count_full_idx=TRUE;

						}
						else if(peeks_pulses_count_full_idx==FALSE)
						{
							peeks_count=arr_number_of_pulses_between_peeks_idx+1;
						}



						for(unsigned char j=0; j<peeks_count; j++)
						{
							AVG_Peek_Value+=arr_number_of_pulses_between_peeks[j];
						}
						AVG_Peek_Value/=peeks_count;


						heart_rate=3000/AVG_Peek_Value;

						//						heart_rate=(60000/(AVG_Peek_Value*20));
						//
						//						UART_SEND_string("\r\n heart_rate= ");
						//						UART_SEND_number((unsigned int)heart_rate);





///////////
 *
 *
 *



 */

