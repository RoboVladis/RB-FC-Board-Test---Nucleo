// Define to prevent recursive inclusion
#include <string.h>
#include <stdio.h> 
#include "stm32f4xx.h" 
#include "stm32f4xx_ll_dma.h"
#include "stm32f4xx_ll_spi.h"
#include "stm32f4xx_ll_gpio.h"

#include "fc_board.h"


#ifndef __FC_GYROACC_H
#define __FC_GYROACC_H

#define ACC0    0
#define ACC1    1

#define ACC0_NSS                GPIOD, LL_GPIO_PIN_5
#define ACC1_NSS                GPIOA, LL_GPIO_PIN_4
//#define ACC0_NSS                GPIOA, LL_GPIO_PIN_4

#define ACC0_SPI                SPI3

#define ACCO_IRQHandler         DMA1_Stream0_IRQHandler

#define ACC0_TX_DMA             DMA1                             
#define ACC0_TX_STREAM          LL_DMA_STREAM_7
#define ACC0_TX_DMA_STREAM      DMA1_Stream7
#define ACC0_TX_CHANNEL         LL_DMA_CHANNEL_0

#define ACC0_RX_DMA             DMA1                             
#define ACC0_RX_STREAM          LL_DMA_STREAM_0
#define ACC0_RX_DMA_STREAM      DMA1_Stream0
#define ACC0_RX_CHANNEL         LL_DMA_CHANNEL_0


#define ACC1_SPI                SPI1

#define ACC1_IRQHandler         DMA2_Stream2_IRQHandler

#define ACC1_TX_DMA             DMA2                             
#define ACC1_TX_STREAM          LL_DMA_STREAM_3
#define ACC1_TX_DMA_STREAM      DMA2_Stream3
#define ACC1_TX_CHANNEL         LL_DMA_CHANNEL_3

#define ACC1_RX_DMA             DMA2                             
#define ACC1_RX_STREAM          LL_DMA_STREAM_2
#define ACC1_RX_DMA_STREAM      DMA2_Stream2
#define ACC1_RX_CHANNEL         LL_DMA_CHANNEL_3

#define ACC_LENGTH_OF_BUFFER   128

#define ACC0_CLEAR_DMA_FLAGS()     {\
    LL_DMA_ClearFlag_TE7(ACC0_TX_DMA); \
    LL_DMA_ClearFlag_TC7(ACC0_TX_DMA); \
    LL_DMA_ClearFlag_TE0(ACC0_RX_DMA); \
    LL_DMA_ClearFlag_TC0(ACC0_RX_DMA); \
    }

#define ACC1_CLEAR_DMA_FLAGS()     {\
    LL_DMA_ClearFlag_TE3(ACC1_TX_DMA); \
    LL_DMA_ClearFlag_TC3(ACC1_TX_DMA); \
    LL_DMA_ClearFlag_TE2(ACC1_RX_DMA); \
    LL_DMA_ClearFlag_TC2(ACC1_RX_DMA); \
    }

typedef enum
{
  ACC_State_Error = 0,          // Error detected
  ACC_State_Uninitialized,      // Sensor after reset
  ACC_State_Initialization1,    // Initialization step 1
  ACC_State_Initialization2,    // Initialization step 2
  ACC_State_Initialization3,    // Initialization step 3
  ACC_State_Initialized,        // Sensor ready for normal operation
}ACC_State_e;

