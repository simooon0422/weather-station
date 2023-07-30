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
HAL_StatusTypeDef dht11_init(void);

// Send start sequence to begin communication with dht11
// return - HAL_OK/HAL_ERROR
HAL_StatusTypeDef start_measurment(void);

// Read measured temperature
// return - temperature in Celsius degrees
uint8_t read_temperature();

// Read measured humidity
// return - relative humidity in %
uint8_t read_temperature();
