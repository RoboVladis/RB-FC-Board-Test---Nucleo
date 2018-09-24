#include "fc_barometer.h"

BAR_Common_t BarCommon;
BAR_t Bar0;
BAR_t Bar1;
int BarReminder[2] = {0xAB, 0xAB};

volatile unsigned int BAR_Timer = 0;


void BAR_SelectDevice()
{
  if(BarCommon.SelectedDevice == BAR0)
  {    
    LL_GPIO_SetPinMode(BAR1_SCK, LL_GPIO_MODE_INPUT);
    LL_GPIO_SetPinMode(BAR1_NSS, LL_GPIO_MODE_INPUT);
    LL_GPIO_SetPinMode(BAR1_MISO, LL_GPIO_MODE_INPUT);
    LL_GPIO_SetPinMode(BAR1_MOSI, LL_GPIO_MODE_INPUT);
    
    LL_GPIO_SetPinMode(BAR0_SCK, LL_GPIO_MODE_ALTERNATE);
    LL_GPIO_SetPinMode(BAR0_NSS, LL_GPIO_MODE_OUTPUT);
    LL_GPIO_SetPinMode(BAR0_MISO, LL_GPIO_MODE_ALTERNATE);
    LL_GPIO_SetPinMode(BAR0_M0SI, LL_GPIO_MODE_ALTERNATE);
  }
  else
  {
    
    LL_GPIO_SetPinMode(BAR0_SCK, LL_GPIO_MODE_INPUT);
    LL_GPIO_SetPinMode(BAR0_NSS, LL_GPIO_MODE_INPUT);
    LL_GPIO_SetPinMode(BAR0_MISO, LL_GPIO_MODE_INPUT);
    LL_GPIO_SetPinMode(BAR0_M0SI, LL_GPIO_MODE_INPUT);
    
    LL_GPIO_SetPinMode(BAR1_SCK, LL_GPIO_MODE_ALTERNATE);
    LL_GPIO_SetPinMode(BAR1_NSS, LL_GPIO_MODE_OUTPUT);
    LL_GPIO_SetPinMode(BAR1_MISO, LL_GPIO_MODE_ALTERNATE);
    LL_GPIO_SetPinMode(BAR1_MOSI, LL_GPIO_MODE_ALTERNATE);
  }
}

void BAR_SetDefaultCommonValues(BAR_Common_t *common)
{
  common->Length = 2;
  common->SelectedDevice = BAR0;
  
  int i;
  uint8_t *ptr;
  
  i = 0;
  ptr = common->TxBuffer;
  while(i++ < BAR_LENGTH_OF_BUFFER) *ptr++ = 0; 
  
  i = 0;
  ptr = common->RxBuffer;
  while(i++ < BAR_LENGTH_OF_BUFFER) *ptr++ = 0;
}

void BAR_SetDefaultValues(BAR_t *bar)
{
  int i;
  int16_t *ptr16;
  uint32_t *ptr32;
  
  i = 0;
  ptr16 = (int16_t*)&bar->PromCoefficients;
  while(i++ < BAR_NUMBER_OF_PROM_BUFFERS) *ptr16++ = 0;
    
  i = 0;
  ptr32 = (uint32_t*)&bar->PressureBuffer;
  while(i++ < BAR_NUMBER_OF_PRESSURE_SAMPLES) *ptr32++ = 0;
  
  i = 0;
  ptr32 = (uint32_t*)&bar->TemperatureBuffer;
  while(i++ < BAR_NUMBER_OF_TEMPERATURE_SAMPLES) *ptr32++ = 0;
  
  bar->State = STATE_UNINITIALIZED;
  bar->Values.Temperature = 0;
  bar->Values.Pressure = 0;  
  bar->WaitTime = 0;
  bar->InternalState = BAR_State_Uninitialized;
  bar->RawTemperature = 0;
  bar->RawPressure = 0;
  bar->PressureIndex = 0;
  bar->TemperatureIndex = 0;
}

