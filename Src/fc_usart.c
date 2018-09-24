#include "fc_usart.h"

USART_Fifo_t FifoTx1;
USART_Fifo_t FifoRx1;

USART_Fifo_t FifoTx2;
USART_Fifo_t FifoRx2;

void AUX1_Init(void)
{ 
  LL_USART_InitTypeDef USART_InitStruct;
  
  LL_DMA_InitTypeDef DMA_InitStruct; 
  
  USART_InitStruct.BaudRate = 115200;
  USART_InitStruct.DataWidth = LL_USART_DATAWIDTH_8B;
  USART_InitStruct.StopBits = LL_USART_STOPBITS_1;
  USART_InitStruct.Parity = LL_USART_PARITY_NONE;
  USART_InitStruct.TransferDirection = LL_USART_DIRECTION_TX_RX;
  USART_InitStruct.HardwareFlowControl = LL_USART_HWCONTROL_NONE;
  USART_InitStruct.OverSampling = LL_USART_OVERSAMPLING_8;
  LL_USART_Init(AUX1, &USART_InitStruct);
  AUX1->BRR = (0x2D << 4) + 0x04;
  
  LL_USART_ConfigAsyncMode(AUX1);  
  LL_USART_Enable(AUX1);
  LL_USART_EnableIT_RXNE(AUX1);
  
  // Enable DMA request for transmission
  LL_USART_EnableDMAReq_TX(AUX1);
  
  // Configure DMA for PC UART
  LL_DMA_StructInit(&DMA_InitStruct);
  DMA_InitStruct.Channel = TX1_CHANNEL;
  DMA_InitStruct.Direction = LL_DMA_DIRECTION_MEMORY_TO_PERIPH;
  DMA_InitStruct.MemoryOrM2MDstAddress = (uint32_t)&FifoTx1.buffer[FifoTx1.head][0];
  DMA_InitStruct.NbData = AUX_BUFFER_LENGTH;
  DMA_InitStruct.PeriphOrM2MSrcIncMode = LL_DMA_PERIPH_NOINCREMENT;
  DMA_InitStruct.MemoryOrM2MDstIncMode = LL_DMA_MEMORY_INCREMENT;
  DMA_InitStruct.PeriphOrM2MSrcAddress = (uint32_t)&AUX1->DR;
  LL_DMA_Init(TX1_DMA, TX1_STREAM, &DMA_InitStruct);
  
  LL_DMA_SetChannelSelection(TX1_DMA, TX1_STREAM, TX1_CHANNEL);
  LL_DMA_SetStreamPriorityLevel(TX1_DMA, TX1_STREAM, LL_DMA_PRIORITY_LOW);
  LL_DMA_SetMode(TX1_DMA, TX1_STREAM, LL_DMA_MODE_NORMAL);
  LL_DMA_DisableFifoMode(TX1_DMA, TX1_STREAM);
}

void AUX2_Init(void)
{ 
  LL_USART_InitTypeDef USART_InitStruct;
  
  LL_DMA_InitTypeDef DMA_InitStruct; 
  
  USART_InitStruct.BaudRate = 115200;
  USART_InitStruct.DataWidth = LL_USART_DATAWIDTH_8B;
  USART_InitStruct.StopBits = LL_USART_STOPBITS_1;
  USART_InitStruct.Parity = LL_USART_PARITY_NONE;
  USART_InitStruct.TransferDirection = LL_USART_DIRECTION_TX_RX;
  USART_InitStruct.HardwareFlowControl = LL_USART_HWCONTROL_NONE;
  USART_InitStruct.OverSampling = LL_USART_OVERSAMPLING_8;
  LL_USART_Init(AUX2, &USART_InitStruct);
  AUX2->BRR = (0x2D << 4) + 0x04;
  
  LL_USART_ConfigAsyncMode(AUX2);  
  LL_USART_Enable(AUX2);
  LL_USART_EnableIT_RXNE(AUX2);
  
  // Enable DMA request for transmission
  LL_USART_EnableDMAReq_TX(AUX2);
  
  // Configure DMA for PC UART
  LL_DMA_StructInit(&DMA_InitStruct);
  DMA_InitStruct.Channel = TX2_CHANNEL;
  DMA_InitStruct.Direction = LL_DMA_DIRECTION_MEMORY_TO_PERIPH;
  DMA_InitStruct.MemoryOrM2MDstAddress = (uint32_t)&FifoTx2.buffer[FifoTx2.head][0];
  DMA_InitStruct.NbData = AUX_BUFFER_LENGTH;
  DMA_InitStruct.PeriphOrM2MSrcIncMode = LL_DMA_PERIPH_NOINCREMENT;
  DMA_InitStruct.MemoryOrM2MDstIncMode = LL_DMA_MEMORY_INCREMENT;
  DMA_InitStruct.PeriphOrM2MSrcAddress = (uint32_t)&AUX2->DR;
  LL_DMA_Init(TX2_DMA, TX2_STREAM, &DMA_InitStruct);
  
  LL_DMA_SetChannelSelection(TX2_DMA, TX2_STREAM, TX2_CHANNEL);
  LL_DMA_SetStreamPriorityLevel(TX2_DMA, TX2_STREAM, LL_DMA_PRIORITY_LOW);
  LL_DMA_SetMode(TX2_DMA, TX2_STREAM, LL_DMA_MODE_NORMAL);
  LL_DMA_DisableFifoMode(TX2_DMA, TX2_STREAM);
}

