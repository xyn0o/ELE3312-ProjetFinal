// Accelerometer.c

#include "Accelerometer.h"

#define MPU6050_ADDR 0x68

// Define variables
static volatile float accel_x = 0.0f;
static volatile float accel_y = 0.0f;
static volatile float accel_z = 0.0f;

static uint8_t data_buffer[6];
static int scale_factor;

// Function to check WHO_AM_I register
void Check_who_am_i() {
    uint8_t who_am_i = 0;
    HAL_StatusTypeDef res;
    uint8_t WHO_AM_I_ADDRESS = 0x75;

    res = HAL_I2C_Mem_Read(&hi2c1, MPU6050_ADDR << 1, WHO_AM_I_ADDRESS, I2C_MEMADD_SIZE_8BIT, &who_am_i, 1, HAL_MAX_DELAY);

    if (res == HAL_OK) {
        if (who_am_i == 0x68) {
            printf("MPU-6050 detected OK\r\n");
        } else {
            printf("Error: MPU-6050 detected, wrong value read: %i\r\n", who_am_i);
        }
    } else {
        printf("Error: Unable to read WHO_AM_I register\r\n");
    }
}

// Function to configure the MPU-6050
void Configure_MPU6050() {
    HAL_StatusTypeDef ret;

    uint8_t pwr_mgmt_1_value = 0x80;  // DEVICE_RESET = 1
    ret = HAL_I2C_Mem_Write(&hi2c1, MPU6050_ADDR << 1, 0x6B, I2C_MEMADD_SIZE_8BIT, &pwr_mgmt_1_value, 1, HAL_MAX_DELAY);
    if (ret == HAL_OK) printf("Reset OK PWR_MGMT, ret = %d\r\n", ret);

    HAL_Delay(100);  // Wait for reset

    // PWR_MGMT_1: Select internal clock
    pwr_mgmt_1_value = 0x08;
    ret = HAL_I2C_Mem_Write(&hi2c1, MPU6050_ADDR << 1, 0x6B, I2C_MEMADD_SIZE_8BIT, &pwr_mgmt_1_value, 1, HAL_MAX_DELAY);
    HAL_Delay(100);
    if (ret != HAL_OK) printf("Error configuring PWR_MGMT_1, ret = %d\r\n", ret);

    // GYRO_CONFIG: FS_SEL = ±250 °/s, disable self-test
    uint8_t gyro_config_value = 0b00000000;
    ret = HAL_I2C_Mem_Write(&hi2c1, MPU6050_ADDR << 1, 0x1B, I2C_MEMADD_SIZE_8BIT, &gyro_config_value, 1, HAL_MAX_DELAY);
    if (ret != HAL_OK) printf("Error configuring GYRO_CONFIG\r\n");

    // ACCEL_CONFIG: AFS_SEL = ±2g, disable self-test
    uint8_t accel_config_value = 0b00000000;
    scale_factor = 16384;
    ret = HAL_I2C_Mem_Write(&hi2c1, MPU6050_ADDR << 1, 0x1C, I2C_MEMADD_SIZE_8BIT, &accel_config_value, 1, HAL_MAX_DELAY);
    if (ret != HAL_OK) printf("Error configuring ACCEL_CONFIG\r\n");

    // CONFIG: Bandwidth = 94 Hz
    uint8_t config_value = 0b00000010;
    ret = HAL_I2C_Mem_Write(&hi2c1, MPU6050_ADDR << 1, 0x1A, I2C_MEMADD_SIZE_8BIT, &config_value, 1, HAL_MAX_DELAY);
    if (ret != HAL_OK) printf("Error configuring CONFIG\r\n");

    // INT_ENABLE: Enable DATA_RDY_EN interrupt
    uint8_t int_enable_value = 0b00000001;
    ret = HAL_I2C_Mem_Write(&hi2c1, MPU6050_ADDR << 1, 0x38, I2C_MEMADD_SIZE_8BIT, &int_enable_value, 1, HAL_MAX_DELAY);
    if (ret != HAL_OK) printf("Error configuring INT_ENABLE\r\n");

    printf("MPU-6050 configuration complete.\r\n");
}

// Initialization function
void Accelerometer_Init(void) {
    Check_who_am_i();
    Configure_MPU6050();
}

// Getter functions
float Accelerometer_GetX(void) {
    return accel_x;
}

float Accelerometer_GetY(void) {
    return accel_y;
}

float Accelerometer_GetZ(void) {
    return accel_z;
}

// Interrupt callback functions
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

        accel_x = (float)raw_accel_x / scale_factor;
        accel_y = (float)raw_accel_y / scale_factor;
        accel_z = (float)raw_accel_z / scale_factor;

        // Optional: Print or process the data
        // printf("Accel X: %.2f, Y: %.2f, Z: %.2f\r\n", accel_x, accel_y, accel_z);
    }
}
