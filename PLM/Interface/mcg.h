/**************************************************************************
 * Filename: mcg.h
 *
 * Description: MCG driver header file for ARM CORTEX-M4 processor
 *
 * Copyright (c) 2012, Freescale Semiconductor, Inc. All rights reserved.
 *
 ***************************************************************************
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
 ***************************************************************************/
#ifndef __MCG_H__
#define __MCG_H__

#include "EmbeddedTypes.h"
#include "PortConfig.h"

/******************************************************************************
 *******************************************************************************
 * Public macros
 *******************************************************************************
 ******************************************************************************/

/******************************************************************************
 *******************************************************************************
 * Public type definitions
 *******************************************************************************
 ******************************************************************************/

/******************************************************************************
 *******************************************************************************
 * Public memory definitions
 *******************************************************************************
 ******************************************************************************/

extern int gMCG_coreClkKHz;
extern int gMCG_coreClkMHz;
extern int gMCG_periphClkKHz;

/*****************************************************************************
 ******************************************************************************
 * Public prototypes
 ******************************************************************************
 *****************************************************************************/

/******************************************************************************
 * Name: MCG_PLLInit
 * Description: Initialize the PLL module according to clock option and crystal
 *              value
 * Parameters: [IN] clk_option - clock option (PLL50, PLL100, PLL96, PLL48)
 *             [IN] crystal_val - crystal value (XTAL2, XTAL4, XTAL6, XTAL10,
 *                                XTAL12, XTAL14, XTAL16, XTAL18, XTAL20, 
 *                                XTAL22, XTAL24, XTAL26, XTAL28, XTAL30,
 *                                XTAL32
 *                                
 * Notes: It is assumed that the MCG is in default FEI mode out of reset.                                
 * Return: PLL frequency (MHz)
 ******************************************************************************/ 
extern uint8_t MCG_PLLInit
(
        void
);

/******************************************************************************
 * Name: MCG_Pee2Blpi
 * Description: Changes the MCG operation mode from PEE to BLPI
 *              Transition chain from PEE to BLPI: PEE -> PBE -> FBE -> 
 *              FBI -> BLPI
 * Parameters: -
 * Return: -
 ******************************************************************************/ 
extern void MCG_Pee2Blpi
(
        void
);

/******************************************************************************
 * Name: MCG_Blpi2Pee
 * Description: Changes the MCG operation mode from BLPI to PEE
 *              Transition from BLPI to PEE: BLPI -> FBI -> FEI -> FBE -> 
 *              PBE -> PEE
 * Parameters: -
 * Return: -
 ******************************************************************************/
extern void MCG_Blpi2Pee
(
        void
);

/******************************************************************************
 * Name: MCG_Pbe2Pee
 * Description: Changes the MCG operation mode from PBE to PEE
 * Parameters: -
 * Return: -
 ******************************************************************************/
extern void MCG_Pbe2Pee
(
        void
);

/******************************************************************************
 * Name: MCG_Fei2Fee
 * Description: Changes the MCG operation mode from FEI to FEE
 * Parameters: -
 * Return: FEE running frequency (MHz)
 ******************************************************************************/
extern uint8_t MCG_Fei2Fee
(
      void
);

/******************************************************************************
 * Name: MCG_TraceSysClk
 * Description: Trace the system clock (divided by 1000) using the FTM2 module
 * Parameters: -
 * Return: -
 ******************************************************************************/
extern void MCG_TraceSysClk
(
    void
);

/******************************************************************************
 * Name: MCG_Fee2Fei
 * Description: Changes the MCG operation mode from FEE to FEI
 * Parameters: -
 * Return: -
 ******************************************************************************/
void MCG_Fee2Fei
(
    void
);

/******************************************************************************
 * Name: MCG_Pee2Fei
 * Description: Changes the MCG operation mode from PEE to FEI
 * Parameters: -
 * Return: -
 ******************************************************************************/
void MCG_Pee2Fei
(
    void
);

#endif /* __MCG_H__ */

/********************************** EOF ***************************************/