// This function transmits data from buffer via auxiliary serial port #1 if buffer is not empty 
// Good place for this function is background loop
void TX1_Run(void)
{  
  // Check if data can be transmitted
  //    1. DMA stream must not be active
  //    2. Serial tx register must be empty and ready for new data
  //    3. The Tx FIFO buffer must contain unsent data
  if(!(TX1_DMA_STREAM->CR & 0x01) && (LL_USART_IsActiveFlag_TXE(AUX1)) && (FifoTx1.unprocessedBuffers > 0))
  {
    // Disable interrupts before modifying fifo buffers
    __disable_irq();
    
    signed char tail = (FifoTx1.head - FifoTx1.unprocessedBuffers);
    if(tail < 0)  tail +=  AUX_NUMBER_OF_BUFFERS;
    char *tailPtr = &FifoTx1.buffer[tail][0];
   
    // Clear "Transfer Error" and "Transfer Complete" flags
    LL_DMA_ClearFlag_TE7(TX1_DMA);
    LL_DMA_ClearFlag_TC7(TX1_DMA);
    
    // Set memory address & number of bytes to transmit
    LL_DMA_SetMemoryAddress(TX1_DMA, TX1_STREAM, (uint32_t)tailPtr);
    LL_DMA_SetDataLength(TX1_DMA, TX1_STREAM, AUX_BUFFER_LENGTH);   
    
    // Enable "Transfer Complete" interrupt
    LL_DMA_EnableIT_TC(TX1_DMA, TX1_STREAM);
    
    // Clear RXNE flag and Rx Data register. 
    // As long as dummy byte is not used further, compiler can ignore this action. 
    // Use "volatile" to avoid such kind of optimisation
    volatile uint8_t dummy;
    dummy = AUX1->DR;
    
    // Start DMA transfer
    LL_DMA_EnableStream(TX1_DMA, TX1_STREAM);
    
    // Move to the next string in cyclic FIFO buffer
    if(FifoTx1.unprocessedBuffers > 0) FifoTx1.unprocessedBuffers--;
    __enable_irq();
  }
}
// This function transmits data from buffer via auxiliary serial port #2 if buffer is not empty 
// Good place for this function is background loop
void TX2_Run(void)
{  
  // Check if data can be transmitted
  //    1. DMA stream must not be active
  //    2. Serial tx register must be empty and ready for new data
  //    3. The Tx FIFO buffer must contain unsent data
  if(!(TX2_DMA_STREAM->CR & 0x01) && (LL_USART_IsActiveFlag_TXE(AUX2)) && (FifoTx2.unprocessedBuffers > 0))
  {
    // Disable interrupts before modifying fifo buffers
    __disable_irq();
    
    signed char tail = (FifoTx2.head - FifoTx2.unprocessedBuffers);
    if(tail < 0)  tail +=  AUX_NUMBER_OF_BUFFERS;
    char *tailPtr = &FifoTx2.buffer[tail][0];
   
    // Clear "Transfer Error" flag
    LL_DMA_ClearFlag_TE6(TX2_DMA);
    LL_DMA_ClearFlag_TC6(TX2_DMA);
    
    // Clear "Transfer Error" and "Transfer Complete" flags
    LL_DMA_SetMemoryAddress(TX2_DMA, TX2_STREAM, (uint32_t)tailPtr);
    LL_DMA_SetDataLength(TX2_DMA, TX2_STREAM, AUX_BUFFER_LENGTH);   
    
    // Enable "Transfer Complete" interrupt
    LL_DMA_EnableIT_TC(TX2_DMA, TX2_STREAM);
    
    // Clear RXNE flag and Rx Data register. 
    // As long as dummy byte is not used further, compiler can ignore this action. 
    // Use "volatile" to avoid such kind of optimisation
    volatile uint8_t dummy;
    dummy = AUX2->DR;
    
    // Start DMA transfer
    LL_DMA_EnableStream(TX2_DMA, TX2_STREAM);
    
    // Move to the next string in cyclic FIFO buffer
    if(FifoTx2.unprocessedBuffers > 0) FifoTx2.unprocessedBuffers--;
    __enable_irq();
  }
}
  
