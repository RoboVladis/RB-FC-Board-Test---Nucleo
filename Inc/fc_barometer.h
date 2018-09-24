#ifndef __FC_BAROMETER_H
#define __FC_BAROMETER_H

#include "stm32f4xx.h" 
#include "stm32f4xx_ll_dma.h"
#include "stm32f4xx_ll_spi.h"
#include "stm32f4xx_ll_gpio.h"

#include "fc_board.h"

// ---------------------------------------------------------------------------
// Defines
//----------------------------------------------------------------------------

// Define sensor numbers
#define BAR0    0
#define BAR1    1

// Define SPI  pins
#define BAR0_NSS        GPIOC, LL_GPIO_PIN_0
#define BAR1_NSS        GPIOB, LL_GPIO_PIN_12

#define BAR0_MISO       GPIOC, LL_GPIO_PIN_2
#define BAR1_MISO       GPIOB, LL_GPIO_PIN_14   

#define BAR0_M0SI       GPIOC, LL_GPIO_PIN_1
#define BAR1_MOSI       GPIOB, LL_GPIO_PIN_15 

#define BAR0_SCK        GPIOB, LL_GPIO_PIN_10
#define BAR1_SCK        GPIOB, LL_GPIO_PIN_13

// Define peripherals:
#define BAR_SPI         SPI2
#define BAR_IRQHandler  DMA1_Stream3_IRQHandler

#define BAR_TX_DMA             DMA1                             
#define BAR_TX_STREAM          LL_DMA_STREAM_4
#define BAR_TX_DMA_STREAM      DMA1_Stream4
#define BAR_TX_CHANNEL         LL_DMA_CHANNEL_0

#define BAR_RX_DMA             DMA1                             
#define BAR_RX_STREAM          LL_DMA_STREAM_3
#define BAR_RX_DMA_STREAM      DMA1_Stream3
#define BAR_RX_CHANNEL         LL_DMA_CHANNEL_0

#define BAR_LENGTH_OF_BUFFER            8
#define BAR_NUMBER_OF_PROM_BUFFERS      8

// Defines number of smsamples in average
#define BAR_NUMBER_OF_PRESSURE_SAMPLES          1
#define BAR_NUMBER_OF_TEMPERATURE_SAMPLES       256


// Define functions (macros) to clear DMA "Transfer complete" and "Error" flags

#define BAR_CLEAR_DMA_FLAGS()     {\
    LL_DMA_ClearFlag_TE4(BAR_TX_DMA); \
    LL_DMA_ClearFlag_TC4(BAR_TX_DMA); \
    LL_DMA_ClearFlag_TE3(BAR_RX_DMA); \
    LL_DMA_ClearFlag_TC3(BAR_RX_DMA); \
    }

// ---------------------------------------------------------------------------
// Typedefs
//----------------------------------------------------------------------------

// This enumeration defines barometer states used to run state machine
typedef enum
{
  BAR_State_Error = 0,          // Error detected
  BAR_State_Uninitialized,      // Sensor after reset
  BAR_State_Initialization,     // First step of initialization
  BAR_State_ReadProm0,          // Read prom register 0
  BAR_State_ReadProm1,          // Read prom register 1
  BAR_State_ReadProm2,          // Read prom register 2
  BAR_State_ReadProm3,          // Read prom register 3
  BAR_State_ReadProm4,          // Read prom register 4
  BAR_State_ReadProm5,          // Read prom register 5
  BAR_State_ReadProm6,          // Read prom register 6
  BAR_State_ReadProm7,          // Read prom register 7
  BAR_State_CrcCheck,           // Check Crc
  BAR_State_Initialized,        // Initialization step 2
  BAR_State_ConvertPressure,
  BAR_State_ConvertTemperature,
  BAR_State_ReadPressure,
  BAR_State_ReadTemperature
}BAR_State_e;


