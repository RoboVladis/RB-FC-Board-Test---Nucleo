#include "fc_board.h"

#define INTERVAL_DATA_OUTPUT    (SYSTICK_FREQUENCY / 20)

volatile double temp_value[4];

volatile int dataRx = 0xAB;
volatile int dummyData = 0;
unsigned long SoftTimerCompare1 = 0;

typedef struct _FC_Data_t_
{
  float acceleration[6];
  float rotation[6];  
  float pressure[2];
  double temperature[2];
}FC_Data_t;


FC_Data_t GlobalData;

void WriteReport(void);

int main(void)
{
  // Configure borad
  InitBoard();
  
  // Enable global interrupts
  __enable_irq();

  // Turn on required devices
  LL_GPIO_SetOutputPin(LOADSW_ACC0);
  LL_GPIO_SetOutputPin(LOADSW_ACC1); 
  LL_GPIO_SetOutputPin(LOADSW_BAR0);
  LL_GPIO_SetOutputPin(LOADSW_BAR1);
  
  // Background loop
  while (1)
  {
    
    if((SoftTimer - SoftTimerCompare1) > INTERVAL_DATA_OUTPUT)
    {    
      SoftTimerCompare1 = SoftTimer;     
      GlobalTime++;      
      //CalculateOutput();      
      WriteReport();      
    }
    
    TX1_Run();
    TX2_Run();
  }
}


void SysTick_Handler(void)
{ 
  LL_GPIO_SetOutputPin(OUT1);
  LL_GPIO_TogglePin(BAR0_MISO);  
  SoftTimer++;  
  
  ACC_Run(ACC0);
  ACC_Run(ACC1);
  
  BAR_Run();
  
  /*
  if(++SoftTimer2 >= 400)
  {
    SoftTimer2 = 0;     
    LOADSW_WriteRegs(0);
  }  
  else if (SoftTimer2 == 100) LOADSW_WriteRegs(1);
  else if (SoftTimer2 == 200) LOADSW_ReadRegs(0);
  else if (SoftTimer2 == 300) LOADSW_ReadRegs(1);
*/

  LL_GPIO_ResetOutputPin(OUT1); 
}


void WriteReport(void)
{
  // Transmit report via aux serial port
  char str[256];
  /*
  sprintf(str, "%d | %2.3fg %2.3fg %2.3fg | %3.2fdps %3.2fdps %3.2fdps || %2.3fg %2.3fg %2.3fg | %3.2fdps %3.2fdps %3.2fdps || %.3f mbar | %.3f mbar || %.2fC | %.2fC ||\r\n",
          GlobalTime,
          GlobalData.acceleration[0],
          GlobalData.acceleration[1],
          GlobalData.acceleration[2],
          GlobalData.rotation[0],
          GlobalData.rotation[1],
          GlobalData.rotation[2],
          GlobalData.acceleration[3],
          GlobalData.acceleration[4],
          GlobalData.acceleration[5],
          GlobalData.rotation[3],
          GlobalData.rotation[4],
          GlobalData.rotation[5],
          GlobalData.pressure[0],
          GlobalData.pressure[1],
          GlobalData.temperature[0],
          GlobalData.temperature[1]); 
  */
  
    sprintf(str, "%.2f %d %.2f %.2f\r\n",
          (GlobalTime / 20.0),
          Bar0.RawPressure,
          Bar0.Values.Pressure,
          Bar0.Values.Temperature);
  
  TX1_WriteToFifo(str);     

}