void TX1_WriteToFifo(char *str)
{
  __disable_irq();
  
  strncpy(&FifoTx1.buffer[FifoTx1.head][0], str, AUX_BUFFER_LENGTH);
  
  FifoTx1.head = (FifoTx1.head < (AUX_NUMBER_OF_BUFFERS - 1)) ? FifoTx1.head + 1 : 0;
  FifoTx1.unprocessedBuffers = (FifoTx1.unprocessedBuffers < AUX_NUMBER_OF_BUFFERS) ?  FifoTx1.unprocessedBuffers + 1 :  FifoTx1.unprocessedBuffers;
  __enable_irq();
}

void TX2_WriteToFifo(char *str)
{
  __disable_irq();
  
  strncpy(&FifoTx2.buffer[FifoTx2.head][0], str, AUX_BUFFER_LENGTH);
  
  FifoTx2.head = (FifoTx2.head < (AUX_NUMBER_OF_BUFFERS - 1)) ? FifoTx2.head + 1 : 0;
  FifoTx2.unprocessedBuffers = (FifoTx2.unprocessedBuffers < AUX_NUMBER_OF_BUFFERS) ?  FifoTx2.unprocessedBuffers + 1 :  FifoTx2.unprocessedBuffers;
  __enable_irq();
}

void AUX1_IRQHandler(void)
{
  
  if(LL_USART_IsActiveFlag_ORE(AUX1))
  {
    LL_USART_ClearFlag_ORE(AUX1);
  }
  
  else if(LL_USART_IsActiveFlag_IDLE(AUX1))
  {   
    LL_USART_ClearFlag_IDLE(AUX1);
  }  
  
  else if (LL_USART_IsActiveFlag_RXNE(AUX1))
  {    
    USART_Fifo_t *fifo = &FifoRx1;
    
    volatile char byteRx; 
    
    // get received byte
    byteRx = AUX1->DR; 
    
    // if received character is lower case letter (a to z), make it upper case
    if((byteRx >= 'a') && (byteRx <= 'z')) byteRx -= 0x20;
    
    // make sure that buffer is not overloaded
    if(fifo->positionIn < AUX_BUFFER_LENGTH) 
    {
      fifo->buffer[fifo->head][fifo->positionIn] = byteRx; 
      fifo->positionIn++;          
    }   
    
    // if received character is "carriage return",
    // finish current buffer and increment fifo head position
    if(byteRx == '\r')
    {
      // reset position in current buffer to 0;
      fifo->positionIn = 0;
      
      // Increment fifo head position
      fifo->head = (fifo->head < (AUX_NUMBER_OF_BUFFERS - 1)) ? fifo->head + 1 : 0;
      fifo->unprocessedBuffers = (fifo->unprocessedBuffers < AUX_NUMBER_OF_BUFFERS) ?  fifo->unprocessedBuffers + 1 : fifo->unprocessedBuffers;
    }    
  }
}

void AUX2_IRQHandler(void)
{
  
  if(LL_USART_IsActiveFlag_ORE(AUX2))
  {
    LL_USART_ClearFlag_ORE(AUX2);
  }
  
  else if(LL_USART_IsActiveFlag_IDLE(AUX2))
  {   
    LL_USART_ClearFlag_IDLE(AUX2);
  }  
  
  else if (LL_USART_IsActiveFlag_RXNE(AUX2))
  {    
    USART_Fifo_t *fifo = &FifoRx2;
    
    volatile char byteRx; 
    
    // get received byte
    byteRx = AUX2->DR; 
    
    // if received character is lower case letter (a to z), make it upper case
    if((byteRx >= 'a') && (byteRx <= 'z')) byteRx -= 0x20;
    
    // make sure that buffer is not overloaded
    if(fifo->positionIn < AUX_BUFFER_LENGTH) 
    {
      fifo->buffer[fifo->head][fifo->positionIn] = byteRx; 
      fifo->positionIn++;          
    }   
    
    // if received character is "carriage return",
    // finish current buffer and increment fifo head position
    if(byteRx == '\r')
    {
      // reset position in current buffer to 0;
      fifo->positionIn = 0;
      
      // Increment fifo head position
      fifo->head = (fifo->head < (AUX_NUMBER_OF_BUFFERS - 1)) ? fifo->head + 1 : 0;
      fifo->unprocessedBuffers = (fifo->unprocessedBuffers < AUX_NUMBER_OF_BUFFERS) ?  fifo->unprocessedBuffers + 1 : fifo->unprocessedBuffers;
    }    
  }
}
