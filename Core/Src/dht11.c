/*
 * dht11.c
 *
 *  Created on: Jul 30, 2023
 *      Author: Szymon
 */
#include "dht11.h"
#include "gpio.h"
#include "tim.h"

uint8_t received_data[40] = {0};


HAL_StatusTypeDef dht11_init(void)
{
	HAL_Delay(1000);
	return HAL_TIM_Base_Start(&htim6);
}

void delay_us(uint8_t us_value)
{
	__HAL_TIM_SET_COUNTER(&htim6, 0);
	while(__HAL_TIM_GET_COUNTER(&htim6) < us_value) {}
}

static HAL_StatusTypeDef start_measurment(void)
{
	int response;

	HAL_GPIO_WritePin(DHT11_GPIO_Port, DHT11_Pin, GPIO_PIN_RESET);
	HAL_Delay(18);
	HAL_GPIO_WritePin(DHT11_GPIO_Port, DHT11_Pin, GPIO_PIN_SET);
	delay_us(40);
	response = HAL_GPIO_ReadPin(DHT11_GPIO_Port, DHT11_Pin);
	delay_us(160);
	return HAL_OK;
	if(response == 0)
	{
		return HAL_OK;
	}
	else
	{
		return HAL_ERROR;
	}
}

static int read_bit(void)
{
	uint8_t reading;
	reading = HAL_GPIO_ReadPin(DHT11_GPIO_Port, DHT11_Pin);
	while(reading == 0)
	{
		reading = HAL_GPIO_ReadPin(DHT11_GPIO_Port, DHT11_Pin);
	}
	__HAL_TIM_SET_COUNTER(&htim6, 0);
	while (reading == 1)
	{
		reading = HAL_GPIO_ReadPin(DHT11_GPIO_Port, DHT11_Pin);
	}

	if(__HAL_TIM_GET_COUNTER(&htim6) >= 50)
	{
		reading = 1;
	}
	else if (__HAL_TIM_GET_COUNTER(&htim6) < 50)
	{
		reading = 0;
	}
	else
	{
		reading = 3;
	}

//	delay_us(50);
////	while(reading == 0)
////	{
////		reading = HAL_GPIO_ReadPin(DHT11_GPIO_Port, DHT11_Pin);
////	}
//
//	__HAL_TIM_SET_COUNTER(&htim6, 0);
//	while (reading == 1)
//	{
//		reading = HAL_GPIO_ReadPin(DHT11_GPIO_Port, DHT11_Pin);
//	}
//
//	if(__HAL_TIM_GET_COUNTER(&htim6) > 28)
//	{
//		reading = 1;
//	}

	return reading;
}

 void read_data(void)
 {
	 if (start_measurment() == HAL_OK)
	 {
		 for (int i = 0; i < 40; i++)
		 {
			 received_data[i] = read_bit();
		 }
	 }
 }

//uint8_t get_temperature()
//{
//
//}
//
//uint8_t get_humidity()
//{
//
//}

