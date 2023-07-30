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

HAL_StatusTypeDef start_measurment(void)
{
	int response;

	HAL_GPIO_WritePin(DHT11_GPIO_Port, DHT11_Pin, GPIO_PIN_RESET);
	HAL_Delay(20);
	HAL_GPIO_WritePin(DHT11_GPIO_Port, DHT11_Pin, GPIO_PIN_SET);
	delay_us(40);
	response = HAL_GPIO_ReadPin(DHT11_GPIO_Port, DHT11_Pin);
	delay_us(80);

	if(response == 0)
	{
		return HAL_OK;
	}
	else
	{
		return HAL_ERROR;
	}
}

//static int read_bit(void)
//{
//
//}
//
// read_data(void)
// {
//
// }
//
//uint8_t read_temperature()
//{
//
//}
//
//uint8_t read_humidity()
//{
//
//}

