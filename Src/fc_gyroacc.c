#include "fc_gyroacc.h"

ACC_t Acc0;
ACC_t Acc1;

unsigned long ACC0_Timer = 0;
unsigned long ACC1_Timer = 0;


void ACC_Init(int device)
{  
  ACC_t *acc;
  
  SPI_TypeDef *spi;
  
  DMA_TypeDef *tx_dma;
  uint32_t tx_stream;
  uint32_t tx_channel;
  
  DMA_TypeDef *rx_dma;
  uint32_t rx_stream;
  uint32_t rx_channel;  
  
  if(device == ACC0)
  {
    acc = &Acc0;
    spi = ACC0_SPI;
    
    tx_dma = ACC0_TX_DMA;
    tx_stream = ACC0_TX_STREAM;
    tx_channel = ACC0_TX_CHANNEL;
    
    rx_dma = ACC0_RX_DMA;
    rx_stream = ACC0_RX_STREAM;
    rx_channel = ACC0_RX_CHANNEL;     
  }
  
  else
  {
    acc = &Acc1;
    spi = ACC1_SPI;
    
    tx_dma = ACC1_TX_DMA;
    tx_stream = ACC1_TX_STREAM;
    tx_channel = ACC1_TX_CHANNEL;
    
    rx_dma = ACC1_RX_DMA;
    rx_stream = ACC1_RX_STREAM;
    rx_channel = ACC1_RX_CHANNEL;
  }
    
  
  ACC_SetDefaultValues(acc);
  
  LL_DMA_InitTypeDef DMA_InitStruct;   
  LL_SPI_InitTypeDef SPI_InitStruct;
  
  // Configure SPI for motion sensor
  SPI_InitStruct.TransferDirection = LL_SPI_FULL_DUPLEX;
  SPI_InitStruct.Mode = LL_SPI_MODE_MASTER;
  SPI_InitStruct.DataWidth = LL_SPI_DATAWIDTH_8BIT;
  SPI_InitStruct.ClockPolarity = LL_SPI_POLARITY_LOW;           // Inactive clock state - low
  SPI_InitStruct.ClockPhase = LL_SPI_PHASE_1EDGE;               // Data shwitched out on the first clock edge
  SPI_InitStruct.NSS = LL_SPI_NSS_SOFT;                         // Use software controlled NSS (nCS) signal
  SPI_InitStruct.BaudRate = LL_SPI_BAUDRATEPRESCALER_DIV16;     // 42 MHz APB_CLock / 8 = 5.25 MHz
  SPI_InitStruct.BitOrder = LL_SPI_MSB_FIRST;                   // Bit order - Most significant bit first
  SPI_InitStruct.CRCCalculation = LL_SPI_CRCCALCULATION_DISABLE;
  LL_SPI_Init(spi, &SPI_InitStruct);
  
  LL_SPI_SetStandard(spi, LL_SPI_PROTOCOL_MOTOROLA);
  
  LL_SPI_EnableDMAReq_TX(spi);     // Enable DMA Request for data transmission
  LL_SPI_EnableDMAReq_RX(spi);     // Enable DMA REquest for data reception
  
  // Configure DMA for data transmission to motion sensor
  LL_DMA_StructInit(&DMA_InitStruct);
  DMA_InitStruct.Channel = tx_channel;
  DMA_InitStruct.Direction = LL_DMA_DIRECTION_MEMORY_TO_PERIPH;
  DMA_InitStruct.MemoryOrM2MDstAddress = (uint32_t)&acc->tx_buffer;
  DMA_InitStruct.NbData = acc->length;
  DMA_InitStruct.PeriphOrM2MSrcIncMode = LL_DMA_PERIPH_NOINCREMENT;
  DMA_InitStruct.MemoryOrM2MDstIncMode = LL_DMA_MEMORY_INCREMENT;
  DMA_InitStruct.PeriphOrM2MSrcAddress = (uint32_t)&spi->DR;
  LL_DMA_Init(tx_dma, tx_stream, &DMA_InitStruct);
  
  LL_DMA_SetChannelSelection(tx_dma, tx_stream, tx_channel);
  LL_DMA_SetStreamPriorityLevel(tx_dma, tx_stream, LL_DMA_PRIORITY_LOW);
  LL_DMA_SetMode(tx_dma, tx_stream, LL_DMA_MODE_NORMAL);
  LL_DMA_DisableFifoMode(tx_dma, tx_stream);
  
  // Configure DMA for data reception from motion sensor
  LL_DMA_StructInit(&DMA_InitStruct);
  DMA_InitStruct.Channel = rx_channel;
  DMA_InitStruct.Direction = LL_DMA_DIRECTION_PERIPH_TO_MEMORY;
  DMA_InitStruct.MemoryOrM2MDstAddress = (uint32_t)&acc->rx_buffer;
  DMA_InitStruct.NbData = acc->length;
  DMA_InitStruct.PeriphOrM2MSrcIncMode = LL_DMA_PERIPH_NOINCREMENT;
  DMA_InitStruct.MemoryOrM2MDstIncMode = LL_DMA_MEMORY_INCREMENT;
  DMA_InitStruct.PeriphOrM2MSrcAddress = (uint32_t)&spi->DR;
  LL_DMA_Init(rx_dma, rx_stream, &DMA_InitStruct);
  
  LL_DMA_SetChannelSelection(rx_dma, rx_stream, rx_channel);
  LL_DMA_SetStreamPriorityLevel(rx_dma, rx_stream, LL_DMA_PRIORITY_LOW);
  LL_DMA_SetMode(rx_dma, rx_stream, LL_DMA_MODE_NORMAL);
  LL_DMA_DisableFifoMode(rx_dma, rx_stream);  
  
}

