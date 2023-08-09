/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "dma.h"
#include "i2c.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <math.h>

#include "dht11.h"
#include "lps25hb.h"
#include "lcd.h"

#include "hagl.h"
#include "font6x9.h"
#include "rgb565.h"

#include "icons.c"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
	if (hspi == &hspi2)
	{
		HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET);
	}
}

uint8_t num_places(uint8_t n) {
    if (n < 10) return 1;
    return 1 + num_places(n / 10);
}

int __io_putchar(int ch)
{
  if (ch == '\n') {
    __io_putchar('\r');
  }

  HAL_UART_Transmit(&huart2, (uint8_t*)&ch, 1, HAL_MAX_DELAY);

  return 1;
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_TIM6_Init();
  MX_USART2_UART_Init();
  MX_SPI2_Init();
  MX_I2C1_Init();
  /* USER CODE BEGIN 2 */
  void display_temperature()
  {
	  uint8_t temp = dht11_get_temperature();
	  uint8_t digits_num = num_places(temp);
	  char temp_char[digits_num];
	  sprintf(temp_char, "%u", temp);

	  wchar_t text[] = L"Temperature:   'C";

	  for (int i = 0; i < digits_num; i++)
	  {
		  text[12+i+(3-digits_num)] = temp_char[i];
	  }

	  lcd_draw_image_8(0, 0, 40, 40, temperature_icon);
	  hagl_put_text(text, 42, 16, YELLOW, font6x9);
	  lcd_copy();
  }

  void display_humidity()
  {
	  uint8_t hum = dht11_get_humidity();
	  uint8_t digits_num = num_places(hum);
	  char temp_char[digits_num];
	  sprintf(temp_char, "%u", hum);

	  wchar_t text[] = L"Humidity:   %";

	  for (int i = 0; i < digits_num; i++)
	  {
		  text[9+i+(3-digits_num)] = temp_char[i];
	  }

	  lcd_draw_image_8(0, 44, 40, 40, humidity_icon);
	  hagl_put_text(text, 42, 60, YELLOW, font6x9);
	  lcd_copy();
  }

//  float lps25hb_read_rel_pressure()
//  {
//	  const float h = 275; // height above sea level
//
//	  float temp = lps25hb_read_temp() + 273.15;
//	  float p = lps25hb_read_pressure();
//	  float p0 = p * exp(0.034162608734308 * h / temp);
//
//	  return p0;
//  }


  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  dht11_init(&htim6);
  lcd_init();

  if (lps25hb_init() == HAL_OK) {
    printf("OK: LPS25HB\n");
  } else {
    printf("Error: LPS25HB not found\n");
    Error_Handler();
  }
//  lps25hb_set_calib(-24);


//  lcd_draw_image_8(0, 0, 40, 40, temperature_icon);
//  lcd_draw_image_8(0, 44, 40, 40, humidity_icon);
//  hagl_put_text(L"Temperature:", 42, 16, YELLOW, font6x9);
//  hagl_put_text(L"Humidity:", 42, 60, YELLOW, font6x9);
//  lcd_copy();


  dht11_read_data();

  printf("Temperature: %d\n", dht11_get_temperature());
  printf("Humidity: %d\n", dht11_get_humidity());
  printf("Temperature LPS: %.1f*C\n", lps25hb_read_temp());
  printf("Pressure = %.1f hPa\n", lps25hb_read_pressure());
  printf("Relative pressure = %.1f hPa\n", lps25hb_read_rel_pressure());

  display_temperature();
  display_humidity();

  while (1)
  {
	  HAL_Delay(1500);
	  printf("Temperature LPS: %.1f*C\n", lps25hb_read_temp());
	  printf("Pressure = %.1f hPa\n", lps25hb_read_pressure());
	  printf("Relative pressure = %.1f hPa\n", lps25hb_read_rel_pressure());
//	  dht11_read_data();
//
//	  printf("Temperature: %d\n", dht11_get_temperature());
//	  printf("Humidity: %d\n", dht11_get_humidity());
//
//	  display_temperature();
//	  display_humidity();
//
//	  dht11_clear_data();
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = 0;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 40;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
