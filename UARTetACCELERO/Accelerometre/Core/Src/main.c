/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
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
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define MPU6050_ADDR 0x68


/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

uint8_t data_buffer[6];
int scale_factor;
volatile int flag;
float accel_x;
float accel_y;
float accel_z;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

void Check_who_am_i(){
	uint8_t who_am_i=0;
	HAL_StatusTypeDef res;
	uint8_t WHO_AM_I_ADRESS=0x75;
	
	res=HAL_I2C_Mem_Read(&hi2c1,MPU6050_ADDR<<1,WHO_AM_I_ADRESS,I2C_MEMADD_SIZE_8BIT,&who_am_i,1,HAL_MAX_DELAY);
	
	if (res==HAL_OK){
		if (who_am_i==0x68){
			printf("MPU-6050 detecte ok \r\n");
		}
		
		else{
			printf("Erreur: MPU-6050 detecte, mauvaise valeur lue : %i \r\n",who_am_i);
		}
	}
	
	else{
		printf("Erreur: Impossible de lire le registre WHO_AM_I\r\n ");
	
	}

}

void Configure_MPU6050() {
    HAL_StatusTypeDef ret;
	
		uint8_t pwr_mgmt_1_value = 0x80;  // DEVICE_RESET = 1
    ret = HAL_I2C_Mem_Write(&hi2c1, MPU6050_ADDR << 1, 0x6B, I2C_MEMADD_SIZE_8BIT, &pwr_mgmt_1_value, 1, HAL_MAX_DELAY);
    if (ret == HAL_OK) printf("reset ok PWR_MGMT, ret = %d\r\n", ret);
		
		HAL_Delay(100);  // Attendre le reset

    
    // PWR_MGMT_1 : Sélection de l'horloge interne
    pwr_mgmt_1_value = 0x08;
    ret = HAL_I2C_Mem_Write(&hi2c1, MPU6050_ADDR << 1, 0x6B, I2C_MEMADD_SIZE_8BIT, &pwr_mgmt_1_value, 1, HAL_MAX_DELAY);
    HAL_Delay(100);
		if (ret != HAL_OK) printf("Erreur de configuration PWR_MGMT_1, ret = %d\r\n", ret);
		
	
    // GYRO_CONFIG : FS_SEL = ±250 °/s, désactiver Selftest
    uint8_t gyro_config_value = 0b00000000;
    ret = HAL_I2C_Mem_Write(&hi2c1, MPU6050_ADDR << 1, 0x1B, I2C_MEMADD_SIZE_8BIT, &gyro_config_value, 1, HAL_MAX_DELAY);
    if (ret != HAL_OK) printf("Erreur de configuration GYRO_CONFIG\r\n");
	

    // ACCEL_CONFIG : AFS_SEL = ±2g, désactiver Selftest
    uint8_t accel_config_value = 0b00000000;
		scale_factor=16384;
    ret = HAL_I2C_Mem_Write(&hi2c1, MPU6050_ADDR << 1, 0x1C, I2C_MEMADD_SIZE_8BIT, &accel_config_value, 1, HAL_MAX_DELAY);
    if (ret != HAL_OK) printf("Erreur de configuration ACCEL_CONFIG\r\n");

    // CONFIG : Bandwidth = 94 Hz
    uint8_t config_value = 0b00000010;
    ret = HAL_I2C_Mem_Write(&hi2c1, MPU6050_ADDR << 1, 0x1A, I2C_MEMADD_SIZE_8BIT, &config_value, 1, HAL_MAX_DELAY);
    if (ret != HAL_OK) printf("Erreur de configuration CONFIG\r\n");

    // INT_ENABLE : Activer l'interruption DATA_RDY_EN
    uint8_t int_enable_value = 0b00000001;
    ret = HAL_I2C_Mem_Write(&hi2c1, MPU6050_ADDR << 1, 0x38, I2C_MEMADD_SIZE_8BIT, &int_enable_value, 1, HAL_MAX_DELAY);
    if (ret != HAL_OK) printf("Erreur de configuration INT_ENABLE\r\n");

    printf("Configuration du MPU-6050 terminee.\r\n");
}


void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    if (GPIO_Pin == GPIO_PIN_11) {
        if (hi2c1.State == HAL_I2C_STATE_READY) {
            HAL_I2C_Mem_Read_DMA(&hi2c1, MPU6050_ADDR << 1, 0x3B, I2C_MEMADD_SIZE_8BIT, data_buffer, 6);
           
        }
    }
}


void HAL_I2C_MemRxCpltCallback(I2C_HandleTypeDef *hi2c) {
    if (hi2c->Instance == I2C1) {
				int16_t raw_accel_x = (data_buffer[0] << 8) | data_buffer[1];
        int16_t raw_accel_y = (data_buffer[2] << 8) | data_buffer[3];
        int16_t raw_accel_z = (data_buffer[4] << 8) | data_buffer[5];
			
				accel_x=(float)raw_accel_x/scale_factor;
				accel_y=(float)raw_accel_y/scale_factor;
				accel_z=(float)raw_accel_z/scale_factor;
        flag=1;
				//printf("data_buffer: %02X %02X %02X %02X %02X %02X\r\n", data_buffer[0], data_buffer[1], data_buffer[2], data_buffer[3], data_buffer[4], data_buffer[5]);
				
				
    }
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
	HAL_Delay(100);
  MX_I2C1_Init();
	
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */
	
	
	Check_who_am_i();
	Configure_MPU6050();
	
	HAL_StatusTypeDef status = HAL_I2C_IsDeviceReady(&hi2c1, MPU6050_ADDR<<1, 1, 100);
				HAL_Delay(1000);
				if (status == HAL_OK) {
						printf("Device is ready.\n");
				} else if (status == HAL_ERROR) {
						printf("Error: Device is not ready.\n");
				} else if (status == HAL_BUSY) {
						printf("Device is busy.\n");
				} else if (status == HAL_TIMEOUT) {
						printf("Operation timed out.\n");
				} else {
						printf("Unknown status.\n");
				}
	
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */
		printf("data_buffer: %.2f  %.2f  %.2f \r\n",accel_x, accel_y,accel_z); 
		HAL_Delay(250);
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
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */



/* USER CODE BEGIN 4 */
int fputc(int ch, FILE *f)
{ 
 /* Place your implementation of fputc here */ 
 /* e.g. write a character to the USART2 and Loop until the end of transmission */ 
	HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, 0xFFFF); 
	return ch; 
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