void Acc_ReadData(int device)
{
  ACC_t *acc = (device == ACC0) ? &Acc0 : &Acc1;
  
  acc->tx_buffer[0] = 0x3B | 0x80; // Address of first register plus RW bit (0x80)
  acc->length = (0x48 - 0x3B + 2);  // Read all registers from the range
  
  if(device == ACC0) ACC0_StartTransfer();
  else ACC1_StartTransfer();
}

void Acc_ReadConfig(int device)
{
  ACC_t *acc = (device == ACC0) ? &Acc0 : &Acc1;
  
  acc->tx_buffer[0] = 0x19 | 0x80; // Address of first register plus RW bit (0x80)
  acc->length = (0x70 - 0x19 + 2);  // Read all registers from the range
  
  if(device == ACC0) ACC0_StartTransfer();
  else ACC1_StartTransfer();
}

void Acc_WriteConfig1(int device)
{
  ACC_t *acc = (device == ACC0) ? &Acc0 : &Acc1;
  
  acc->tx_buffer[0] = 0x19;             // Address of first register without RW bit
  acc->tx_buffer[1] = 0x00;             // Smplrt_div: set divider to 1  
  acc->tx_buffer[2] = 0x00;             // Config: enable module and set bandwidth filter to 250 Hz
  acc->tx_buffer[3] = 0x00;             // Gyro config: gyro full scale = 250 dps; self test disabled; data rate = 8kHz
  acc->tx_buffer[4] = 0x00;             // Accel config: full scale = 2g, self test disabled; 
  acc->tx_buffer[5] = 0x08;             // Accel config 2: data rate = 4kHz
  acc->tx_buffer[6] = 0x00;             // Lp_mode_cfg: turn low-power mode off
  acc->length = (6 + 2);  // Read all registers from the range
  
  if(device == ACC0) ACC0_StartTransfer();
  else ACC1_StartTransfer();
}

void Acc_WriteConfig2(int device)
{
  ACC_t *acc = (device == ACC0) ? &Acc0 : &Acc1;
  
  acc->tx_buffer[0] = 0x6A;             // Address of first register without RW bit
  acc->tx_buffer[1] = 0x00;             // User_ctrl: 
  acc->tx_buffer[2] = 0x01;             // Pwr_mgmt_1: 
  acc->tx_buffer[3] = 0x00;             // Pwr_mgmt_2
  acc->tx_buffer[4] = 0x40;             // I2C_if: disable I2C interface
  acc->length = (4 + 2);  // Read all registers from the range
  
  if(device == ACC0) ACC0_StartTransfer();
  else ACC1_StartTransfer();
}

//! \brief      Transfers prepared number of bytes and to motion sensor using DMA and SPI combination
void ACC0_StartTransfer(void)
{ 
  // Stop both Tx and Rx DMA transfers to start them simultaneously
  // Disable TC interrupt to avoid generation of interrupt
  LL_DMA_DisableIT_TC(ACC0_RX_DMA, ACC0_RX_STREAM);
  LL_DMA_DisableStream(ACC0_RX_DMA, ACC0_RX_STREAM); // Stop DMA transfer
  LL_DMA_DisableStream(ACC0_TX_DMA, ACC0_TX_STREAM); // Stop DMA transfer 
  
  ACC0_CLEAR_DMA_FLAGS();

  // Set number of bytes to transmit
  LL_DMA_SetDataLength(ACC0_TX_DMA, ACC0_TX_STREAM, Acc0.length);
  LL_DMA_SetDataLength(ACC0_RX_DMA, ACC0_RX_STREAM, Acc0.length);
 
  // Enable "Transfer Complete" interrupt
  LL_DMA_EnableIT_TC(ACC0_RX_DMA, ACC0_RX_STREAM);

  //Clear RXNE flag
  volatile uint8_t dummy;
  dummy = ACC0_SPI->DR;

  // Set NSS pin low
  LL_GPIO_ResetOutputPin(ACC0_NSS);
  
  // Start DMA transfer
  LL_DMA_EnableStream(ACC0_RX_DMA, ACC0_RX_STREAM);
  LL_DMA_EnableStream(ACC0_TX_DMA, ACC0_TX_STREAM);

  LL_SPI_Enable(ACC0_SPI); 
}

