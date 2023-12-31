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
#include <stdlib.h>
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
volatile uint8_t current_screen = 0;

//Function to handle interrupts (debouncing buttons)
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  if ((GPIO_Pin == BUTTON_INC_Pin) && (__HAL_TIM_GET_COUNTER(&htim7) > 3000))
  {
	  __HAL_TIM_SET_COUNTER(&htim7, 0);
	  if (current_screen == 3)
	  {
		  current_screen = 0;
	  }
	  else current_screen++;
  }
  else if ((GPIO_Pin == BUTTON_DEC_Pin) && (__HAL_TIM_GET_COUNTER(&htim7) > 3000))
  {
	  __HAL_TIM_SET_COUNTER(&htim7, 0);
	  if (current_screen == 0)
	  {
		  current_screen = 3;
	  }
	  else current_screen--;
  }
}


void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
	if (hspi == &hspi2)
	{
		HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET);
	}
}

//Function returning number of digits in a number
uint8_t num_places(uint16_t n) {
    if (n < 10) return 1;
    return 1 + num_places(n / 10);
}

//Function to use printf() with UART
int __io_putchar(int ch)
{
  if (ch == '\n') {
    __io_putchar('\r');
  }

  HAL_UART_Transmit(&huart2, (uint8_t*)&ch, 1, HAL_MAX_DELAY);

  return 1;
}