void BAR_Init(void)
{
  LL_DMA_InitTypeDef DMA_InitStruct;   
  LL_SPI_InitTypeDef SPI_InitStruct;
  
  BAR_SetDefaultValues(&Bar0);
  BAR_SetDefaultValues(&Bar1);
 
  // Configure SPI for motion sensor
  SPI_InitStruct.TransferDirection = LL_SPI_FULL_DUPLEX;
  SPI_InitStruct.Mode = LL_SPI_MODE_MASTER;
  SPI_InitStruct.DataWidth = LL_SPI_DATAWIDTH_8BIT;
  SPI_InitStruct.ClockPolarity = LL_SPI_POLARITY_LOW;           // Inactive clock state - low
  SPI_InitStruct.ClockPhase = LL_SPI_PHASE_1EDGE;               // Data shwitched out on the first clock edge
  SPI_InitStruct.NSS = LL_SPI_NSS_SOFT;                         // Use software controlled NSS (nCS) signal
  SPI_InitStruct.BaudRate = LL_SPI_BAUDRATEPRESCALER_DIV16;      // 42 MHz APB_CLock / 8 = 5.25 MHz
  SPI_InitStruct.BitOrder = LL_SPI_MSB_FIRST;                   // Bit order - Most significant bit first
  SPI_InitStruct.CRCCalculation = LL_SPI_CRCCALCULATION_DISABLE;
  LL_SPI_Init(BAR_SPI, &SPI_InitStruct);
  
  LL_SPI_SetStandard(BAR_SPI, LL_SPI_PROTOCOL_MOTOROLA);
  
  LL_SPI_EnableDMAReq_TX(BAR_SPI);     // Enable DMA Request for data transmission
  LL_SPI_EnableDMAReq_RX(BAR_SPI);     // Enable DMA REquest for data reception
  
  // Configure DMA for data transmission to motion sensor
  LL_DMA_StructInit(&DMA_InitStruct);
  DMA_InitStruct.Channel = BAR_TX_CHANNEL;
  DMA_InitStruct.Direction = LL_DMA_DIRECTION_MEMORY_TO_PERIPH;
  DMA_InitStruct.MemoryOrM2MDstAddress = (uint32_t)&BarCommon.TxBuffer;
  DMA_InitStruct.NbData = BarCommon.Length;
  DMA_InitStruct.PeriphOrM2MSrcIncMode = LL_DMA_PERIPH_NOINCREMENT;
  DMA_InitStruct.MemoryOrM2MDstIncMode = LL_DMA_MEMORY_INCREMENT;
  DMA_InitStruct.PeriphOrM2MSrcAddress = (uint32_t)&BAR_SPI->DR;
  LL_DMA_Init(BAR_TX_DMA, BAR_TX_STREAM, &DMA_InitStruct);
  
  LL_DMA_SetChannelSelection(BAR_TX_DMA, BAR_TX_STREAM, BAR_TX_CHANNEL);
  LL_DMA_SetStreamPriorityLevel(BAR_TX_DMA, BAR_TX_STREAM, LL_DMA_PRIORITY_LOW);
  LL_DMA_SetMode(BAR_TX_DMA, BAR_TX_STREAM, LL_DMA_MODE_NORMAL);
  LL_DMA_DisableFifoMode(BAR_TX_DMA, BAR_TX_STREAM);
  
  // Configure DMA for data reception from motion sensor
  LL_DMA_StructInit(&DMA_InitStruct);
  DMA_InitStruct.Channel = BAR_RX_CHANNEL;
  DMA_InitStruct.Direction = LL_DMA_DIRECTION_PERIPH_TO_MEMORY;
  DMA_InitStruct.MemoryOrM2MDstAddress = (uint32_t)&BarCommon.RxBuffer;
  DMA_InitStruct.NbData = BarCommon.Length;
  DMA_InitStruct.PeriphOrM2MSrcIncMode = LL_DMA_PERIPH_NOINCREMENT;
  DMA_InitStruct.MemoryOrM2MDstIncMode = LL_DMA_MEMORY_INCREMENT;
  DMA_InitStruct.PeriphOrM2MSrcAddress = (uint32_t)&BAR_SPI->DR;
  LL_DMA_Init(BAR_RX_DMA, BAR_RX_STREAM, &DMA_InitStruct);
  
  LL_DMA_SetChannelSelection(BAR_RX_DMA, BAR_RX_STREAM, BAR_RX_CHANNEL);
  LL_DMA_SetStreamPriorityLevel(BAR_RX_DMA, BAR_RX_STREAM, LL_DMA_PRIORITY_LOW);
  LL_DMA_SetMode(BAR_RX_DMA, BAR_RX_STREAM, LL_DMA_MODE_NORMAL);
  LL_DMA_DisableFifoMode(BAR_RX_DMA, BAR_RX_STREAM);  
  
  BAR_SelectDevice();
}


