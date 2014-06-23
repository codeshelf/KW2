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
 * @file usb_sim_settings.h
 *
 * @author
 *
 * @version
 *
 * @date
 *
 * @brief   This software is the USB driver stack for S08 family
 *****************************************************************************/
#ifndef _USB_SIM_SETTINGS_H
#define _USB_SIM_SETTINGS_H

/******************************************************************************
 * Macro's
 *****************************************************************************/
#if defined (MCU_MK21DN512) || defined (MCU_MK21DX256)
    #define gUSBOTG_IntNo_c   (53)
#elif (defined(MCU_MK60N512VMD100) || defined(MCU_MK60D10))
    #define gUSBOTG_IntNo_c   (73)
#endif

/*****************************************************************************
 * Global Functions Prototypes
 *****************************************************************************/
extern void USB_SIM_Settings(void);
extern void USB_SIM_UsbIntEnable(void);
extern void USB_SIM_UsbIntDisable(void);
#endif// _USB_SIM_SETTINGS_H
/* EOF */