void ACC1_StartTransfer(void)
{ 
  // Stop both Tx and Rx DMA transfers to start them simultaneously
  // Disable TC interrupt to avoid generation of interrupt
  LL_DMA_DisableIT_TC(ACC1_RX_DMA, ACC1_RX_STREAM);
  LL_DMA_DisableStream(ACC1_RX_DMA, ACC1_RX_STREAM); // Stop DMA transfer
  LL_DMA_DisableStream(ACC1_TX_DMA, ACC1_TX_STREAM); // Stop DMA transfer 
  
  ACC1_CLEAR_DMA_FLAGS();

  // Set number of bytes to transmit
  LL_DMA_SetDataLength(ACC1_TX_DMA, ACC1_TX_STREAM, Acc1.length);
  LL_DMA_SetDataLength(ACC1_RX_DMA, ACC1_RX_STREAM, Acc1.length);
 
  // Enable "Transfer Complete" interrupt
  LL_DMA_EnableIT_TC(ACC1_RX_DMA, ACC1_RX_STREAM);

  // Clear "Rx Is Not Empty" flag
  volatile uint8_t dummy;
  dummy = ACC1_SPI->DR;

  // Set NSS pin low
  LL_GPIO_ResetOutputPin(ACC1_NSS);
  
  // Start DMA transfer
  LL_DMA_EnableStream(ACC1_RX_DMA, ACC1_RX_STREAM);
  LL_DMA_EnableStream(ACC1_TX_DMA, ACC1_TX_STREAM);

  LL_SPI_Enable(ACC1_SPI); 
}


void ACC_SetDefaultValues(ACC_t *acc)
{
  acc->state = ACC_State_Uninitialized;
  
  acc->length = 0;
  
  int i = 0;
  uint8_t *ptr;
  
  ptr = (uint8_t*)&acc->regs;
  while(i++ < sizeof(acc->regs)) *ptr++ = 0; 
  
  i = 0;
  ptr = acc->tx_buffer;
  while(i++ < sizeof(acc->tx_buffer)) *ptr++ = 0;
  
  i = 0;
  ptr = acc->rx_buffer;
  while(i++ < sizeof(acc->rx_buffer)) *ptr++ = 0;  
}

// This interrupt occurs when DMA transfers last byte from spi to memory
void ACCO_IRQHandler(void)
{ 
  if(LL_DMA_IsActiveFlag_TC0(ACC0_RX_DMA))
  {
    ACC0_CLEAR_DMA_FLAGS();
    //LL_SPI_Disable(ACC0_SPI);
    
    // Set NSS pin high
    LL_GPIO_SetOutputPin(ACC0_NSS);
    
    // If first transmitted byte contains high read/write bit,
    // read all following bytes and save them to the corresponding registers
    if(Acc0.tx_buffer[0] & 0x80)
    {
      uint8_t *ptr = (uint8_t *)&Acc0.regs + (Acc0.tx_buffer[0] & 0x7F);
      int i;
      for(i = 1; i < Acc0.length; i++) *ptr++ = Acc0.rx_buffer[i];
    }  
  } 
  
}

// This interrupt occurs when DMA transfers last byte from spi to memory
void ACC1_IRQHandler(void)
{ 
  if(LL_DMA_IsActiveFlag_TC2(ACC1_RX_DMA))
  {
    ACC1_CLEAR_DMA_FLAGS();
    //LL_SPI_Disable(ACC1_SPI);
    
    // Set NSS pin high
    LL_GPIO_SetOutputPin(ACC1_NSS);
    
    // If first transmitted byte contains high read/write bit,
    // read all following bytes and save them to the corresponding registers
    if(Acc1.tx_buffer[0] & 0x80)
    {
      uint8_t *ptr = (uint8_t *)&Acc1.regs + (Acc1.tx_buffer[0] & 0x7F);
      int i;
      for(i = 1; i < Acc1.length; i++) *ptr++ = Acc1.rx_buffer[i];
    }  
  }   
}

void ACC_Run(int device)
{
  ACC_t *acc;
  unsigned long *timer;
  
  if(device == ACC0)
  {
    acc = &Acc0;
    timer = &ACC0_Timer;
  }
  
  else
  {
    acc = &Acc1;
    timer = &ACC1_Timer;
  }
  
  
  if(acc->state == ACC_State_Initialized)
  {
    Acc_ReadData(device);
  }
  
  else
  {
    if(++(*timer) >= 300)
    {
      *timer = 0; 
      
      switch (acc->state)
      {
      case ACC_State_Error:
        break;
        
      case ACC_State_Uninitialized:
        Acc_WriteConfig1(device);
        acc->state = ACC_State_Initialization1;
        break;
        
      case ACC_State_Initialization1:
        Acc_WriteConfig2(device);
        acc->state = ACC_State_Initialization2;
        break;
        
      case ACC_State_Initialization2:
        Acc_ReadConfig(device);
        acc->state = ACC_State_Initialization3;
        break;
        
      case ACC_State_Initialization3:
        acc->state = ACC_State_Initialized;
        break;
        
      default:
        break;      
      }   
    } 
  }
  
}