void BAR_SendComand(BAR_Cmd_e command)
{
  BAR_SelectDevice();
  
  BAR_t *bar = (BarCommon.SelectedDevice == BAR0) ? &Bar0 : &Bar1;
  
  BarCommon.TxBuffer[0] = command;
  
  if(command == BAR_Cmd_ReadAdc) BarCommon.Length = 5;
  else if ((command & 0xF0) == 0xA0) BarCommon.Length = 4;
  else BarCommon.Length = 1;
  
  if((command & 0xF0) == 0x40) bar->MeasuredParameter = BAR_ParamType_Pressure;
  else if((command & 0xF0) == 0x50) bar->MeasuredParameter = BAR_ParamType_Temperature;
  
  LL_DMA_SetDataLength(BAR_TX_DMA, BAR_TX_STREAM, BarCommon.Length);
  LL_DMA_SetDataLength(BAR_RX_DMA, BAR_RX_STREAM, BarCommon.Length);
  BAR_StartTransfer();
}


void BAR_StartTransfer(void)
{
  if((!LL_DMA_IsEnabledStream(BAR_RX_DMA, BAR_RX_STREAM)) && (!LL_DMA_IsEnabledStream(BAR_TX_DMA, BAR_TX_STREAM)))
  {
    
    // Set NSS pin low
    LL_GPIO_ResetOutputPin(BAR0_NSS);
    LL_GPIO_ResetOutputPin(BAR1_NSS);     
    
    // Stop both Tx and Rx DMA transfers to restart them simultaneously
    // Disable TC interrupt to avoid generation of interrupt
    LL_DMA_DisableIT_TC(BAR_RX_DMA, BAR_RX_STREAM);
    LL_DMA_DisableStream(BAR_RX_DMA, BAR_RX_STREAM); // Stop DMA transfer
    LL_DMA_DisableStream(BAR_TX_DMA, BAR_TX_STREAM); // Stop DMA transfer 
    
    BAR_CLEAR_DMA_FLAGS();
    
    // Enable "Transfer Complete" interrupt
    LL_DMA_EnableIT_TC(BAR_RX_DMA, BAR_RX_STREAM);
    
    // Clear "Rx Is Not Empty" flag
    volatile uint8_t dummy;
    dummy = BAR_SPI->DR;
    
    // Start DMA transfer
    LL_DMA_EnableStream(BAR_RX_DMA, BAR_RX_STREAM);
    LL_DMA_EnableStream(BAR_TX_DMA, BAR_TX_STREAM);
    
    // When DMA transfer is ready, SPI can be enabled
    LL_SPI_Enable(BAR_SPI);   
  }
}


// This interrupt occurs when DMA transfers last byte from spi to memory
void BAR_IRQHandler(void)
{  
  
  
  BAR_CLEAR_DMA_FLAGS();
  
  // Set NSS pin high
  LL_GPIO_SetOutputPin(BAR0_NSS);
  LL_GPIO_SetOutputPin(BAR1_NSS);    
  
  BAR_t *bar = (BarCommon.SelectedDevice == BAR0) ? &Bar0 : &Bar1;
  
  if(BarCommon.TxBuffer[0] == BAR_Cmd_ReadAdc)
  {
    uint32_t data = (BarCommon.RxBuffer[1] << 16) + (BarCommon.RxBuffer[2] << 8) + BarCommon.RxBuffer[3];
    
    if(bar->MeasuredParameter == BAR_ParamType_Pressure)
    {
      LL_GPIO_SetOutputPin(OUT1);
      bar->PressureBuffer[bar->PressureIndex] = data;
      if(++bar->PressureIndex >= BAR_NUMBER_OF_PRESSURE_SAMPLES) bar->PressureIndex = 0;
      BAR_CalculateOutput(BarCommon.SelectedDevice);
    } 
    
    else if(bar->MeasuredParameter == BAR_ParamType_Temperature)
    {
      bar->TemperatureBuffer[bar->TemperatureIndex] = data;
      if(++bar->TemperatureIndex >= BAR_NUMBER_OF_TEMPERATURE_SAMPLES) bar->TemperatureIndex = 0;
    }
  }
  
  if((BarCommon.TxBuffer[0] & 0xF0) == (BAR_Cmd_ReadProm0))
  {
    int prom_address = (BarCommon.TxBuffer[0] & 0x0F) >> 1;
    bar->PromCoefficients[prom_address] = (BarCommon.RxBuffer[1] << 8) + BarCommon.RxBuffer[2];
  }
  
  LL_GPIO_ResetOutputPin(OUT1);
}


