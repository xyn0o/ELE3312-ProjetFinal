
#ifndef ACCELEROMETER_H
#define ACCELEROMETER_H

#include "stm32f4xx_hal.h" 
#include <stdint.h>

// Function prototypes
void Accelerometer_Init(void);
float Accelerometer_GetX(void);
float Accelerometer_GetY(void);
float Accelerometer_GetZ(void);

extern I2C_HandleTypeDef hi2c1;

#endif // ACCELEROMETER_H
