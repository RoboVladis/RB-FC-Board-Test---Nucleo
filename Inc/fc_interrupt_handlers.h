/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __FC_INTERRUPT_HANDLERS_H
#define __FC_INTERRUPT_HANDLERS_H

#ifdef __cplusplus
 extern "C" {
#endif 

#include "stm32f4xx.h"
#include "stm32f4xx_ll_system.h"
#include "stm32f4xx_ll_gpio.h"
#include "stm32f4xx_ll_exti.h"
#include "fc_board.h"

void NMI_Handler(void);

void HardFault_Handler(void);

void MemManage_Handler(void);

void BusFault_Handler(void);

void UsageFault_Handler(void);

void SVC_Handler(void);

void DebugMon_Handler(void);

void PendSV_Handler(void);

#ifdef __cplusplus
}
#endif

#endif /* __FC_INTERRUPT_HANDLERS_H */