void BAR_CalculateOutput(int device)
{
    BAR_t *bar = (device == BAR0) ? &Bar0 : &Bar1 ;
    
    int i;
    bar->RawPressure = 0;
    bar->RawTemperature = 0;
    
    for(i = 0; i < BAR_NUMBER_OF_TEMPERATURE_SAMPLES; i++)
      bar->RawTemperature += bar->TemperatureBuffer[i];
    
    bar->RawTemperature = bar->RawTemperature / BAR_NUMBER_OF_TEMPERATURE_SAMPLES;
    
    for(i = 0; i < BAR_NUMBER_OF_PRESSURE_SAMPLES; i++)
      bar->RawPressure += bar->PressureBuffer[i];

    bar->RawPressure = bar->RawPressure / BAR_NUMBER_OF_PRESSURE_SAMPLES;
    
    
    float a_temp = (float)bar->PromCoefficients[6] / (1 << 23) * 0.01;
    float b_temp = 20.0 - (float)bar->PromCoefficients[5] * (float)bar->PromCoefficients[6] / (1 << 15) * 0.01;
    
    float a_offset = (float)bar->PromCoefficients[4] / 128.0;
    float b_offset = (float)bar->PromCoefficients[2] * 65536.0 - (float)bar->PromCoefficients[4] * (float)bar->PromCoefficients[5] * 2.0;
    
    float a_sensitivity = (float)bar->PromCoefficients[3] / 256.0 ;
    float b_sensitivity = (float)bar->PromCoefficients[2] * 32768.0 - (float)bar->PromCoefficients[3] * (float)bar->PromCoefficients[5];
    
    float sensitivity = (a_sensitivity * (float)bar->RawTemperature + b_sensitivity ) / (1 << 21);
    float offset = (a_offset * (float)bar->RawTemperature + b_offset);

    bar->Values.Pressure = (sensitivity * bar->RawPressure - offset) / (1 << 15); 
    bar->Values.Temperature = a_temp * (float)bar->RawTemperature + b_temp;
     /*
    // Calculate Temperature
    int32_t temp_delta = bar->RawTemperature - (bar->PromCoefficients[5] << 8);
    bar->Values.Temperature = 20.0 + ((temp_delta * bar->PromCoefficients[6]) / (1 << 23)) * 0.01;
         
    // Calculate pressure
    int64_t offset = ((int64_t)bar->PromCoefficients[2] << 16) + (int64_t)((bar->PromCoefficients[4] * temp_delta) / (1 << 7));
    int64_t sensitivity = (bar->PromCoefficients[1] << 15) + ((bar->PromCoefficients[3] * temp_delta) >> 8);
    bar->Values.Pressure = (((bar->RawPressure * sensitivity >> 21) -  offset) >> 15);    
    */
}


