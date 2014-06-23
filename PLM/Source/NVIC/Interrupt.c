/******************************************************************************
 * Filename: NVIC.c
 *
 * Description: NVIC minimal functions set for ARM CORTEX-M4 processor
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

#include "EmbeddedTypes.h"
#include "PortConfig.h"
#include "Interrupt.h"

#if defined(__IAR_SYSTEMS_ICC__)
#include "intrinsics.h"
#endif

/******************************************************************************
 ******************************************************************************
 * Private macros
 ******************************************************************************
 ******************************************************************************/

/******************************************************************************
 ******************************************************************************
 * Private type definitions
 ******************************************************************************
 ******************************************************************************/

#if (defined(__GNUC__))
typedef uint32_t    __istate_t;
#endif

/******************************************************************************
 ******************************************************************************
 * Public memory definitions
 ******************************************************************************
 ******************************************************************************/

/******************************************************************************
 ******************************************************************************
 * Private function prototypes
 ******************************************************************************
 ******************************************************************************/

/******************************************************************************
 ******************************************************************************
 * Public functions
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
bool_t NVIC_EnableIRQ 
(
        int irq
)
{		
    int div;    

    if (irq > (int)(gNvicMaxIrqValue_c))
    {
        return FALSE;
    }

    div = irq/32;

    switch (div)
    {
    case 0x0:
        NVICICPR0 = 1 << (irq%32);
        NVICISER0 = 1 << (irq%32);
        break;
    case 0x1:
        NVICICPR1 = 1 << (irq%32);
        NVICISER1 = 1 << (irq%32);
        break;
    case 0x2:
        NVICICPR2 = 1 << (irq%32);
        NVICISER2 = 1 << (irq%32);
        break;
        
    default:
        return FALSE;
    }              
    
    return TRUE;
}

/******************************************************************************
 * Name: NVIC_DisableIRQ
 * Description: Disables the specified IRQ
 * Parameters: [IN] irq - IRQ number
 * Return: TRUE if IRQ successfully disabled / FALSE otherwise
 * Note: If you want to disable all interrupts, then use the 
 *       DisableInterrupts macro instead.
 ******************************************************************************/
bool_t NVIC_DisableIRQ 
(
        int irq
)
{
    int div;

    if (irq > (int)(gNvicMaxIrqValue_c))
       return FALSE;

    div = irq/32;

    switch (div)
    {
    case 0x0:
        NVICICER0 = 1 << (irq%32);
        break;
    case 0x1:
        NVICICER1 = 1 << (irq%32);
        break;
    case 0x2:
        NVICICER2 = 1 << (irq%32);
        break;
        
    default:
        return FALSE;
    } 
    
    return TRUE;
}

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
)
{    
    uint8_t *prio_reg;

    if (irq > (int)(gNvicMaxIrqValue_c))
        return FALSE;

    if (prio > (int)(gNvicMaxPrioValue_c))
        return FALSE;

    prio_reg = (uint8_t *)(((uint32_t)&NVICIP0) + irq);    
    *prio_reg = ( (prio&0xF) << (8 - gNvicInterruptLevelBits_c) );      
    
    return TRUE;
}

#if (defined(__GNUC__))
/******************************************************************************
 * Name: __set_interrupt_state
 * Description: sets the specified interrupt state
 * Parameters: [IN] bits - interrupt state bit mask
 * Return: - 
 ******************************************************************************/
void __set_interrupt_state
(
        __istate_t bits
)
{	
	asm("MSR PRIMASK, %0" : : "r" (bits));	
}

/******************************************************************************
 * Name: __get_interrupt_state
 * Description: gets the current interrupt state
 * Parameters: -
 * Return: interrupt state bit mask
 ******************************************************************************/
__istate_t __get_interrupt_state
(
        void
)
{	
	__istate_t tmp=0;
	asm("mrs %0,PRIMASK" : "=r" (tmp));
	return tmp;
}
#endif

/******************************************************************************
 * Name: IntRestoreAll
 * Description: restores the interrupt state and enables interrupts
 * Parameters: [IN] if_bits - interrupt state bit mask
 * Return: -
 ******************************************************************************/
void IntRestoreAll
(
        uint32_t    if_bits
)
{ 
#if (defined(__GNUC__))
//	if_bits = if_bits;
//	EnableInterrupts();
	__set_interrupt_state((__istate_t)if_bits);	
#elif defined(__IAR_SYSTEMS_ICC__)
    __set_interrupt_state((__istate_t)if_bits);
#else
#warning "Unknown / unsupported toolchain"
#endif
}

/******************************************************************************
 * Name: IntDisableAll
 * Description: disables interrupts
 * Parameters: -
 * Return: interrupt state bit mask before being disabled
 ******************************************************************************/
uint32_t IntDisableAll
(
        void
)
{
#if defined(__IAR_SYSTEMS_ICC__)
    __istate_t istate = 0; 
    istate = __get_interrupt_state();
    DisableInterrupts();
    return (unsigned int)istate;
#elif defined(__GNUC__)
    __istate_t istate = 0; 
    istate = __get_interrupt_state();    
    DisableInterrupts();
    return (unsigned int)istate;
#else
#warning "Unknown / unsupported toolchain"
    return 0;
#endif
}

/******************************************************************************
 * Name: NVIC_ClearPendingIRQ
 * Description: Clear IRQn Pending Status 
 * Parameters: interrupt number
 * Return: -
 ******************************************************************************/
void NVIC_ClearPendingIRQ(uint8_t IRQn)
{
  NVIC_ICPR(((uint32_t)(IRQn) >> 5)) = (1 << ((uint32_t)(IRQn) & 0x1F)); /* Clear pending interrupt */
}
