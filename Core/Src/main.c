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


/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
//-----UART-----//
#define UART_BUFFER_SIZE 255
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
uint8_t rx_byte; 
uint8_t rx_data_size=0;
uint8_t buffer[UART_BUFFER_SIZE];
uint16_t buffer_index = 0;

volatile static uint8_t rx_data[UART_BUFFER_SIZE];
volatile static uint8_t rx_size;
volatile static int reception_complete;

volatile static int transmission_complete=1;
uint8_t static tx_buffer[UART_BUFFER_SIZE];
uint8_t static size_tx;


             			
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
			if (flag-5000==0){
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















void print_data(uint8_t *table, int size){

	printf("{");

	for (int i=0; i<size; i++){
		printf("%d, ",table[i]);
	}
	
	printf("}\r\n");
}

void copy_array(volatile uint8_t *dest, const uint8_t *src, uint8_t size) {
  
    for (uint8_t i = 0; i < size; i++) {
        dest[i] = src[i];
    }
}



void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    if (huart->Instance == UART5) { // Vérifiez que l'interruption provient d'UART5
        switch (uart_state) {
						
            case STATE_IDLE:
                if (rx_byte == 0xFF) { // Debut d'une nouvelle transmission
                    uart_state = STATE_NEW_TRANSMISSION;
                    buffer_index = 0; // Reinitialise le buffer
                }
                break;

            case STATE_NEW_TRANSMISSION:
                if (rx_byte <= UART_BUFFER_SIZE) {
                    uart_state = STATE_RECEIVE; // Passe de l'etat de reception continue
										rx_data_size=rx_byte;
                } else {
                    uart_state = STATE_IDLE; // le buffer ne peut pas stocker tout l'info
                }
                break;

            case STATE_RECEIVE:
                if (buffer_index < rx_data_size-1) {
                    buffer[buffer_index++] = rx_byte;
                } else {
										buffer[buffer_index++] = rx_byte;
                    uart_state = STATE_IDLE; // toutes les donnees recues
										copy_array(rx_data,buffer,rx_data_size);
										rx_size=rx_data_size;
										
										reception_complete = 1;
                }
								
                break;
        }

        
    }
		
		// Relancer la reception pour le prochain byte
    HAL_UART_Receive_IT(&huart5, &rx_byte, 1);
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
    if (huart->Instance == UART5) {
        transmission_complete = 1; // Reinitialise le flag pour autoriser un nouvel envoi
    }
}


void send_data(uint8_t *data, uint8_t size) {
    // V�rifiez que la taille des donn�es est valide
    if (size + 2 > UART_BUFFER_SIZE) {
        return;
    }
		
		if (!transmission_complete){
			return;
		}

    // Pr�parer le buffer
    tx_buffer[0] = 0xFF;  // Bit de debut
    tx_buffer[1] = size;  // Taille des donn�es
    for (uint8_t i = 0; i < size; i++) {
        tx_buffer[i + 2] = data[i]; // Copiez les donn�es dans le buffer
				//printf("tx_buffer:%d",data[i]);
    }
		
    // Envoyer les donn�es avec DMA
    HAL_UART_Transmit_DMA(&huart5, tx_buffer, size + 2);
}
void send_position(player_t* player){
		
			uint8_t data[4]; 

			
			data[0] = (uint8_t)(player->current_pos.x & 0xFF);       
			data[1] = (uint8_t)((player->current_pos.x >> 8) & 0xFF); 

			
			data[2] = (uint8_t)(player->current_pos.y & 0xFF);       
			data[3] = (uint8_t)((player->current_pos.y >> 8) & 0xFF);
	
		
			
			//print_data(data,sizeof(data));
			send_data(data, 4);
			
}
void get_enemy_position(const volatile uint8_t *data, player_t* enemy, int *status) {
    if (data == NULL || enemy == NULL ) {
		*status=0;
        return; 
    }
		
		//enemy->previous_pos=enemy->current_pos;
		
    
    enemy->current_pos.x = (int16_t)(data[0] | (data[1] << 8));

    enemy->current_pos.y = (int16_t)(data[2] | (data[3] << 8));
		//printf("get_enemy_position");
		


	*status=1;
}

int receive_position(player_t* enemy){
	if(reception_complete){
		reception_complete=0;
		if (rx_size==4){
			int status=0;
			get_enemy_position(rx_data,enemy, &status);
			return status;
		}
	}
		
		return 0;

}



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

  
	HAL_UART_Receive_IT(&huart5, &rx_byte, 1);
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
	 player_t* local_player = &players[LOCAL_PLAYER_ID];
   player_t* enemy_player = &players[ENEMY_PLAYER_ID];
    /* Initialize test positions */
   
	
	
	/*Set initial positions 
   player->current_pos.x = 10; // Example initial position
   player->current_pos.y = 20;*/
		
	/* Infinite loop */
	uint8_t test_array[4] = {22, 34, 55, 11};
  while (1)
  {
		send_data(test_array, sizeof(test_array));
		print_data(rx_data,rx_data_size);
		HAL_Delay(50);
		
	
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
		//HAL_Delay(20); // � remplacer avec un timer
		
		

		
			
		
		if (flag20==100){
			
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
				
				send_position(player);	
				
				if (receive_position(enemy)==1){
					drawRemotePlayer(_screen,enemy);
				
				}
				
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
