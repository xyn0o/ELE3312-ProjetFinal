/* USER CODE BEGIN Header */
/** 
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program for the ELE3312 project
	* Authors         :
	* Group           :
	*
  ******************************************************************************
	*
  * 
  * This file contains the main logic of the game.
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
#include "ili9341.h"
#include "ili9341_gfx.h"
#include "stdio.h"
#include "stdlib.h"
#include "math.h"

#include "Graphics.h"
#include "Menu.h"
#include "Game.h"
#include "Battle.h"
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

typedef enum {
    POSITION = 1,
    GAME_STATE = 2
} ContentType;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
//-----UART-----//
#define UART_BUFFER_SIZE 32
#define START_BYTE 0xFF
//----ACCELERO-----//
#define MPU6050_ADDR 0x68
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
ili9341_t *_screen;
player_t players[NUM_PLAYERS] = {0};


UART_State uart_state = STATE_IDLE;
uint8_t dma_tx_buffer[UART_BUFFER_SIZE];      	  // buffer for DMA transmision
uint8_t uart_buffer[UART_BUFFER_SIZE]; 	// buffer for RX
uint8_t received_byte = 0;                   			// temporary storage for received byte
uint16_t buffer_index = 0;                   			// current position in RX buffer

//--ACCELERO--//
uint8_t data_buffer[6];
int scale_factor=16384;
volatile int flag=0;
volatile float accel_x = 0.0f;
volatile float accel_y = 0.0f;
volatile float accel_z = 0.0f;
volatile int flag20=0;

volatile float filtered_accel_x = 0.0f;
volatile float filtered_accel_y = 0.0f;
const float alpha = 0.3f; 
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
// Game.c or main.c
void Game_Update(void);
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
    pwr_mgmt_1_value = 0x00;
    ret = HAL_I2C_Mem_Write(&hi2c1, MPU6050_ADDR << 1, 0x6B, I2C_MEMADD_SIZE_8BIT, &pwr_mgmt_1_value, 1, HAL_MAX_DELAY);
		if (ret != HAL_OK) printf("Erreur de configuration PWR_MGMT_1, ret = %d\r\n", ret);
		
	
    // GYRO_CONFIG : FS_SEL = ±250 °/s, désactiver Selftest
    uint8_t gyro_config_value = 0b00000000;
    ret = HAL_I2C_Mem_Write(&hi2c1, MPU6050_ADDR << 1, 0x1B, I2C_MEMADD_SIZE_8BIT, &gyro_config_value, 1, HAL_MAX_DELAY);
    if (ret != HAL_OK) printf("Erreur de configuration GYRO_CONFIG\r\n");
	

    // ACCEL_CONFIG : AFS_SEL = ±2g, désactiver Selftest
    uint8_t accel_config_value = 0b00000000;
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
				flag+=1;
			if (flag-30==0){
				flag=0;
					if (hi2c1.State == HAL_I2C_STATE_READY) {
							if (HAL_I2C_Mem_Read_DMA(&hi2c1, MPU6050_ADDR << 1, 0x3B, I2C_MEMADD_SIZE_8BIT, data_buffer, 6) != HAL_OK) {
									printf("I2C read error!\r\n");
							}
        }
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
				//printf("x,y,z: %.2f  %.2f  %.2f \r\n",accel_x, accel_y,accel_z); 
			}
				
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
			
				printf("RX interrupt triggered"); // Received: %c\n", received_byte);
        UART_Receive_Handler(received_byte);
        HAL_UART_Receive_IT(&huart5, &received_byte, 1); // Restart interrupt
    }
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
    if (huart->Instance == UART5) {
        //printf("DMA TX Complete\n");
    }
}

void Send_Message(const uint8_t *message, uint16_t size, uint16_t content_type) {
		if (size + 4 > UART_BUFFER_SIZE) {
    printf("Error: Message too large\n");
    return;
		}
    
    uint8_t full_message[UART_BUFFER_SIZE];
    full_message[0] = START_BYTE;       // Start byte
		full_message[1] = content_type;    // Content type (position)
		memcpy(&full_message[2], message, size);
		full_message[2 + size] = '\n';
		HAL_UART_Transmit_DMA(&huart5, full_message, size + 4);
		printf("full_message (decimal): ");
    for (uint16_t i = 0; i < size + 3; i++) { // size + 3 = START_BYTE + CONTENT_TYPE + PAYLOAD + '\n'
        printf("%d ", full_message[i]);
    }
    printf("\n");
		
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
	HAL_Delay(500);
  /* USER CODE BEGIN Init */
	/* Default players setting */
	volatile  game_state_t game_state = CHOOSE_PLAYER;
  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_SPI1_Init();
  MX_USART2_UART_Init();
  MX_I2C1_Init();
  MX_UART5_Init();
  MX_TIM14_Init();
	
	HAL_TIM_Base_Start_IT(&htim14);
  /* USER CODE BEGIN 2 */
	Check_who_am_i();
	Configure_MPU6050();
	HAL_Delay(500);
	// Initialize the screen
	_screen = ili9341_new(
		  &hspi1,
		  Void_Display_Reset_GPIO_Port, Void_Display_Reset_Pin,
		  TFT_CS_GPIO_Port, TFT_CS_Pin,
		  TFT_DC_GPIO_Port, TFT_DC_Pin,
		  isoLandscapeFlip,
		  NULL, NULL,
		  NULL, NULL,
		  itsNotSupported,
		  itnNormalized);
	char text[40];
	ili9341_text_attr_t text_attr = {&ili9341_font_11x18,  ILI9341_WHITE, ILI9341_BLACK, 10, 0};
	ili9341_fill_screen(_screen, ILI9341_BLACK);
	
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	player_t* player = &players[LOCAL_PLAYER_ID];
	player_t* enemy = &players[ENEMY_PLAYER_ID];
	
	int16_t x = 0, y = 0;
		x = 258;
		y = 4;
		int16_t position_data[2] = {x, y};
	  Send_Message((uint8_t *)position_data, sizeof(position_data), POSITION);
	
		
	/* Infinite loop */
  while (1)
  {
    /* USER CODE END WHILE */
	
    /* USER CODE BEGIN 3 */
		//HAL_Delay(20); // � remplacer avec un timer

		
		
		
		if (flag20==10){
			
			switch(game_state) {
			case CHOOSE_PLAYER:
				choosePlayer(_screen, players);
				uint16_t message;
				game_state = INIT_MAZE;
				flag20=0;
				break;
			
			case INIT_MAZE:
				player->current_pos = player->start_pos;
				enemy->current_pos = enemy->start_pos;
			
			
				drawMaze(_screen, players);			
				game_state = WANDER_MAZE;
				flag20=0;
				break;
				
			case WANDER_MAZE:
				flag20=0;

				while(flag20==0);
				float accel_x_copy = accel_x;
				float accel_y_copy = accel_y;

				
				filtered_accel_x = alpha * accel_x_copy + (1 - alpha) * filtered_accel_x;
				filtered_accel_y = alpha * accel_y_copy + (1 - alpha) * filtered_accel_y;
				float threshold = 0.01f;    
				float sensitivity = 5.0f;  
				float max_speed = 3.0f;
			
				
				x = player->current_pos.x;
				y = player->current_pos.y;

				float delta_x = 0.0f;
				float delta_y = 0.0f;

			
				if (filtered_accel_x > threshold) {
						delta_x = (filtered_accel_x - threshold) * sensitivity;
				} else if (filtered_accel_x < -threshold) {
						delta_x = (filtered_accel_x + threshold) * sensitivity;
				}
				if (delta_x > max_speed) {
						delta_x = max_speed;
				} else if (delta_x < -max_speed) {
						delta_x = -max_speed;
				}
				x -= delta_x;
				if (filtered_accel_y > threshold) {
						delta_y = -(filtered_accel_y - threshold) * sensitivity; // Adjust sign based on coordinate system
				} else if (filtered_accel_y < -threshold) {
						delta_y = -(filtered_accel_y + threshold) * sensitivity;
				}
				if (delta_y > max_speed) {
						delta_y = max_speed;
				} else if (delta_y < -max_speed) {
						delta_y = -max_speed;
				}
				y -= delta_y;
				
				/*int16_t position_data[2] = {x, y};
				Send_Message((uint8_t *)position_data, sizeof(position_data), POSITION);
				printf("uart_buffer[0]: %d,", uart_buffer[0]);
				if (uart_buffer[0]==1){
					enemy->current_pos.x=(int16_t)(uart_buffer[1] << 8 | uart_buffer[2]);
					enemy->current_pos.y=(int16_t)(uart_buffer[3] << 8 | uart_buffer[4]);
					printf("Received Position: x = %d, y = %d\n", x, y);}*/
				if(updatePosition(_screen, (position_t){x, y}, players)){
					game_state = BATTLE;
					flag20=0;
					continue;
				}
				flag20=0;
				break;
			case BATTLE:
				battle(_screen, players);
				game_state = INIT_MAZE;
				flag20=0;
				break;
		}
	}
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
int fputc(int ch, FILE *f)
{
  HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
  return ch;
}



void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM14)
    {
        flag20=1;
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