int BAR_Crc(BAR_t *bar)
{
  int result = 0;
  int i;
  uint16_t crcBackup;
  uint16_t reminder = 0;
  uint8_t bit;
  
  crcBackup = bar->PromCoefficients[7];          // Save read CRC 
  bar->PromCoefficients[7] &= 0xFF00;           // Replace CRC byte by 0
  
  for(i = 0; i < 16; i++)
  {
    // Chose LSB or MSB and perform XOR on reminder
    if(i & 0x01) reminder ^= (uint8_t)(bar->PromCoefficients[i >> 1] & 0x00FF);
    else reminder ^= (uint8_t)((bar->PromCoefficients[i >> 1] & 0xFF00) >> 8);
    
    for(bit = 8; bit > 0; bit--)
    {
      if(reminder & 0x8000) reminder = (reminder << 1) ^ 0x3000;
      else reminder = reminder << 1;
    }
  }
  
  reminder = (0x000F & (reminder >> 12));
  bar->PromCoefficients[7] = crcBackup;
  
  if((bar->PromCoefficients[7] & 0x000F) == reminder) 
  {
    result = 1;
  }
    
  return result;
}


void BAR_Run(void)
{ 
    BarCommon.SelectedDevice = BAR0;
    
    BAR_RunSelectedDevice();
}


void BAR_RunSelectedDevice()
{  
  BAR_t *bar = (BarCommon.SelectedDevice == BAR0) ? &Bar0 : &Bar1;
  
  if(bar->WaitTime > 0)
  {
    bar->WaitTime--;
  }
  else
  {
    switch (bar->InternalState)
    {
    case BAR_State_Error:
      break;
      
    case BAR_State_Uninitialized:
      bar->InternalState = BAR_State_Initialization;
      bar->WaitTime = 160;
      break;
      
    case BAR_State_Initialization:
      BAR_SendComand(BAR_Cmd_Reset);
      bar->InternalState = BAR_State_ReadProm0;
      bar->WaitTime = 80;
      break;
      
    case BAR_State_ReadProm0:      
      BAR_SendComand(BAR_Cmd_ReadProm0);
      bar->InternalState = BAR_State_ReadProm1;
      bar->WaitTime = 80;
      break;
      
    case BAR_State_ReadProm1:      
      BAR_SendComand(BAR_Cmd_ReadProm1);
      bar->InternalState = BAR_State_ReadProm2;
      break;
      
    case BAR_State_ReadProm2:      
      BAR_SendComand(BAR_Cmd_ReadProm2);
      bar->InternalState = BAR_State_ReadProm3;
      break;
      
    case BAR_State_ReadProm3:      
      BAR_SendComand(BAR_Cmd_ReadProm3);
      bar->InternalState = BAR_State_ReadProm4;
      break;
      
    case BAR_State_ReadProm4:      
      BAR_SendComand(BAR_Cmd_ReadProm4);
      bar->InternalState = BAR_State_ReadProm5;
      break;
      
    case BAR_State_ReadProm5:      
      BAR_SendComand(BAR_Cmd_ReadProm5);
      bar->InternalState = BAR_State_ReadProm6;
      break;
      
    case BAR_State_ReadProm6:      
      BAR_SendComand(BAR_Cmd_ReadProm6);
      bar->InternalState = BAR_State_ReadProm7;
      break;
      
    case BAR_State_ReadProm7:      
      BAR_SendComand(BAR_Cmd_ReadProm7);
      bar->InternalState = BAR_State_CrcCheck;
      break;
      
    case BAR_State_CrcCheck:      
      BAR_SendComand(BAR_Cmd_ReadProm7);
      if(BAR_Crc(bar)) bar->InternalState = BAR_State_ConvertPressure;
      else bar->InternalState = BAR_State_Error;
      break;
      
    case BAR_State_ConvertPressure:     
      BAR_SendComand(BAR_Cmd_ConvertD1_4096);
      bar->InternalState = BAR_State_ReadPressure;
      bar->WaitTime = 80;
      break;
      
    case BAR_State_ReadPressure: 
      BAR_SendComand(BAR_Cmd_ReadAdc);
      bar->InternalState = BAR_State_ConvertTemperature;
      bar->WaitTime = 2;
      break;   
      
    case BAR_State_ConvertTemperature: 
      BAR_SendComand(BAR_Cmd_ConvertD2_4096);
      bar->InternalState = BAR_State_ReadTemperature;
      bar->WaitTime = 80;
      break;  
      
    case BAR_State_ReadTemperature: 
      BAR_SendComand(BAR_Cmd_ReadAdc);
      bar->InternalState = BAR_State_ConvertPressure;
     bar->WaitTime = 2;
      break;
      
    default:
      break;
    }
  }
}