/*
 * dht11.c
 *
 *  Created on: Jul 30, 2023
 *      Author: Szymon
 */
#include "dht11.h"
#include "gpio.h"
#include "tim.h"

#define INIT_WAIT 1000 //wait to pass the unstable state
#define START_LOW 18 //start signal low state length in ms
#define START_HIGH 40 //start signal high state length in us
#define WAIT_FOR_DATA 160 //time from start signal's end to the start of data transfer in us
#define BIT_THRESHOLD 28 //bit = 1 if high state lasts longer than threshold, else bit = 0
#define HUM_FIRST_BIT 0 //position of first bit of humidity integer data in received array
#define HUM_LAST_BIT 7 //position of last bit of humidity integer data in received array
#define TEMP_FIRST_BIT 16 //position of first bit of temperature integer data in received array
#define TEMP_LAST_BIT 23 //position of last bit of temperature integer data in received array

static uint8_t received_data[40] = {0};
static TIM_HandleTypeDef *dhttimer;

HAL_StatusTypeDef dht11_init(TIM_HandleTypeDef *timer)
{
	dhttimer = timer;
	HAL_Delay(INIT_WAIT);
	return HAL_TIM_Base_Start(dhttimer);
}

static void delay_us(uint8_t us_value)
{
	__HAL_TIM_SET_COUNTER(dhttimer, 0);
	while(__HAL_TIM_GET_COUNTER(dhttimer) < us_value) {}
}

static uint8_t start_signal()
{
	HAL_GPIO_WritePin(DHT11_GPIO_Port, DHT11_Pin, GPIO_PIN_RESET);
	HAL_Delay(START_LOW);
	HAL_GPIO_WritePin(DHT11_GPIO_Port, DHT11_Pin, GPIO_PIN_SET);
	delay_us(START_HIGH);

	return HAL_GPIO_ReadPin(DHT11_GPIO_Port, DHT11_Pin);
}

static HAL_StatusTypeDef start_measurment(void)
{
	if(start_signal() == 0)
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
	while(HAL_GPIO_ReadPin(DHT11_GPIO_Port, DHT11_Pin) == 0) {}

	__HAL_TIM_SET_COUNTER(&htim6, 0);

	while (HAL_GPIO_ReadPin(DHT11_GPIO_Port, DHT11_Pin) == 1) {}

	if(__HAL_TIM_GET_COUNTER(&htim6) > BIT_THRESHOLD)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

 void dht11_read_data(void)
 {
	 if (start_measurment() == HAL_OK)
	 {
		 delay_us(WAIT_FOR_DATA);
		 for (int i = 0; i < 40; i++)
		 {
			 received_data[i] = read_bit();
		 }
	 }
	 else
	 {
		 for (int i = 0; i < 40; i++)
		 {
			 received_data[i] = 9;
		 }
	 }
 }

uint8_t dht11_get_temperature()
{
	uint8_t temp_reading = 0;

	for (int i = TEMP_FIRST_BIT; i <= TEMP_LAST_BIT; i++)
	{
		temp_reading <<= 1;
		temp_reading |= received_data[i];
	}

	return temp_reading;
}

uint8_t dht11_get_humidity()
{
	uint8_t hum_reading = 0;

	for (int i = HUM_FIRST_BIT; i <= HUM_LAST_BIT; i++)
	{
		hum_reading <<= 1;
		hum_reading |= received_data[i];
	}

	return hum_reading;
}
