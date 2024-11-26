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
#include "string.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef enum {
    STATE_IDLE,
    STATE_NEW_TRANSMISSION,
    STATE_RECEIVE
} UART_State;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define UART_BUFFER_SIZE 256
#define START_BYTE 0xFF
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
UART_State uart_state = STATE_IDLE;
uint8_t dma_tx_buffer[UART_BUFFER_SIZE];     // DMA transmision buffer
uint8_t uart_buffer[UART_BUFFER_SIZE];       // RX message buffer
uint8_t received_byte = 0;                   // Temporary storage for received byte
uint16_t buffer_index = 0;                   // Tracks current position in RX buffer
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void Handle_Received_Message(uint8_t *buffer, uint16_t size);
void Send_Message(const uint8_t *message, uint16_t size);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */


int fputc(int ch, FILE *f) {
    HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
    return ch;
}

void Handle_Received_Message(uint8_t *buffer, uint16_t size) {
    // Echo back received message via UART5
    //HAL_UART_Transmit(&huart5, buffer, size, HAL_MAX_DELAY);
    printf("Received Message: ");
    for (uint16_t i = 0; i < size; i++) {
        printf("%c", buffer[i]);
    }
    printf("\n");
}

void UART_Receive_Handler(uint8_t byte) {
    switch (uart_state) {
        case STATE_IDLE:
            if (byte == START_BYTE) {
                uart_state = STATE_NEW_TRANSMISSION;
                buffer_index = 0;
            }
            break;

        case STATE_NEW_TRANSMISSION:
            if (buffer_index < UART_BUFFER_SIZE) {
                uart_buffer[buffer_index++] = byte;
                uart_state = STATE_RECEIVE;
            } else {
                uart_state = STATE_IDLE; // Buffer overflow
            }
            break;

        case STATE_RECEIVE:
            if (buffer_index < UART_BUFFER_SIZE) {
                uart_buffer[buffer_index++] = byte;
            } else {
                uart_state = STATE_IDLE; // Buffer overflow
            }
            if (byte == '\n') {
                Handle_Received_Message(uart_buffer, buffer_index);
                uart_state = STATE_IDLE;
            }
            break;

        default:
            uart_state = STATE_IDLE;
            break;
    }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    if (huart->Instance == UART5) {
				//printf("RX interrupt triggered. Received: %c\n", received_byte);
        UART_Receive_Handler(received_byte);
        HAL_UART_Receive_IT(&huart5, &received_byte, 1); // Restart interrupt
    }
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
    if (huart->Instance == UART5) {
        //printf("DMA TX Complete\n");
    }
}

void Send_Message(const uint8_t *message, uint16_t size) {
    if (size + 3 > UART_BUFFER_SIZE) {
        printf("Error: Message too large\n");
        return;
    }

    uint8_t full_message[UART_BUFFER_SIZE];
    full_message[0] = START_BYTE;
    memcpy(&full_message[1], message, size);
    full_message[1 + size] = '\n';
		HAL_UART_Transmit_DMA(&huart5, full_message, size + 3);

    /*if (HAL_UART_Transmit_DMA(&huart5, full_message, size + 2) != HAL_OK) {
        printf("Error: DMA transmission failed.\n");
    }*/
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
  MX_I2C1_Init();
  MX_USART2_UART_Init();
  MX_UART5_Init();
  /* USER CODE BEGIN 2 */
    printf("Starting UART COMMUNICATION Example\n");
    HAL_UART_Receive_IT(&huart5, &received_byte, 1); // Start RX interrupt
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
    while (1) {
        //const uint8_t test_message[] = "1\r\n";
				//const uint8_t test_message[] = "je suis la carte de la prof voici mon message\r\n";
				const uint8_t test_message[] = "je suis la carte deux, je ne dois pas voir mon propre message\r\n";
        Send_Message(test_message, strlen((char *)test_message));
        HAL_Delay(2000); // Wait 1 second
				//printf("Message sent in main loop\n"); // Debugging log
    }
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
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
