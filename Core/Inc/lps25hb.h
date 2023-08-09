#pragma once
#include "stm32l4xx.h"

// Initialize LPS25HB sensor
// Wake up and cyclic measurements
// return - HAL_OK or HAL_ERROR
HAL_StatusTypeDef lps25hb_init(void);

// Temperature reading
// return - result in Celsius degrees
float lps25hb_read_temp(void);

// Pressure reading
// return - result in hPa
float lps25hb_read_pressure(void);

//calibration
void lps25hb_set_calib(uint16_t value);