typedef struct _ACC_Regs_t_
{
  uint8_t reserved_0[4];      
  uint8_t xg_offs_tc_h;       // 0x04
  uint8_t xg_offs_tc_l;       // 0x05
  uint8_t reserved_1;
  uint8_t yg_offs_tc_h;       // 0x07
  uint8_t yg_offs_tc_l;       // 0x08
  uint8_t reserved_2;
  uint8_t zg_offs_tc_h;       // 0x0A
  uint8_t zg_offs_tc_l;       // 0x0B 
  uint8_t reserved_3;
  uint8_t self_test_x_accel;  // 0x0D 
  uint8_t self_test_y_accel;  // 0x0E 
  uint8_t self_test_z_accel;  // 0x0F 
  uint8_t reserved_4[3];
  uint8_t xg_offs_usrh;       // 0x13
  uint8_t xg_offs_usrl;       // 0x14
  uint8_t yg_offs_usrh;       // 0x15
  uint8_t yg_offs_usrl;       // 0x16
  uint8_t zg_offs_usrh;       // 0x17
  uint8_t zg_offs_usrl;       // 0x18
  uint8_t smplrt_div;         // 0x19
  uint8_t config;             // 0x1A
  uint8_t gyro_config;        // 0x1B
  uint8_t accel_config;       // 0x1C
  uint8_t accel_config_2;     // 0x1D
  uint8_t lp_mode_cfg;        // 0x1E
  uint8_t reserved_5;         
  uint8_t accel_wom_x_thr;    // 0x20
  uint8_t accel_wom_y_thr;    // 0x21
  uint8_t accel_wom_z_thr;    // 0x22
  uint8_t fifo_en;            // 0x23
  uint8_t reserved_6[18];     
  uint8_t fsync_int;          // 0x36          
  uint8_t int_pin_cfg;        // 0x37  
  uint8_t int_enable;         // 0x38
  uint8_t fifo_wm_int_status; // 0x39  
  uint8_t int_status;         // 0x3A  
  uint8_t accel_xout_h;       // 0x3B  
  uint8_t accel_xout_l;       // 0x3C  
  uint8_t accel_yout_h;       // 0x3D  
  uint8_t accel_yout_l;       // 0x3E 
  uint8_t accel_zout_h;       // 0x3F  
  uint8_t accel_zout_l;       // 0x40  
  uint8_t temp_out_h;         // 0x41
  uint8_t temp_out_l;         // 0x42
  uint8_t gyro_xout_h;        // 0x43
  uint8_t gyro_xout_l;        // 0x44
  uint8_t gyro_yout_h;        // 0x45
  uint8_t gyro_yout_l;        // 0x46
  uint8_t gyro_zout_h;        // 0x47
  uint8_t gyro_zout_l;        // 0x48
  uint8_t reserved_7[7];
  uint8_t self_test_x_gyro;   // 0x50
  uint8_t self_test_y_gyro;   // 0x51
  uint8_t self_test_z_gyro;   // 0x52
  uint8_t reserved_8[13];
  uint8_t fifo_wm_th1;        // 0x60
  uint8_t fifo_wm_th2;        // 0x61
  uint8_t reserved_9[6];      
  uint8_t signal_path_reset;  // 0x68
  uint8_t accel_intel_ctrl;   // 0x69
  uint8_t user_ctrl;          // 0x6A
  uint8_t pwr_mgmt_1;         // 0x6B
  uint8_t pwr_mgmt_2;         // 0x6C
  uint8_t reserved_10[3];
  uint8_t i2c_if;             // 0x70  
  uint8_t reserved_11;
  uint8_t fifo_counth;        // 0x72 
  uint8_t fifo_countl;        // 0x73 
  uint8_t fifo_r_w;           // 0x74
  uint8_t who_am_i;           // 0x75
  uint8_t reserved_12;
  uint8_t xa_offset_h;        // 0x77
  uint8_t xa_offset_l;        // 0x78 
  uint8_t reserved_13;
  uint8_t ya_offset_h;        // 0x7A 
  uint8_t ya_offset_l;        // 0x7B 
  uint8_t reserved_14;
  uint8_t za_offset_h;        // 0x7D 
  uint8_t za_offset_l;        // 0x7E 
}ACC_Regs_t; 

typedef struct _ACC_t_
{
  ACC_State_e state;            // State of sensor
  uint8_t tx_buffer[ACC_LENGTH_OF_BUFFER];       // Transmitted data buffer
  uint8_t rx_buffer[ACC_LENGTH_OF_BUFFER];       // Received data buffer
  uint8_t length;               // LNumber of bytes to transmit/receive
  ACC_Regs_t regs;              // Register of 
}ACC_t;



extern ACC_t Acc0;
extern ACC_t Acc1;


#ifdef __cplusplus
extern "C" {
#endif
  
  //! \brief      Configures DMA and SPI to proper communication with motion sensor  
  extern void ACC_Init(int device);
  
  //! \brief      Sets default values to global motion sensor structure 
  extern void ACC_SetDefaultValues(ACC_t *acc);
  
  //! \brief      "Transfer Complete" interrupt handler of Rx DMA Stream
  extern void ACCO_IRQHandler(void);
  
  extern void Acc_ReadData(int device);
  
  extern void Acc_ReadConfig(int device);
  
  extern void Acc_WriteConfig1(int device);
    
  extern void Acc_WriteConfig2(int device);
  
  extern void ACC0_StartTransfer(void);
  
  extern void ACC1_StartTransfer(void);
  
  extern void ACC_Run(int device);
 

#ifdef __cplusplus
}
#endif

#endif // __FC_GYROACC_H