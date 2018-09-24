#include "fc_load_sw.h"

LOADSW_t LoadSw;

void LOADSW_Init(void)
{
  LoadSw.device = SW0;          // Target device: LoadSwitch #0 (SW0)
  LoadSw.index = 0;             // Reset buffer index
  LoadSw.read = 0;              // Defaul operation - read
  LoadSw.regAddr = 0x05;         // Set register address to Control reg (0x05)
    
  // Fill buffer with zeros 
  int i = 0;
  int *ptr = &LoadSw.buffer[0][0];
  while(i < sizeof(LoadSw.buffer))
  {
    *ptr++ = 0;
    i++;    
  }
  
  LL_I2C_InitTypeDef I2C_InitStruct;
  
  // Initialize I2C module
  I2C_InitStruct.PeripheralMode = LL_I2C_MODE_I2C;
  I2C_InitStruct.ClockSpeed = 100000;
  I2C_InitStruct.DutyCycle = LL_I2C_DUTYCYCLE_16_9;
  I2C_InitStruct.OwnAddress1 = 0;
  I2C_InitStruct.TypeAcknowledge = LL_I2C_ACK;
  I2C_InitStruct.OwnAddrSize = LL_I2C_OWNADDRESS1_7BIT;
  LL_I2C_Init(LOADSW_I2C, &I2C_InitStruct);
  
  LL_I2C_SetClockSpeedMode(LOADSW_I2C, LL_I2C_CLOCK_SPEED_STANDARD_MODE);
  LL_I2C_DisableAnalogFilter(LOADSW_I2C);
  LL_I2C_SetOwnAddress2(LOADSW_I2C, 0);
  LL_I2C_DisableOwnAddress2(LOADSW_I2C);
  LL_I2C_DisableGeneralCall(LOADSW_I2C);
  LL_I2C_EnableClockStretching(LOADSW_I2C);
    
  // Enable I2C peripheral
  LL_I2C_Enable(LOADSW_I2C);                       
}

void LOADSW_ReadRegs(int device)
{
  if(!LL_I2C_IsActiveFlag_BUSY(LOADSW_I2C))
  {
    LoadSw.index = 1;
    LoadSw.regAddr = 0x01 | 0x80;
    LoadSw.read = 1;         
    LoadSw.device = (device == SW0) ? SW0 : SW1;

    LOADSW_StartTransmission();
  }
}

void LOADSW_StartTransmission(void)
{
    LL_I2C_GenerateStartCondition(LOADSW_I2C); 
    LL_I2C_EnableIT_EVT(LOADSW_I2C);
    LL_I2C_EnableIT_ERR(LOADSW_I2C);
    LL_I2C_EnableIT_BUF(LOADSW_I2C);
    LL_I2C_Enable(LOADSW_I2C);
}

void LOADSW_WriteRegs(int device)
{
  if(!LL_I2C_IsActiveFlag_BUSY(LOADSW_I2C))
  {    
    LoadSw.index = 1;
    LoadSw.regAddr = 0x01 | 0x80;      // Start Writing at reg 0x01 & set autoincrement bit
    LoadSw.read = 0;
    LoadSw.device = (device == SW0) ? SW0 : SW1;
    
    LoadSw.buffer[0][0] = 0x00;
    LoadSw.buffer[0][1] = 0x02;
    LoadSw.buffer[0][2] = 0x02;
    LoadSw.buffer[0][3] = 0x02;
    LoadSw.buffer[0][4] = 0x02;
    LoadSw.buffer[0][5] = 0x00;
    
    LoadSw.buffer[1][0] = 0x00;
    LoadSw.buffer[1][1] = 0x02;
    LoadSw.buffer[1][2] = 0x02;
    LoadSw.buffer[1][3] = 0x02;
    LoadSw.buffer[1][4] = 0x02;
    LoadSw.buffer[1][5] = 0x00;  

    LOADSW_StartTransmission();
  }
}

void LOADSW_ER_IRQHandler(void)
{
  // Acknowledge failure
  if(LL_I2C_IsActiveFlag_AF(LOADSW_I2C))
  {
    LL_I2C_ClearFlag_AF(LOADSW_I2C);
    LL_I2C_GenerateStopCondition(LOADSW_I2C);    
  }
  
  // Arbitration lost
  if(LL_I2C_IsActiveFlag_ARLO(LOADSW_I2C))
  {
    LL_I2C_ClearFlag_ARLO(LOADSW_I2C);
  }
  
  // Bus error
  if(LL_I2C_IsActiveFlag_BERR(LOADSW_I2C))
  {
    LL_I2C_ClearFlag_BERR(LOADSW_I2C);
    LL_I2C_Disable(LOADSW_I2C);
  }
}

void LOADSW_EV_IRQHandler(void)
{

  uint32_t StatusRegister1 = LOADSW_I2C->SR1;
  
  // Start bit is sent
  if(StatusRegister1 & I2C_SR1_SB)
  {
    uint8_t address = (LoadSw.device == SW0) ? SW0_ADDR : SW1_ADDR;

    if(LoadSw.read < 2) LOADSW_I2C->DR = address;
    else LOADSW_I2C->DR = address | 0x01;       // Set R/W bit
  }
  
  // Address sent (Master) or Address matched (Slave)
  else if(StatusRegister1 & I2C_SR1_ADDR)
  { 
    LL_I2C_ClearFlag_ADDR(LOADSW_I2C);
    
    if(LoadSw.read < 2)
    {
      LOADSW_I2C->DR = LoadSw.regAddr;  
      if(LoadSw.read == 1)
      {
        LoadSw.read++;
        LL_I2C_GenerateStartCondition(LOADSW_I2C);
      }
    }
    else 
    {
     LL_I2C_AcknowledgeNextData(LOADSW_I2C, LL_I2C_ACK);
     LoadSw.index = 1;
    }
  }  
  
  // 10-bit header sent
  else if(StatusRegister1 & I2C_SR1_ADD10)
  {
    // Do nothing
  }
  
  else if(StatusRegister1 & I2C_SR1_RXNE)
  {
    if(LoadSw.index < LOADSW_NUMBER_OF_REGS)
    {
      uint8_t *ptr = (uint8_t*)&LoadSw.regs[LoadSw.device] + LoadSw.index++;
      *ptr = LOADSW_I2C->DR;
      //LoadSw.buffer[LoadSw.index++] = LOADSW_I2C->DR;
      //LL_I2C_GenerateStopCondition(LOADSW_I2C);
    }
    if(LoadSw.index == LOADSW_NUMBER_OF_REGS - 1)
    {
      LL_I2C_AcknowledgeNextData(LOADSW_I2C, LL_I2C_NACK);
      LL_I2C_GenerateStopCondition(LOADSW_I2C);
    }
  }
  
  else if(StatusRegister1 & I2C_SR1_TXE)
  {
    if(LoadSw.index < LOADSW_NUMBER_OF_REGS) 
    {
      LOADSW_I2C->DR = LoadSw.buffer[LoadSw.device][LoadSw.index++];
    }
    else
    {
      LL_I2C_GenerateStopCondition(LOADSW_I2C);
    }  
  }
  
  // Data byte transfer finished
  else if(StatusRegister1 & I2C_SR1_BTF)
  {
   
  }
  
  // Stop Received (Slave)
  else if(StatusRegister1 & I2C_SR1_STOPF)
  {
    LL_I2C_DisableIT_EVT(LOADSW_I2C);
    LL_I2C_DisableIT_ERR(LOADSW_I2C);
    LL_I2C_DisableIT_BUF(LOADSW_I2C);
    LL_I2C_Disable(LOADSW_I2C);  
  } 

}