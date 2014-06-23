/******************************************************************************
 * Filename: NVIC.h
 *
 * Description: Header file for NVIC minimal functions set for ARM CORTEX-M4 
 *              processor
 *
 * Copyright (c) 2012, Freescale Semiconductor, Inc. All rights reserved.
 *
 ******************************************************************************
 *
 * THIS SOFTWARE IS PROVIDED BY FREESCALE "AS IS" AND ANY EXPRESSED OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL FREESCALE OR ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 *
 *****************************************************************************/

#ifndef _NVIC_H_
#define _NVIC_H_

#include "EmbeddedTypes.h"

/******************************************************************************
 ******************************************************************************
 * Public macros
 ******************************************************************************
 ******************************************************************************/

/*
 * Name: gNvicInterruptLevelBits_c
 * Description: The number of bits used to store the interrupt priority
 */
#define gNvicInterruptLevelBits_c       4

/* 
 * Name: gNvicMaxIrqValue_c
 * Description: Maximum IRQ number 
 */
#define gNvicMaxIrqValue_c              91

/* 
 * Name: gNvicMaxPrioValue_c
 * Description: Maximum value for IRQ priority 
 */
#define gNvicMaxPrioValue_c		        15

/*
 * Name: IntEnableAll
 * Description: Macro to enable all individual interrupts
 */
#define IntEnableAll()         IntRestoreAll(0)

/*
 * Name: EnableInterrupts
 * Description: Macro to enable all interrupts
 */
#define EnableInterrupts() asm(" CPSIE i");
  
/*
 * Name: DisableInterrupts
 * Description: Macro to disable all interrupts
 */
#define DisableInterrupts() asm(" CPSID i");
/***********************************************************************/

/******************************************************************************
 ******************************************************************************
 * Public functions prototypes
 ******************************************************************************
 *****************************************************************************/

/******************************************************************************
 * Name: NVIC_EnableIRQ
 * Description: Enables the specified IRQ
 * Parameters: [IN] irq - IRQ number
 * Return: TRUE if IRQ successfully enabled / FALSE otherwise
 * Note: Interrupts will also need to be enabled in the ARM core. This can be 
 *       done using the EnableInterrupts macro.
 ******************************************************************************/
extern bool_t NVIC_EnableIRQ 
(
        int irq
);

/******************************************************************************
 * Name: NVIC_DisableIRQ
 * Description: Disables the specified IRQ
 * Parameters: [IN] irq - IRQ number
 * Return: TRUE if IRQ successfully disabled / FALSE otherwise
 * Note: If you want to disable all interrupts, then use the 
 *       DisableInterrupts macro instead.
 ******************************************************************************/
extern bool_t NVIC_DisableIRQ 
(
        int irq
);

/******************************************************************************
 * Name: NVIC_SetPriority
 * Description: Sets the specified IRQ priority
 * Parameters: [IN] irq - IRQ number
 *             [IN] prio - the desired priority (0..15 levels, 0 = maximum)
 * Return: TRUE if IRQ priority successfully set / FALSE otherwise 
 ******************************************************************************/
bool_t NVIC_SetPriority 
(
        int irq,
        int prio
);

/******************************************************************************
 * Name: IntRestoreAll
 * Description: restores the interrupt state and enables interrupts
 * Parameters: [IN] if_bits - interrupt state bit mask
 * Return: -
 ******************************************************************************/
extern void IntRestoreAll
(
        uint32_t    if_bits
);

/******************************************************************************
 * Name: IntDisableAll
 * Description: disables interrupts
 * Parameters: -
 * Return: interrupt state bit mask before being disabled
 ******************************************************************************/
extern uint32_t IntDisableAll
(
        void
);

/******************************************************************************
 * Name: NVIC_ClearPendingIRQ
 * Description: Clear IRQn Pending Status 
 * Parameters: interrupt number
 * Return: -
 ******************************************************************************/
void NVIC_ClearPendingIRQ
(
        uint8_t IRQn
);

#endif /* _NVIC_H_ */
