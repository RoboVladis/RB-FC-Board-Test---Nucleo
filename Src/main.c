#include "fc_board.h"

int main(void)
{
  // Configure borad
  InitBoard();

  // Enable global interrupts
  __enable_irq();

  // Background loop
  while (1)
  {
    TX1_Transmit();

    //LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_12);
  }

}

void TIM2_IRQHandler(void)
{
  LL_TIM_ClearFlag_UPDATE(TIM2);
  LL_TIM_ClearFlag_UPDATE(TIM2);
 
  GlobalTime++;
  
  SoftTimer1++;
  if(SoftTimer1 >= 800)
  {
    SoftTimer1 = 0;
    char str[256];
    sprintf(str, "%d Hello!\r\n", GlobalTime);
    TX1_WriteToFifo(str);
  }
  
  LL_GPIO_SetOutputPin(GPIOA, LL_GPIO_PIN_12);
}



