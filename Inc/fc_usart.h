// Define to prevent recursive inclusion
#ifndef __FC_USART_H
#define __FC_USART_H

#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>
#include <stdio.h> 
#include "stm32f4xx.h" 
#include "stm32f4xx_ll_dma.h"
#include "stm32f4xx_ll_usart.h"
#include "stm32f4xx_ll_gpio.h"
  
#define AUX_BUFFER_LENGTH     256
#define AUX_NUMBER_OF_BUFFERS 4  

#define AUX1                    USART1
#define AUX1_IRQHandler         USART1_IRQHandler
  
#define TX1                     USART1
#define TX1_DMA                 DMA2
#define TX1_STREAM              LL_DMA_STREAM_7
#define TX1_DMA_STREAM          DMA2_Stream7
#define TX1_CHANNEL             LL_DMA_CHANNEL_4
  
#define RX1                     USART1
#define RX1_DMA                 DMA2
#define RX1_STREAM              LL_DMA_STREAM_5
#define RX1_DMA_STREAM          DMA2_Stream5
#define RX1_CHANNEL             LL_DMA_CHANNEL_4 
  
  
#define AUX2                    USART6
#define AUX2_IRQHandler         USART6_IRQHandler
  
#define TX2                     USART6
#define TX2_DMA                 DMA2
#define TX2_STREAM              LL_DMA_STREAM_6
#define TX2_DMA_STREAM          DMA2_Stream6
#define TX2_CHANNEL             LL_DMA_CHANNEL_5
  
#define RX2                     USART6
#define RX2_DMA                 DMA2
#define RX2_STREAM              LL_DMA_STREAM_1
#define RX2_DMA_STREAM          DMA2_Stream1
#define RX2_CHANNEL             LL_DMA_CHANNEL_5     


typedef struct _AUX_Fifo_t_
{
  char buffer[AUX_NUMBER_OF_BUFFERS][AUX_BUFFER_LENGTH];      // the buffer of buffers
  unsigned int head;                // defines which buffer is currently written
  unsigned int positionIn;          // defines a write position in buffer
  unsigned int positionOut;         // defines a read position in buffer
  unsigned int status;              // defines the status of buffer
  unsigned int unprocessedBuffers;  // defines number of unprocessed buffers
}USART_Fifo_t;

extern USART_Fifo_t FifoTx1;
extern USART_Fifo_t FifoRx1;

extern USART_Fifo_t FifoTx2;
extern USART_Fifo_t FifoRx2;

extern void AUX1_Init(void);
extern void AUX2_Init(void);

extern void TX1_Run(void);
extern void TX2_Run(void);

extern void TX1_WriteToFifo(char *str);
extern void TX2_WriteToFifo(char *str);

extern void AUX1_IRQHandler(void);
extern void AUX2_IRQHandler(void);

  
#ifdef __cplusplus
}
#endif

#endif // __FC_USART_H