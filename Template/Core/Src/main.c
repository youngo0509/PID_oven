/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
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
#include "adc.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include "gpio_if.h"
#include "pwm.h"
#include "pid.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define TX_TIMEOUT_MS  100   /**< Transmission time timeout over UART */
#define DELAY_MS       100   /**< Delay timeout */
#define GPIO_LD2_ENABLED 0 /**< Use LD2 as GPIO driven LED. It needs special config in IOC */

#define BUFFER_RECEIVE_SIZE 2

float temperature;

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
volatile uint8_t interruptflag=0;
uint8_t buf_receive[BUFFER_RECEIVE_SIZE]; //SPI buffer to read temperature
float temperaturesetpoint = 150; //Temperature setpoint
float pid_out=0; //Test variable for PWM
float pid_out_final_1=0; //Test variable for PWM
float error = 0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
#if GPIO_LD2_ENABLED
  t_gpio_pin user_led_pin = {LD4_GPIO_Port, LD4_Pin};
  t_gpio_if user_led;
#endif
  t_pwm pwm;
  SPid pid;

  //set P, I and D parameters
  pid.propGain = 2;
  pid.integratGain = 0.018;
  pid.derGain = 0;
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
  MX_USART2_UART_Init();
  MX_ADC1_Init();
  MX_TIM2_Init();
  MX_SPI1_Init();
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

  /* Init custom GPIO */
#if GPIO_LD2_ENABLED
  gpio_if_init(&user_led, ACTIVE_HIGH, &user_led_pin, GPIO_IF_CLEAR);
  if (gpio_if_open(&user_led) != GPIO_IF_SUCCESS)
  {
    Error_Handler();
  }
#endif

  /* Init PWM */
  pwm_init(&pwm, &htim2, TIM_CHANNEL_1, COUNTER_PERIOD_VALUE);
  if (pwm_open(&pwm) != PWM_SUCCESS)
  {
    Error_Handler();
  }

  /* Welcome message */
  printf("ELO301 PID Horno Init\r\n");

  while (1)
  {
#if GPIO_LD2_ENABLED
    /* Blink user LED */
    gpio_if_toggle(&user_led);
#endif

    HAL_Delay(10*DELAY_MS);

    /* read sensor temperature data using SPI*/
    HAL_GPIO_WritePin (GPIOA,CS_Pin, GPIO_PIN_RESET);
    HAL_SPI_Receive(&hspi1, buf_receive, BUFFER_RECEIVE_SIZE, DELAY_MS);
    HAL_GPIO_WritePin (GPIOA,CS_Pin, GPIO_PIN_SET);

    /*Combine two buffer elements into one sensor data*/
    uint16_t sensorData = ((uint16_t)buf_receive[0] << 8) | (buf_receive[1]);

    // Extracting magnitude bits (bits 3 to 14)
    uint16_t magnitude = (sensorData & 0x7FF8)>>3;

    // Calculate the temperature with LSB corresponding to 0.25°C
     temperature = magnitude * 0.25;

    // Print the temperature
    uint16_t temperature_display=(uint16_t)temperature;
    printf("Temperature: %u C\n\r", temperature_display);
    uint16_t duty_cycle_display=(uint16_t)pid_out;
    printf("PID duty cycle: %u percent \n\r", duty_cycle_display);




    /* calculate error */
   error = temperaturesetpoint - temperature;
   uint16_t error_display = (uint16_t)error;
   printf("error: %u degrees \n\r",error_display);


    /* run PID */
    pid_out=UpdatePID(&pid,error);
    if (pid_out > 100)
    	pid_out = 100;
    else if (pid_out <= 0)
    	    	pid_out = 0;
    pid_out_final_1 = pid_out;


   if(interruptflag==1){
	   interruptflag = 0;
	   pwm_update(&pwm,pid_out);
	   printf("Zero-cross detected \n\r");
   }





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
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 10;
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

/**
 * @brief Function that bridges HAL with printf
 * @param file Not in use
 * @param ptr Pointer to data to be printed out
 * @param len Length of the data to be printed out
 * @return len
 */

int _write(int file, char *ptr, int len)
{
  if (HAL_UART_Transmit(&huart2, (uint8_t *)ptr, len, TX_TIMEOUT_MS) != HAL_OK)
  {
    Error_Handler();
  }

  return len;
}

//interruption code
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_PIN){

	if(GPIO_PIN == ZC_Pin){
				interruptflag=1;
	}

}




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
