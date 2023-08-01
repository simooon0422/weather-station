/*
 * dht11.h
 *
 *  Created on: Jul 30, 2023
 *      Author: Szymon
 */

#pragma once

#include "stm32l4xx.h"

// Initialization of 1-wire
// Start timer for us delay
// return - HAL_OK/HAL_ERROR
HAL_StatusTypeDef dht11_init(TIM_HandleTypeDef *timer);

// Read bits sent from sensor
void read_data(void);

// Read measured temperature
// return - temperature in Celsius degrees
uint8_t get_temperature();

// Read measured humidity
// return - relative humidity in %
uint8_t get_humidity();
