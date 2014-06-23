/******************************************************************************
 *
 * Freescale Semiconductor Inc.
 * (c) Copyright 2004-2009 Freescale Semiconductor, Inc.
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
 * @file usb_bdt_kinetis.h
 *
 * @author 
 *
 * @version 
 *
 * @date Jun-05-2009
 *
 * @brief The file contains definitions of Buffer Descriptor Table.
 *
 *****************************************************************************/

#ifndef _USBBDT_H
#define _USBBDT_H

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "EmbeddedTypes.h"
/******************************************************************************
 * Constants - None
 *****************************************************************************/

/******************************************************************************
 * Macro's
 *****************************************************************************/
/* Buffer Descriptor Status Register Initialization Parameters */
#define gUsbBdtSCtl_Stall_d         (0x04)           /* Buffer Stall enable */
#define gUsbBdtSCtl_Data0_d         (0x00)           /* DATA0 packet expected next */
#define gUsbBdtSCtl_Data1_d         (0x40)           /* DATA1 packet expected next */
#define gUsbBdtSCtl_DTS_d           (0x08)           /* DTS Mask */
#define gUsbBdtSCtl_SIE_d           (0x80)           /* SIE owns buffer */
#define gUsbBdtSCtl_CPU_d           (0x00)           /* CPU owns buffer */
#define gUsbBdtSCtl_KEEP_d          (0x20)           /* keep bit */  
                       
#define gUsbMaxBdtIndex_d  (64)             /* Maximum BDT Indexes */


/******************************************************************************
 * Types
 *****************************************************************************/
 /* This structure is an exact replica of the BDT MAP in the USB RAM 
    The BDT_STAT defines the stat byte of the buffer descriptor vector.
    McuCtlBit structure defines the bits that have a meaning from CPU 
    point of view.SieCtlBit structure defines the bits that have a 
    meaning from USB controller point of view.      
 */
 
#if defined(__CWCC__)
#pragma align_array_members on
#elif defined(__IAR_SYSTEMS_ICC__)   
#pragma pack(1)
#endif

typedef struct mcuCtlBit_tag{
        uint8_t :1;
        uint8_t :1;
        uint8_t bdtstall:1;            /* Buffer Stall Enable */
        uint8_t dts:1;                 /* Data Toggle Synch Enable */
        uint8_t keep:1;                /* Address Increment Disable */
        uint8_t ninc:1;                /* BD Keep Enable */
        uint8_t data:1;                /* Data Toggle Synch Value */
        uint8_t own:1;                 /* USB Ownership */
}mcuCtlBit_t; /* read Stat */

typedef struct sieCtlBit_tag{
        uint8_t :1;
        uint8_t :1;
        uint8_t pid0:1;                /* Packet Identifier bit 0 */
        uint8_t pid1:1;                /* Packet Identifier bit 1 */
        uint8_t pid2:1;                /* Packet Identifier bit 2 */
        uint8_t pid3:1;                /* Packet Identifier bit 3 */
        uint8_t :1;
        uint8_t own:1;        
}sieCtlBit_t;  /* write Stat */      

typedef struct recPid_tag{
        uint8_t    :2;
        uint8_t pid:4;                 /* Packet Identifier */
        uint8_t    :2;
}recPid_t;

typedef union bdStat_tag
{
    uint8_t byte;                    
    mcuCtlBit_t mcuCtlBit;
    sieCtlBit_t sieCtlBit;
    recPid_t recPid;
} bdStat_t;                            /* Buffer Descriptor Status Register */

typedef struct buffDsc_tag
{
        bdStat_t stat;
        uint8_t  reserved1;
        uint16_t cnt;                  /* Count of bytes receieved or sent */      
                                      /* six MSB bits are reserved ones */      
        uint32_t addr;                 /* Buffer Address */
} buffDsc_t, *pBuffDsc_t;              /* Buffer Descriptor Table */
typedef struct bdtMap_tag {

  buffDsc_t epDsc[gUsbMaxBdtIndex_d];     /* Endpoint Descriptor */  
}bdtMap_t; 
#if defined(__CWCC__)
#pragma align_array_members off
#pragma options align=reset
#elif defined(__IAR_SYSTEMS_ICC__)
#pragma pack()
#endif
/******************************************************************************
 * Global Functions - None
 *****************************************************************************/

#endif 