//Function to map values
int map(int x, int in_min, int in_max, int out_min, int out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
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
  MX_TIM7_Init();
  /* USER CODE BEGIN 2 */

  //Arrays storing values of 25 last measurements
  uint8_t last_25_temp[25] = {0};
  uint8_t last_25_hum[25] = {0};
  uint16_t last_25_pres[25] = {0};

  //Arrays storing values for scales in charts
  char hours[5][3] = {" 0 ", "-6 ", "-12", "-18", "-24"};
  char t_scale[6][3] = {"-10", "  0", " 10", " 20", " 30", " 40"};
  char h_scale[5][3] = {"  0", " 25", " 50", " 75", "100"};
  char p_scale[5][4] = {"990 ", "1000", "1010", "1020", "1030"};

  //Function to initialize sensors and lcd display
  void initialize_peripherals()
  {
	  HAL_TIM_Base_Start(&htim7);

	  //initialize dht11 sensor
	  if (dht11_init(&htim6) == HAL_OK) {
	    printf("OK: DHT11 timer started\n");
	  } else {
	    printf("Error: DHT11 timer error\n");
	    Error_Handler();
	  }
	  //initialize lps25hb sensor
	  if (lps25hb_init() == HAL_OK) {
	    printf("OK: LPS25HB found\n");
	  } else {
	    printf("Error: LPS25HB not found\n");
	    Error_Handler();
	  }
	  lps25hb_set_calib(-24); //calibrate lps25hb sensor

	  //initialize lcd display
	  lcd_init();
  }

  //Function to read data from sensors and store them in arrays
  void read_and_store_data()
  {
	  for (int i = 0; i < 24; i++)
	  {
		  last_25_temp[i] = last_25_temp[i+1];
		  last_25_hum[i] = last_25_hum[i+1];
		  last_25_pres[i] = last_25_pres[i+1];
	  }

	  dht11_read_data();

	  last_25_temp[24] = dht11_get_temperature();;
	  last_25_hum[24] = dht11_get_humidity();
	  last_25_pres[24] = roundf(lps25hb_read_rel_pressure());
  }

  //Function to display temperature icon and value on lcd display
  void display_temperature(uint8_t temp)
  {
	  uint8_t digits_num = num_places(temp);
	  char temp_char[digits_num];
	  sprintf(temp_char, "%u", temp);

	  wchar_t text[] = L"Temperature:   'C";

	  for (int i = 0; i < digits_num; i++)
	  {
		  text[12+i+(3-digits_num)] = temp_char[i];
	  }

	  lcd_draw_image_8(0, 0, 40, 40, temperature_icon);
	  hagl_put_text(text, 42, 16, GREEN, font6x9);
  }

  //Function to display humidity icon and value on lcd display
  void display_humidity(uint8_t hum)
  {
	  uint8_t digits_num = num_places(hum);
	  char temp_char[digits_num];
	  sprintf(temp_char, "%u", hum);

	  wchar_t text[] = L"Humidity:   %";

	  for (int i = 0; i < digits_num; i++)
	  {
		  text[9+i+(3-digits_num)] = temp_char[i];
	  }

	  lcd_draw_image_8(0, 44, 40, 40, humidity_icon);
	  hagl_put_text(text, 42, 60, GREEN, font6x9);
  }

  //Function to display pressure icon and value on lcd display
  void display_pressure(uint16_t pres)
  {
	  uint8_t digits_num = num_places(pres);
	  char temp_char[digits_num];
	  sprintf(temp_char, "%u", pres);

	  wchar_t text[] = L"Pressure:     hPa";

	  for (int i = 0; i < digits_num; i++)
	  {
		  text[10+i+(4-digits_num)] = temp_char[i];
	  }

	  lcd_draw_image_8(0, 88, 40, 40, pressure_icon);
	  hagl_put_text(text, 42, 104, GREEN, font6x9);
  }

  ///////////////////////DRAWING CHARTS FUNCTIONS BEGIN////////////////////////////////////

  //Function to draw chart's axes
  void draw_axes()
  {
	  uint8_t x0 = 20;
	  uint8_t x_length = 135;
	  uint8_t x_offset = 30;

	  uint8_t y0 = 105;
	  uint8_t y_start = 10;
	  uint8_t y_length = 118;

	  uint8_t divider_length = 5;

	  // X Axis
	  hagl_draw_hline(x0, y0, x_length, YELLOW);
	  for (int i = 0; i < 5; i++)
	  {
		  hagl_draw_vline(x_offset + i * 30, y0 - divider_length, divider_length, YELLOW);
	  }

	  // Y Axis
	  hagl_draw_vline(x0, y_start, y_length, YELLOW);
	  for (int i = 0; i < 6; i++)
	  {
		  hagl_draw_hline(x0, i * 20 + 25, divider_length, YELLOW);
	  }

  }

  //Function to draw chart's scales depending on parameter
  void draw_scales(char par)
  {
	  uint8_t y0 = 105;
	  uint8_t x_desc_start = 2;
	  uint8_t y_desc_start = y0 + 5;

	  // X Axis
	  hagl_put_text(L"[h]", 140, y0 - 10, RED, font6x9);

	  for (int i = 0; i < 5; i++)
	  {
		  wchar_t text[] = L"   ";
		  for (int j = 0; j < 3; j++)
		  {
			  text[j] = hours[i][j];
		  }
		  hagl_put_text(text, 143 - i * 30, y_desc_start, RED, font6x9);
	  }

	  // Y Axis
	  switch(par)
	  {
	  //Temperature
	  case 't':
		  // Title
		  hagl_put_text(L"TEMPERATURE", 50, 5, GREEN, font6x9);
		  hagl_put_text(L"['C]", x_desc_start, 5, RED, font6x9);

		  //Y scale
		  for (int i = 0; i < 6; i++)
		  {
			  wchar_t text[] = L"   ";
			  for (int j = 0; j < 3; j++)
			  {
				  text[j] = t_scale[i][j];
			  }
			  hagl_put_text(text, x_desc_start, 120 - i * 20, RED, font6x9);
		  }
		  break;
	//Humidity
	  case 'h':
		  // Title
		  hagl_put_text(L"HUMIDITY", 60, 5, GREEN, font6x9);
		  hagl_put_text(L"[%]", x_desc_start+5, 5, RED, font6x9);

		  //Y scale
		  for (int i = 0; i < 5; i++)
		  {
			  wchar_t text[] = L"   ";
			  for (int j = 0; j < 3; j++)
			  {
				  text[j] = h_scale[i][j];
			  }
			  hagl_put_text(text, x_desc_start, 100 - i * 20, RED, font6x9);
		  }
		  break;
	//Pressure
	  case 'p':
		  // Title
		  hagl_put_text(L"PRESSURE", 60, 5, GREEN, font6x9);
		  hagl_put_text(L"[hPa]", x_desc_start, 5, RED, font6x9);

		  //Y scale
		  for (int i = 0; i < 5; i++)
		  {
			  wchar_t text[] = L"    ";
			  for (int j = 0; j < 4; j++)
			  {
				  text[j] = p_scale[i][j];
			  }
			  hagl_put_text(text, x_desc_start, 100 - i * 20, RED, font6x9);
		  }
		  break;

	  default:
		  lcd_clear();
		  break;
	  }
  }

  //Function to draw chart's data (dots and lines)
  void draw_data(char par)
  {
	  uint8_t x_pos = 30;
	  uint8_t x_increment = 5;
	  uint8_t y0 = 105;

	  switch(par)
	  {
	  case 't':
		  for (int i = 0; i < 25; i++)
		  {
			  hagl_fill_circle(x_pos, y0 - map(last_25_temp[i], -10, 40, -20, 80), 2, RED);
			  if(i < 24)
			  {
				  hagl_draw_line(x_pos, y0 - map(last_25_temp[i], -10, 40, -20, 80), x_pos + x_increment, y0 - map(last_25_temp[i+1], -10, 40, -20, 80), RED);
			  }
			  x_pos += x_increment;
		  }
		  break;
	  case 'h':
		  for (int i = 0; i < 25; i++)
		  {
			  hagl_fill_circle(x_pos, y0 - map(last_25_hum[i], 0, 100, 0, 80), 2, CYAN);
			  if(i < 24)
			  {
				  hagl_draw_line(x_pos, y0 - map(last_25_hum[i], 0, 100, 0, 80), x_pos + x_increment, y0 - map(last_25_hum[i+1], 0, 100, 0, 80), CYAN);
			  }
			  x_pos += x_increment;
		  }
		  break;
	  case 'p':
		  for (int i = 0; i < 25; i++)
		  {
			  hagl_fill_circle(x_pos, y0 - map(last_25_pres[i], 990, 1030, 0, 80), 2, MAGENTA);
			  if(i < 24)
			  {
				  hagl_draw_line(x_pos, y0 - map(last_25_pres[i], 990, 1030, 0, 80), x_pos + x_increment, y0 - map(last_25_pres[i+1], 990, 1030, 0, 80), MAGENTA);
			  }
			  x_pos += x_increment;
		  }
		  break;
	  default:
		  lcd_clear(); //clear lcd if parameter is incorrect
		  break;
	  }

  }

  //Function to draw full chart of temperature, humidity or pressure
  void draw_chart(char measurement)
  {
	  lcd_clear();
	  draw_axes();
	  draw_scales(measurement);
	  draw_data(measurement);
	  lcd_copy();
  }

  ///////////////////////DRAWING CHARTS FUNCTIONS END////////////////////////////////////

  //Function displaying main screen with values and icons of temperature, humidity and pressure
  void main_screen(void)
  {
	  lcd_clear();
	  display_temperature(last_25_temp[24]);
	  display_humidity(last_25_hum[24]);
	  display_pressure(last_25_pres[24]);
	  lcd_copy();
  }

  //Update currently displayed screen
  void update_display(uint8_t screen)
  {
	  switch(screen)
	  {
	  case 0:
		  main_screen();
		  break;
	  case 1:
		  draw_chart('t');
		  break;
	  case 2:
		  draw_chart('h');
		  break;
	  case 3:
		  draw_chart('p');
		  break;
	  default:
		  lcd_clear();
	  }
  }

  //Function to control data on UART
  void uart_overseer()
  {
	  printf("Temperature: %d\n", dht11_get_temperature());
	  printf("Humidity: %d\n", dht11_get_humidity());
	  printf("Temperature LPS: %.1f*C\n", lps25hb_read_temp());
	  printf("Pressure float= %.1f hPa\n", lps25hb_read_rel_pressure());
	  printf("Relative pressure = %u hPa\n", last_25_pres[24]);
	  printf("Screen: %u\n", current_screen);
  }

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  initialize_peripherals();

  while (1)
  {
	  read_and_store_data();
	  update_display(current_screen);
	  uart_overseer();
	  HAL_Delay(1500);

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
