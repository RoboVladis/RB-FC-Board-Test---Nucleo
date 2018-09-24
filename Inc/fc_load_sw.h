// Define to prevent recursive inclusion
#include <string.h>
#include <stdio.h> 
#include "stm32f4xx.h" 
#include "stm32f4xx_ll_dma.h"
#include "stm32f4xx_ll_i2c.h"
#include "stm32f4xx_ll_gpio.h"

#ifndef __FC_LOAD_SW_H
#define __FC_LOAD_SW_H

#define LOADSW_I2C              I2C3
#define LOADSW_ER_IRQHandler    I2C3_ER_IRQHandler
#define LOADSW_EV_IRQHandler    I2C3_EV_IRQHandler

#define SW0             0
#define SW1             1

#define SW0_ADDR        0xE0
#define SW1_ADDR        0xE2

// Read/Write only config (1-4) and control registers
// Mode registers are not used
#define LOADSW_NUMBER_OF_REGS   6      

#define LOADSW_CH1      0x01
#define LOADSW_CH2      0x02
#define LOADSW_CH3      0x04
#define LOADSW_CH4      0x08

#define PWR_CTRL0   GPIOG, LL_GPIO_PIN_3
#define PWR_CTRL1   GPIOG, LL_GPIO_PIN_2
#define PWR_CTRL2   GPIOG, LL_GPIO_PIN_6
#define PWR_CTRL3   GPIOG, LL_GPIO_PIN_5
#define PWR_CTRL4   GPIOD, LL_GPIO_PIN_13
#define PWR_CTRL5   GPIOD, LL_GPIO_PIN_14
#define PWR_CTRL6   GPIOD, LL_GPIO_PIN_15

#define LOADSW_ACC0     PWR_CTRL0
#define LOADSW_ACC1     PWR_CTRL4
#define LOADSW_MAG0     PWR_CTRL2
#define LOADSW_MAG1     PWR_CTRL5
#define LOADSW_BAR0     PWR_CTRL1
#define LOADSW_BAR1     PWR_CTRL6
#define LOADSW_FRAM     PWR_CTRL3


// Following structure defines internal registers of Load Switch TPS22994
// These registers are accessible through I2C interface
typedef struct _LOADASW_Regs_t_
{
  uint8_t reserved_1;   // 0x00
  uint8_t config_1;     // 0x01
  uint8_t config_2;     // 0x02
  uint8_t config_3;     // 0x03
  uint8_t config_4;     // 0x04
  uint8_t control;      // 0x05
  uint8_t mode_1;       // 0x06
  uint8_t mode_2;       // 0x07
  uint8_t mode_3;       // 0x08
  uint8_t mode_4;       // 0x09
  uint8_t mode_5;       // 0x0A
  uint8_t mode_6;       // 0x0B
  uint8_t mode_7;       // 0x0C
  uint8_t mode_8;       // 0x0D
  uint8_t mode_9;       // 0x0E
  uint8_t mode_10;      // 0x0F
  uint8_t mode_11;      // 0x10
  uint8_t mode_12;      // 0x11  
}LOADSW_Regs_t;


// This structure defines flags required for
typedef struct _LOADSW_t_
{
  int read;             // Denotes read/write state of curren transfer
  int device;           // Denotes which device is communicated
  int buffer[2][18];        // Denotes buffer
  int index;            // Denotes index in the buffer
  int regAddr;          // Denoter register addres
  LOADSW_Regs_t regs[2];   // Denotes internal registers of the load switch
}LOADSW_t;



#ifdef __cplusplus
extern "C" {
#endif
 
extern void LOADSW_Init(void);
  
extern void LOADSW_ReadRegs(int device);

extern void LOADSW_StartTransmission(void);

extern void LOADSW_WriteRegs(int device);

void LOADSW_EV_IRQHandler(void);
  
#ifdef __cplusplus
}
#endif

#endif // __FC_LOAD_SW_H