// This enumerations defines comand names
typedef enum
{
  BAR_Cmd_ReadAdc = 0x00,       // Read ADC value
  BAR_Cmd_Reset = 0x1E,         // Reset device
  BAR_Cmd_ConvertD1_256 = 0x40,     // Convert pressure (OSR = 256)
  BAR_Cmd_ConvertD1_512 = 0x42,     // Convert pressure (OSR = 512)
  BAR_Cmd_ConvertD1_1024 = 0x44,     // Convert pressure (OSR = 1024)
  BAR_Cmd_ConvertD1_2048 = 0x46,     // Convert pressure (OSR = 2048)
  BAR_Cmd_ConvertD1_4096 = 0x48,     // Convert pressure (OSR = 4096)
  BAR_Cmd_ConvertD2_256 = 0x50,     // Convert temperature (OSR = 256)
  BAR_Cmd_ConvertD2_512 = 0x52,     // Convert temperature (OSR = 512)
  BAR_Cmd_ConvertD2_1024 = 0x54,     // Convert temperature (OSR = 1024)
  BAR_Cmd_ConvertD2_2048 = 0x56,     // Convert temperature (OSR = 2048)
  BAR_Cmd_ConvertD2_4096 = 0x58,     // Convert temperature (OSR = 4096)
  BAR_Cmd_ReadProm0 = 0xA0,     // Read PROM register 0 
  BAR_Cmd_ReadProm1 = 0xA2,     // Read PROM register 1
  BAR_Cmd_ReadProm2 = 0xA4,     // Read PROM register 2
  BAR_Cmd_ReadProm3 = 0xA6,     // Read PROM register 3
  BAR_Cmd_ReadProm4 = 0xA8,     // Read PROM register 4
  BAR_Cmd_ReadProm5 = 0xAA,     // Read PROM register 5
  BAR_Cmd_ReadProm6 = 0xAC,     // Read PROM register 6
  BAR_Cmd_ReadProm7 = 0xAE,     // Read PROM register 7  
}BAR_Cmd_e;


typedef enum
{
  BAR_ParamType_Pressure,      // Indicates that ADC value contains pressure value 
  BAR_ParamType_Temperature,    // Indicates that ADC value contains temperature value  
}BAR_ParamType_e;


// This structure stores measured values in floating point form
typedef struct _BAR_Values_t_
{
  float Pressure;               // Pressure in mbars
  float Temperature;            // Temperature in degrees Celsius
}BAR_Values_t;


// This structure contains all data associated with single barometric sensor
typedef struct _BAR_t_
{
  int           State;                                          // State in standartized reprezentation
  BAR_Values_t  Values;                                         // Measured values
  BAR_State_e   InternalState;                                  // State used in sensor's state machine
  uint16_t      PromCoefficients[BAR_NUMBER_OF_PROM_BUFFERS];   // Buffer
  uint32_t      RawTemperature;                                 // Raw temperature reading
  uint32_t      RawPressure;                                    // Raw pressure reading
  BAR_ParamType_e MeasuredParameter;                            // Indicates which parameter is measured
  uint32_t      PressureBuffer[BAR_NUMBER_OF_PRESSURE_SAMPLES];
  uint32_t      TemperatureBuffer[BAR_NUMBER_OF_TEMPERATURE_SAMPLES];
  uint32_t      TemperatureIndex;
  uint32_t      PressureIndex;
  uint32_t      WaitTime;
}BAR_t;

typedef struct _BAR_Common_t_
{
  uint32_t      SelectedDevice;                         // Indicates active device
  uint8_t       TxBuffer[BAR_LENGTH_OF_BUFFER];         // Buffer for transmitted data
  uint8_t       RxBuffer[BAR_LENGTH_OF_BUFFER];         // Buffer for received data
  uint32_t      Length;                                 // Number of bytes to transmit/receive
}BAR_Common_t;

// ---------------------------------------------------------------------------
// External Variables
//----------------------------------------------------------------------------

extern BAR_t   Bar0;
extern BAR_t   Bar1;
extern BAR_Common_t BarCommon;

// ---------------------------------------------------------------------------
// External Functions
//----------------------------------------------------------------------------

#ifdef __cplusplus
extern "C" {
#endif
  
  // Calculates pressure and temperature from raw readings
  extern void BAR_CalculateOutput(int device);
  
  // Configures DMA and SPI to proper communication with pressure sensor
  extern void BAR_Init(void);
  
  // Runs state machine
  extern void BAR_Run(void);
  
  // Runs device selecte
  extern void BAR_RunSelectedDevice();
  
  // Configures output ports to communicate with selected device
  extern void BAR_SelectDevice();
  
  // Sends selected command
  extern void BAR_SendComand(BAR_Cmd_e command);
  
  // Sets default values to global sensor's structure 
  extern void BAR_SetDefaultValues(BAR_t *bar);
  
  // Prepares output ports to communicate with selected device
  extern void BAR_StartTransfer(void);
  
  extern void BAR_SetDefaultCommonValues(BAR_Common_t *common);
  
  
#ifdef __cplusplus
}
#endif

#endif // __FC_BAROMETER_H