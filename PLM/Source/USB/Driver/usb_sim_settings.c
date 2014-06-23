/******************************************************************************
 *
 * Freescale Semiconductor Inc.
 * (c) Copyright 2004-2010 Freescale Semiconductor, Inc.
 * ALL RIGHTS RESERVED.
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
 **************************************************************************//*!
 *
 * @file main_kinetis.c
 *
 * @author
 *
 * @version
 *
 * @date
 *
 * @brief   This software is the USB driver stack for S08 family
 *****************************************************************************/
#include "EmbeddedTypes.h"
#include "PortConfig.h"
#include "USB_Interface.h"
#include "usb_sim_settings.h"
#include "SMAC.h"

/*****************************************************************************
 * Global Functions Prototypes
 *****************************************************************************/




#if gUsbIncluded_d
/*****************************************************************************
 *
 *    @name     USB_SIM_Settings
 *
 *    @brief    This function Initializes USB module
 *
 *    @param    None
 *
 *    @return   None
 *
 ***************************************************************************/
void USB_SIM_Settings(void)
{

  SIM_CLKDIV2 &= (uint32_t)(~(SIM_CLKDIV2_USBFRAC_MASK | SIM_CLKDIV2_USBDIV_MASK));
  /* Configure USBFRAC = 0, USBDIV = 0 => frq(USBout) = 1 / 1 * frq(PLLin) */
  SIM_CLKDIV2 = SIM_CLKDIV2_USBDIV(0);
  /*  Configure USB to be clocked from PLL */
  SIM_SOPT2 |= SIM_SOPT2_USBSRC_MASK | SIM_SOPT2_PLLFLLSEL_MASK;
  /*  Enable USB-OTG IP clocking */
  SIM_SCGC4 |= (SIM_SCGC4_USBOTG_MASK);
  /* old documentation writes setting this bit is mandatory */
  USB0_USBTRC0 = 0x40;
  SIM_SOPT1CFG |= SIM_SOPT1CFG_URWE_MASK;
  /* Configure enable USB regulator for device */
  SIM_SOPT1 |= SIM_SOPT1_USBREGEN_MASK;	// enable usb voltage regulator
  NVIC_ClearPendingIRQ(gUSBOTG_IntNo_c);
  NVIC_EnableIRQ(gUSBOTG_IntNo_c);
}

/*****************************************************************************
 *
 *    @name     USB_SIM_UsbIntEnable
 *
 *    @brief    This function Initializes USB module
 *
 *    @param    None
 *
 *    @return   None
 *
 ***************************************************************************/
void USB_SIM_UsbIntEnable(void)
{
uint8_t iserIndex;
uint32_t iserIntVal;
iserIndex = gUSBOTG_IntNo_c/32;
iserIntVal = 1<<(gUSBOTG_IntNo_c%32);
NVIC_ISER_REG(NVIC_BASE_PTR,iserIndex) = iserIntVal;
}
/*****************************************************************************
 *
 *    @name     USB_SIM_UsbIntDisable
 *
 *    @brief    This function Initializes USB module
 *
 *    @param    None
 *
 *    @return   None
 *
 ***************************************************************************/
void USB_SIM_UsbIntDisable(void)
{
uint8_t icerIndex;
uint32_t icerIntVal;
icerIndex = gUSBOTG_IntNo_c/32;
icerIntVal = 1<<(gUSBOTG_IntNo_c%32);
NVIC_ICER_REG(NVIC_BASE_PTR,icerIndex) = icerIntVal;
}
#endif
/* EOF */
