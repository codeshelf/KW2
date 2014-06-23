/******************************************************************************
 * Source file for Keyboard driver.
 * 
 * Freescale Semiconductor Inc.
 * (c) Copyright 2004-2011 Freescale Semiconductor, Inc.
 * ALL RIGHTS RESERVED.
 *
*******************************************************************************
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
***************************************************************************//*!
******************************************************************************/

#include "Keyboard.h"

/******************************************************************************
 *******************************************************************************
 * Private macros
 *******************************************************************************
 ******************************************************************************/

/******************************************************************************
 *******************************************************************************
 * Private type definitions
 *******************************************************************************
 ******************************************************************************/

/******************************************************************************
 *******************************************************************************
 * Public memory definitions
 *******************************************************************************
 ******************************************************************************/



/******************************************************************************
 *******************************************************************************
 * Private memory definitions
 *******************************************************************************
 ******************************************************************************/

/******************************************************************************
 *******************************************************************************
 * Private function prototypes
 *******************************************************************************
 ******************************************************************************/

void KeyboardInit (void)
{
  gKeyboard_ClkGatingReg_c |= gKeyboard_ClkGatingBit_c;
  gKeyboard_Switch1PinCtrlReg_c =  PORT_PCR_PS_MASK | PORT_PCR_PE_MASK | PORT_PCR_MUX(gSwitch_PinCtrlAltFunc_c) | PORT_PCR_IRQC(10);
  gKeyboard_Switch2PinCtrlReg_c =  PORT_PCR_PS_MASK | PORT_PCR_PE_MASK | PORT_PCR_MUX(gSwitch_PinCtrlAltFunc_c) | PORT_PCR_IRQC(10);
  gKeyboard_Switch3PinCtrlReg_c =  PORT_PCR_PS_MASK | PORT_PCR_PE_MASK | PORT_PCR_MUX(gSwitch_PinCtrlAltFunc_c) | PORT_PCR_IRQC(10);
  gKeyboard_Switch4PinCtrlReg_c =  PORT_PCR_PS_MASK | PORT_PCR_PE_MASK | PORT_PCR_MUX(gSwitch_PinCtrlAltFunc_c) | PORT_PCR_IRQC(10);
  NVIC_EnableIRQ(gKeyboard_Irq_Number);

}

/******************************************************************************
 *******************************************************************************
 * Private type definitions
 *******************************************************************************
 ******************************************************************************/

/******************************************************************************
 *******************************************************************************
 * Private Memory Declarations
 *******************************************************************************
 ******************************************************************************/


/******************************************************************************
 ******************************************************************************
 * Public functions
 ******************************************************************************
 *****************************************************************************/

 /******************************************************************************/
void Switch_Press_ISR(void)
{
   if (PORT_PCR_ISF_MASK & gKeyboard_Switch1PinCtrlReg_c)
   {     
     Led4Toggle();      
   }  
   if (gKeyboard_Switch2PinCtrlReg_c & PORT_PCR_ISF_MASK)
   {    
     Led3Toggle();      
   }  
    if (gKeyboard_Switch3PinCtrlReg_c & PORT_PCR_ISF_MASK)
   {    
     Led2Toggle();      
   }  
    if (gKeyboard_Switch4PinCtrlReg_c & PORT_PCR_ISF_MASK)
   {    
     Led1Toggle();      
   }  
   
   PORTC_ISFR |= gLED_Led1Pin_c | gLED_Led2Pin_c | gLED_Led3Pin_c | gLED_Led4Pin_c; 
}

