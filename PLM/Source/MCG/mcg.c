/******************************************************************************
 * Filename: mcg.c
 *
 * Description: MCG driver for ARM CORTEX-M4 processor
 * Notes: - Assumes the MCG mode is in the default FEI mode out of reset
 *        - One of 4 clocking options can be selected.
 *        - One of 16 crystal values can be used
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
#include "Utilities_Interface.h"
#include "mcg.h"


/******************************************************************************
 ******************************************************************************
 * Private macros
 ******************************************************************************
 ******************************************************************************/
/*
 * Name: MCG_SystemClkMhz_c
 * Description: System clock frequency (MHz)
 */
#define MCG_SystemClkMhz_c        48

/******************************************************************************
 ******************************************************************************
 * Private type definitions
 ******************************************************************************
 ******************************************************************************/

/******************************************************************************
 ******************************************************************************
 * Private memory definitions
 ******************************************************************************
 ******************************************************************************/

/******************************************************************************
 *******************************************************************************
 * Private function prototypes
 *******************************************************************************
 ******************************************************************************/


static void MCG_SetSysDividers
(
        uint32_t,
        uint32_t,
        uint32_t,
        uint32_t
);

/******************************************************************************
 ******************************************************************************
 * Public memory definitions
 ******************************************************************************
 ******************************************************************************/

/*
 * Name: gMCG_coreClkKHz (KHz units)
 * Description: Actual core clock frequency 
 */
int gMCG_coreClkKHz;

/*
 * Name: gMCG_coreClkMHz
 * Description: Actual core clock frequency (MHz units)
 */
int gMCG_coreClkMHz;

/*
 * Name: gMCG_periphClkKHz
 * Description: Actual peripheral (bus) clock frequency (KHz units) 
 */
int gMCG_periphClkKHz;

/******************************************************************************
 ******************************************************************************
 * Public functions
 ******************************************************************************
 *****************************************************************************/

/******************************************************************************
 * Name: MCG_PLLInit
 * Description: Initialize the PLL module to operate with a 4 MHz external 
 *              reference clock (if MCU is clocked by the radio) or with a
 *              50MHz external reference clock (if MCU is clocked by the TWR 
 *              on-board clock oscillator). The PLL output frequency will be 
 *              48 MHz in both situations.
 *              MCGCLKOUT = PLL output frequency
 * Parameters: -
 *
 * Notes: It is assumed that the MCG is in default FEI mode out of reset.
 * Return: PLL frequency (MHz)
 ******************************************************************************/
uint8_t MCG_PLLInit
(
        void
)
{

#if (defined(__GNUC__))
    /*
     * Define a pointer to function that will points to the RAM copy
     * of MCG_SetSysDividers() function
     */
    void (*pfSetSysDividers)(uint32_t, uint32_t, uint32_t, uint32_t);

    uint32_t fcn_thumb_flag;
    uint32_t fcn_rom_addr;
    uint32_t fcn_ram_addr;

    /*
     * Allocate stack space for MCG_SetSysDividers() function copy
     * The sizeof(fcn_ram_copy) must be enough to
     * fit whole MCG_SetSysDividers() function.
     */
    uint32_t fcn_ram_copy[128];

    /*
     * Copy MCG_SetSysDividers() function to stack @ fcn_ram_addr address
     * Because Thumb-2 instruction mode is used its necessary to set
     * bit[0] correctly to represent the opcode type of the branch target.
     * For details see:
     * http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.faqs/ka12545.html
     */

    fcn_thumb_flag = (uint32_t)MCG_SetSysDividers & 0x01;
    fcn_rom_addr = (uint32_t)MCG_SetSysDividers & ~(uint32_t)0x01;
    fcn_ram_addr = (uint32_t)fcn_ram_copy | (fcn_rom_addr & 0x02);

    FLib_MemCpyAligned32bit((void*)fcn_rom_addr, (void*)fcn_ram_addr, sizeof(fcn_ram_copy)-3);

    /* Get pointer of MCG_SetSysDividers function to run_in_ram_fcn  */
    pfSetSysDividers = (void (*)(uint32_t, uint32_t, uint32_t, uint32_t)) (fcn_ram_addr | fcn_thumb_flag);    
#endif

    /* Load slow internal reference clock (IRC) trim values, if any */
    if ( *((uint8_t*) 0x03FFU) != 0xFFU) 
    {
        MCG_C3 = *((uint8_t*) 0x03FFU);
        MCG_C4 = (MCG_C4 & 0xE0U) | ((*((uint8_t*) 0x03FEU)) & 0x1FU);
    }

    /************************ 
     * Transition: FEI->FBE 
     ************************/
#if defined(MCU_MK60N512VMD100) || defined(MCU_MK60D10)
    MCG_C2 = 0;
#elif (defined(MCU_MK20D5) || defined(MCU_MK21DN512) || defined(MCU_MK21DX256))
    //MCG_C2 = MCG_C2_RANGE0(2) | MCG_C2_HGO0_MASK | MCG_C2_EREFS0_MASK | MCG_C2_IRCS_MASK;
    MCG_C2 = MCG_C2_RANGE0(2) | MCG_C2_HGO0_MASK | MCG_C2_IRCS_MASK;
#else
    //MCG_C2 = MCG_C2_RANGE(2) | MCG_C2_HGO_MASK | MCG_C2_EREFS_MASK | MCG_C2_IRCS_MASK;
    MCG_C2 = MCG_C2_RANGE0(2) | MCG_C2_HGO0_MASK | MCG_C2_IRCS_MASK;
#endif

    /* Select external oscillator and Reference Divider and clear IREFS 
     * to start external oscillator
     * CLKS = 2, FRDIV = 3, IREFS = 0, IRCLKEN = 0, IREFSTEN = 0
     */
#if (defined(MCU_MK20D5) || defined(MCU_MK21DN512) || defined(MCU_MK21DX256))
    MCG_C1 = MCG_C1_CLKS(2) | MCG_C1_FRDIV(3);
#elif (defined(MCU_MK60N512VMD100) || defined(MCU_MK60D10))
    MCG_C1 = MCG_C1_CLKS(2);
#endif

#if (defined(MCU_MK20D5))
    /* Wait for oscillator to initialize */
    while (!(MCG_S & MCG_S_OSCINIT0_MASK)){};
#endif

#if (!defined(MCU_MK20D5))
    /* wait for Reference clock Status bit to clear */
    while (MCG_S & MCG_S_IREFST_MASK) {};
    /* The source of FLL reference clock is the external reference clock */
#endif

    /* Wait for clock status bits to show that clock source is External Reference Clock */
    while (((MCG_S & MCG_S_CLKST_MASK) >> MCG_S_CLKST_SHIFT) != 0x2) {};

    /************************ 
     * Transition: FBE->PBE 
     ************************/    
#if (defined(MCU_MK60N512VMD100))    
    MCG_C5 = MCG_C5_PRDIV(24); /* divide by 25 the external reference clock (50MHz / 25 = 2 MHz) used by PLL */
#elif (defined(MCU_MK60D10))
    MCG_C5 = MCG_C5_PRDIV0(24); /* divide by 25 the external reference clock (50MHz / 25 = 2 MHz) used by PLL */
#elif (defined(MCU_MK20D5))
    MCG_C5 = MCG_C5_PRDIV0(3) | MCG_C5_PLLCLKEN0_MASK;  /* divide by 4 the external reference clock (8MHz /4 = 2MHz) used by PLL */
#elif (defined(MCU_MK21DN512) || defined(MCU_MK21DX256))
    MCG_C5 = MCG_C5_PRDIV0(1) | MCG_C5_PLLCLKEN0_MASK;  /* divide by 2 the external reference clock (4MHz /2 = 2MHz) used by PLL */
#else
    MCG_C5 = MCG_C5_PRDIV(1);  /* divide by 2 the external reference clock */
#endif

    /* Ensure MCG_C6 is at the reset default of 0 */
    MCG_C6 = 0x0;

    /* Set system options dividers
     * MCG = PLL, core = MCG, bus = MCG, FlexBus = MCG, Flash clock= MCG/2
     */
#if (defined(__GNUC__))
    pfSetSysDividers(0,0,0,1);
#elif (defined(__IAR_SYSTEMS_ICC__))
    MCG_SetSysDividers(0,0,0,1);
#endif

#if !(defined(MCU_MK60N512VMD100))
    if(MCG_SC & MCG_SC_LOCS0_MASK)
    {
      MCG_SC |= MCG_SC_LOCS0_MASK;
    }
#endif

    /* Set the VCO divider and enable the PLL for 48MHz operation */
#if (defined(MCU_MK60N512VMD100))   
    MCG_C6 = MCG_C6_PLLS_MASK | MCG_C6_VDIV(0); /* VDIV = 0 (x24), Clock Monitor enabled */
#elif(defined(MCU_MK20D5) || defined(MCU_MK60D10) || defined(MCU_MK21DN512) || defined (MCU_MK21DX256))
    MCG_C6 = MCG_C6_PLLS_MASK | MCG_C6_VDIV0(0); /* VDIV = 0 (x24), Clock Monitor enabled */
#endif

    /* wait for PLL status bit to set */
    while (!(MCG_S & MCG_S_PLLST_MASK)) {};

    /* Wait for LOCK bit to set */
#if (defined (MCU_MK20D5) || defined(MCU_MK60D10) || defined(MCU_MK21DN512) || defined(MCU_MK21DX256))
    while (!(MCG_S & MCG_S_LOCK0_MASK)){};  
#else    
    while (!(MCG_S & MCG_S_LOCK_MASK)) {};
#endif

    /************************ 
     * Transition: PBE->PEE 
     ************************/       

    /* Transition into PEE by setting CLKS to 0
     * CLKS=0, FRDIV=3, IREFS=0, IRCLKEN=0, IREFSTEN=0
     */
    MCG_C1 &= ~MCG_C1_CLKS_MASK;

    /* Wait for clock status bits to update */
    while (((MCG_S & MCG_S_CLKST_MASK) >> MCG_S_CLKST_SHIFT) != 0x3) {};

#if(defined(MCU_MK20D5))
    /* Enable the ER clock of oscillators */
    OSC0_CR = OSC_CR_ERCLKEN_MASK | OSC_CR_EREFSTEN_MASK;
#elif(defined(MCU_MK60N512VMD100) || defined(MCU_MK60D10) || defined(MCU_MK21DN512) || defined(MCU_MK21DX256))
    /* Enable the ER clock of oscillators */
    OSC_CR = OSC_CR_ERCLKEN_MASK | OSC_CR_EREFSTEN_MASK;    
#endif

    /* Enable the clock monitor */    
#if (defined(MCU_MK60N512VMD100))   
    MCG_C6 |= MCG_C6_CME_MASK;
#elif(defined(MCU_MK20D5) || defined(MCU_MK60D10) || defined(MCU_MK21DN512) || defined(MCU_MK21DX256))
    MCG_C6 |= MCG_C6_CME0_MASK;
#endif    

    /* Now running PEE Mode */
    return (uint8_t)(MCG_SystemClkMhz_c);
}

/******************************************************************************
 * Name: MCG_SetSysDividers
 * Description: Initialize the PLL module according to clock option and crystal
 *              value
 * Parameters: [IN] outdivX - system dividers values
 *
 * Notes: This routine must be placed in RAM. It is a workaround for Errata e2448.
 *        Flash prefetch must be disabled when the flash clock divider is changed.
 *        This cannot be performed while executing out of flash.
 *        There must be a short delay after the clock dividers are changed before
 *        prefetch can be re-enabled.
 * Return: -
 ******************************************************************************/
#if (defined(__IAR_SYSTEMS_ICC__))
__ramfunc void MCG_SetSysDividers
(
        uint32_t outdiv1,
        uint32_t outdiv2,
        uint32_t outdiv3,
        uint32_t outdiv4
)
#elif (defined(__GNUC__))
static void MCG_SetSysDividers
(
        uint32_t outdiv1,
        uint32_t outdiv2,
        uint32_t outdiv3,
        uint32_t outdiv4
)
#endif
{
    uint32_t temp_reg;
    uint8_t i;

    /* store present value of FMC_PFAPR */
    temp_reg = FMC_PFAPR;

#if (defined(MCU_MK60N512VMD100) || defined(MCU_MK60D10))    
    /* set M0PFD through M7PFD to 1 to disable prefetch */
    FMC_PFAPR |= FMC_PFAPR_M7PFD_MASK | FMC_PFAPR_M6PFD_MASK | FMC_PFAPR_M5PFD_MASK
            | FMC_PFAPR_M4PFD_MASK | FMC_PFAPR_M3PFD_MASK | FMC_PFAPR_M2PFD_MASK
            | FMC_PFAPR_M1PFD_MASK | FMC_PFAPR_M0PFD_MASK;
#elif (defined(MCU_MK20D5) || defined(MCU_MK21DN512) || defined(MCU_MK21DX256))
    /* set M0PFD through M3PFD to 1 to disable prefetch */
    FMC_PFAPR |= FMC_PFAPR_M3PFD_MASK | FMC_PFAPR_M2PFD_MASK
            | FMC_PFAPR_M1PFD_MASK | FMC_PFAPR_M0PFD_MASK;
#endif

    /* set clock dividers to desired value */
#if (defined(MCU_MK60N512VMD100) || defined(MCU_MK60D10))  
    SIM_CLKDIV1 = SIM_CLKDIV1_OUTDIV1(outdiv1) | SIM_CLKDIV1_OUTDIV2(outdiv2)
                          | SIM_CLKDIV1_OUTDIV3(outdiv3) | SIM_CLKDIV1_OUTDIV4(outdiv4);
#elif (defined (MCU_MK20D5) || defined(MCU_MK21DN512) || defined(MCU_MK21DX256))
    SIM_CLKDIV1 = SIM_CLKDIV1_OUTDIV1(outdiv1) | SIM_CLKDIV1_OUTDIV2(outdiv2)
                          | SIM_CLKDIV1_OUTDIV4(outdiv4);
#endif

    /* wait for dividers to change */
    for (i = 0 ; i < outdiv4 ; i++)
    {}    

    /* re-store original value of FMC_PFAPR */
    FMC_PFAPR = temp_reg;

    return;
}


/******************************************************************************
 * Name: MCG_Pee2Blpi
 * Description: Changes the MCG operation mode from PEE to BLPI
 *              Transition chain from PEE to BLPI: PEE -> PBE -> FBE ->
 *              FBI -> BLPI
 * Parameters: -
 * Return: -
 ******************************************************************************/
void MCG_Pee2Blpi
(
        void
)
{
    uint8_t temp_reg;

#if (defined(MCU_MK60N512VMD100) || defined (MCU_MK20D5) || defined(MCU_MK60D10)  || defined(MCU_MK21DN512) || defined(MCU_MK21DX256))

    /* Step 1: PEE -> PBE */
    MCG_C1 |= MCG_C1_CLKS(2);  /* System clock from external reference OSC, not PLL. */

    /* Wait for clock status to update */
    while (((MCG_S & MCG_S_CLKST_MASK) >> MCG_S_CLKST_SHIFT) != 0x2) {};

    /* Step 2: PBE -> FBE */
    MCG_C6 &= ~MCG_C6_PLLS_MASK;  /* Clear PLLS to select FLL, still running system from EXT OSC */
    while (MCG_S & MCG_S_PLLST_MASK) {}; /* Wait for PLL status flag to reflect FLL selected */

    /* Step 3: FBE -> FBI */
    MCG_C2 &= ~MCG_C2_LP_MASK;  /* FLL remains active in bypassed modes. */
    MCG_C2 |= MCG_C2_IRCS_MASK;  /* Select fast (1MHz) internal reference */
    temp_reg = MCG_C1;
    temp_reg &= ~(MCG_C1_CLKS_MASK | MCG_C1_IREFS_MASK);
    temp_reg |= (MCG_C1_CLKS(1) | MCG_C1_IREFS_MASK);  /* Select internal reference (fast IREF clock @ 1MHz) as MCG clock source. */
    MCG_C1 = temp_reg;

    while (MCG_S & MCG_S_IREFST_MASK) {}; /* Wait for Reference Status bit to update. */
    while (((MCG_S & MCG_S_CLKST_MASK) >> MCG_S_CLKST_SHIFT) != 0x1) {}; /* Wait for clock status bits to update */

    /* Step 4: FBI -> BLPI */
    MCG_C1 |= MCG_C1_IREFSTEN_MASK;  /* Keep internal reference clock running in STOP modes */
    MCG_C2 |= MCG_C2_LP_MASK;  /* FLL remains disabled in bypassed modes */
    while (!(MCG_S & MCG_S_IREFST_MASK)) {}; /* Wait for Reference Status bit to update */
    while (((MCG_S & MCG_S_CLKST_MASK) >> MCG_S_CLKST_SHIFT) != 0x1) {}; /* Wait for clock status bits to update */

#endif

}

/******************************************************************************
 * Name: MCG_Blpi2Pee
 * Description: Changes the MCG operation mode from BLPI to PEE
 *              Transition from BLPI to PEE: BLPI -> FBI -> FEI -> FBE ->
 *              PBE -> PEE
 * Parameters: -
 * Return: -
 ******************************************************************************/
void MCG_Blpi2Pee
(
        void
)
{
    uint8_t temp_reg;

#if (defined(MCU_MK60N512VMD100) || defined (MCU_MK20D5) || defined(MCU_MK60D10) || defined(MCU_MK21DN512) || defined(MCU_MK21DX256))

    /* Step 1: BLPI -> FBI */
    MCG_C2 &= ~MCG_C2_LP_MASK;  /* FLL remains active in bypassed modes */
    while (!(MCG_S & MCG_S_IREFST_MASK)) {}; /* Wait for Reference Status bit to update */
    while (((MCG_S & MCG_S_CLKST_MASK) >> MCG_S_CLKST_SHIFT) != 0x1) {}; /* Wait for clock status bits to update */

    /* Step 2: FBI -> FEI */
    MCG_C2 &= ~MCG_C2_LP_MASK;  /* FLL remains active in bypassed modes */
    temp_reg = MCG_C2;  /* assign temporary variable of MCG_C2 contents */
#if (defined(MCU_MK60N512VMD100))
    temp_reg &= ~MCG_C2_RANGE_MASK;  /* set RANGE field location to zero */
#elif (defined (MCU_MK20D5) || defined(MCU_MK60D10) || defined(MCU_MK21DN512) || defined(MCU_MK21DX256))
    temp_reg &= ~MCG_C2_RANGE0_MASK;  /* set RANGE field location to zero */
#endif
    temp_reg |= (0x2 << 0x4);  /* OR in new values */
    MCG_C2 = temp_reg;  /* store new value in MCG_C2 */
    MCG_C4 = 0x0E;  /* Low-range DCO output (~10MHz bus).  FCTRIM=%0111 */
    MCG_C1 = 0x04;  /* Select internal clock as MCG source, FRDIV=%000, internal reference selected */

    /* Wait for Reference Status bit to update */
    while (!(MCG_S & MCG_S_IREFST_MASK)) {};

    /* Wait for clock status bits to update */
    while (((MCG_S & MCG_S_CLKST_MASK) >> MCG_S_CLKST_SHIFT) != 0x0) {};

    /* Handle FEI to PEE transitions using standard clock initialization routine */
    gMCG_coreClkMHz = MCG_PLLInit();

    /* Use the value obtained from the MCG_PLLInit function to define variables
     * for the core clock in kHz and also the peripheral clock. These
     * variables can be used by other functions that need awareness of the
     * system frequency.
     */

    gMCG_coreClkKHz = gMCG_coreClkMHz * 1000;
    gMCG_periphClkKHz = gMCG_coreClkKHz / (((SIM_CLKDIV1 & SIM_CLKDIV1_OUTDIV2_MASK) >> 24)+ 1);

#endif
}

/******************************************************************************
 * Name: MCG_Pbe2Pee
 * Description: Changes the MCG operation mode from PBE to PEE
 * Parameters: -
 * Return: -
 ******************************************************************************/
void MCG_Pbe2Pee
(
        void
)
{

    /* select PLL as MCG_OUT */
    MCG_C1 &= ~MCG_C1_CLKS_MASK;

    /* Wait for clock status bits to update */
    while (((MCG_S & MCG_S_CLKST_MASK) >> MCG_S_CLKST_SHIFT) != 0x3) {};
}


/******************************************************************************
 * Name: MCG_Fei2Fee
 * Description: Changes the MCG operation mode from FEI to FEE
 * Parameters: -
 * Return: FEE running frequency (MHz)
 ******************************************************************************/
uint8_t MCG_Fei2Fee
(
        void
)
{
    const uint8_t fll_freq = 48;

#if (defined(__GNUC__))
    /*
     * Define a pointer to function that will points to the RAM copy
     * of MCG_SetSysDividers() function
     */
    void (*pfSetSysDividers)(uint32_t, uint32_t, uint32_t, uint32_t);

    uint32_t fcn_thumb_flag;
    uint32_t fcn_rom_addr;
    uint32_t fcn_ram_addr;

    /*
     * Allocate stack space for MCG_SetSysDividers() function copy
     * The sizeof(fcn_ram_copy) must be enough to
     * fit whole MCG_SetSysDividers() function.
     */
    uint32_t fcn_ram_copy[128];

    /*
     * Copy MCG_SetSysDividers() function to stack @ fcn_ram_addr address
     * Because Thumb-2 instruction mode is used its necessary to set
     * bit[0] correctly to represent the opcode type of the branch target.
     * For details see:
     * http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.faqs/ka12545.html
     */

    fcn_thumb_flag = (uint32_t)MCG_SetSysDividers & 0x01;
    fcn_rom_addr = (uint32_t)MCG_SetSysDividers & ~(uint32_t)0x01;
    fcn_ram_addr = (uint32_t)fcn_ram_copy | (fcn_rom_addr & 0x02);

    FLib_MemCpyAligned32bit((void*)fcn_rom_addr, (void*)fcn_ram_addr, sizeof(fcn_ram_copy)-3);

    /* Get pointer of MCG_SetSysDividers function to run_in_ram_fcn  */
    pfSetSysDividers = (void (*)(uint32_t, uint32_t, uint32_t, uint32_t))(fcn_ram_addr | fcn_thumb_flag);    

#endif

    /* FEI - > FEE transition */             
#if (defined(MCU_MK60N512VMD100))   
    MCG_C2 = MCG_C2_RANGE(0); /* low frequency range select */
#elif(defined(MCU_MK20D5) || defined(MCU_MK60D10) || defined(MCU_MK21DN512) || defined(MCU_MK21DX256))
    MCG_C2 = MCG_C2_RANGE0(0); /* low frequency range select */
#endif

    MCG_C1 = MCG_C1_CLKS(2);  /* select external reference clock (ERC) as MCGOUTCLK */

    /* Wait until the output of the ERC is selected */
    while (((MCG_S & MCG_S_CLKST_MASK) >> MCG_S_CLKST_SHIFT != 0x2 )) {};

    /* wait for the FLL Reference clock status bit to clear */
    while (MCG_S & MCG_S_IREFST_MASK) {};

    /* 32.768KHz external reference clock, DCO Mid range, DCO Out = 48 MHz */
    MCG_C4 = MCG_C4_DMX32_MASK | MCG_C4_DRST_DRS(1);

    /* Ensure MCG_C6 is at the reset default of 0. LOLIE disabled, PLL disabled, clk monitor disabled, PLL VCO divider is clear */
    MCG_C6 = 0x0;

    /* Set system options dividers
     * MCG=FLL, core = MCG, bus = MCG, FlexBus = MCG, Flash clock= MCG/2
     */

#if (defined(__GNUC__))
    pfSetSysDividers(0,0,0,1);
#elif (defined(__IAR_SYSTEMS_ICC__))
    MCG_SetSysDividers(0,0,0,1);
#endif

    /* Enable Clock Monitor */    
#if (defined(MCU_MK60N512VMD100))
    MCG_C6 |= MCG_C6_CME_MASK;
#elif(defined(MCU_MK20D5) || defined(MCU_MK60D10) || defined(MCU_MK21DN512) || defined(MCU_MK21DX256))
    MCG_C6 |= MCG_C6_CME0_MASK;    
#endif

    /* wait for FLL status bit to set */
    while ((MCG_S & MCG_S_PLLST_MASK)) {};

    /* Transition into FEE by setting CLKS to 0 */
    MCG_C1 &= ~MCG_C1_CLKS_MASK;

    /* Wait for clock status bits to update */
    while (((MCG_S & MCG_S_CLKST_MASK) >> MCG_S_CLKST_SHIFT)) {};

    /* Now running FEE Mode */    
    return fll_freq;
}

/******************************************************************************
 * Name: MCG_TraceSysClk
 * Description: Trace the system clock (divided by 1000) using the FTM2 module
 * Parameters: -
 * Return: -
 ******************************************************************************/
void MCG_TraceSysClk
(
        void
)
{

#if (defined(MCU_MK60N512VMD100) || defined(MCU_MK60D10))
    /* Enable clock gating on PORTB */
    SIM_SCGC5 |= (uint32_t) SIM_SCGC5_PORTB_MASK;
    /* Enable clock gating on FTM2 module*/
    SIM_SCGC3 |= (uint32_t) SIM_SCGC3_FTM2_MASK;
    /* Setup pin PTB18 */
    PORTB_PCR18 &= ~(uint32_t) PORT_PCR_MUX_MASK;
    PORTB_PCR18 |= PORT_PCR_MUX(3);  
    /* FTM2 clock source = System Clock */
    FTM2_SC = (uint32_t) 0x08;
    /* PWM is edge-aligned. PWM toggles from high to low */
    FTM2_C0SC = (uint32_t)0x28; 
    /* PWM period = bus clock / 1000 */
    FTM2_MOD = 1000;
    /* PWM duty cycle = 50% */
    FTM2_C0V = 500;
#elif (defined(MCU_MK20D5) || defined(MCU_MK21DN512) || defined(MCU_MK21DX256))
    /* Enable clock gating on PORTB */
    SIM_SCGC5 |= (uint32_t) SIM_SCGC5_PORTB_MASK;
    /* Enable clock gating on FTM1 module*/
    SIM_SCGC6 |= (uint32_t) SIM_SCGC6_FTM1_MASK;
    /* Setup pin PTB18 */
    PORTB_PCR18 &= ~(uint32_t) PORT_PCR_MUX_MASK;
    PORTB_PCR18 |= PORT_PCR_MUX(3);  
    /* FTM1 clock source = System Clock */
    FTM1_SC = (uint32_t) 0x08;
    /* PWM is edge-aligned. PWM toggles from high to low */
    FTM1_C0SC = (uint32_t)0x28; 
    /* PWM period = bus clock / 1000 */
    FTM1_MOD = 1000;
    /* PWM duty cycle = 50% */
    FTM1_C0V = 500;
#endif
}

/******************************************************************************
 * Name: MCG_Fee2Fei
 * Description: Changes the MCG operation mode from FEE to FEI
 * Parameters: -
 * Return: -
 ******************************************************************************/
void MCG_Fee2Fei
(
        void
)
{
    /* disable clock monitor */
#if (gTargetTWR_K60N512_d == 1)
    MCG_C6 &= ~MCG_C6_CME_MASK;
#elif (gTargetTWR_K60D100M_d == 1)
    MCG_C6 &= ~MCG_C6_CME0_MASK;
#endif

    /* DCO has a default range of 25%. DCO Range Select: Low range */
    MCG_C4 = ( MCG_C4 & ( MCG_C4_FCTRIM_MASK | MCG_C4_SCFTRIM_MASK ) );

    /* change the FFL clock source. selects the slow internal reference clock */
    MCG_C1 |= MCG_C1_IREFS_MASK;

    /* wait for refernce clock to switch to internal */
    while( (MCG_S & MCG_S_IREFST_MASK) !=  MCG_S_IREFST_MASK ){};
}

/******************************************************************************
 * Name: MCG_Pee2Fei
 * Description: Changes the MCG operation mode from PEE to FEI
 * Parameters: -
 * Return: -
 ******************************************************************************/
void MCG_Pee2Fei
(
        void
)
{
    uint8_t mcgC1Reg;

    /* disable clock monitor */
#if (gTargetTWR_K60N512_d == 1)
    MCG_C6 &= ~MCG_C6_CME_MASK;
#elif (gTargetTWR_K60D100M_d == 1)
    MCG_C6 &= ~MCG_C6_CME0_MASK;
#endif

    // PEE to PBE

    /* switch CLKS mux to select external reference clock as MCG_OUT */
    MCG_C1 &= ~MCG_C1_CLKS_MASK;
    MCG_C1 |= MCG_C1_CLKS(2);

    /* wait for clock status bits to update */
    while( ( ( MCG_S & MCG_S_CLKST_MASK ) >> MCG_S_CLKST_SHIFT ) != 0x2 ){};

    // PBE to FBE

    /* disable PLL */
    MCG_C6 &= ~MCG_C6_PLLS_MASK;

    /* wait for PLLST bit to be clear */
    while( (MCG_S & MCG_S_PLLST_MASK) ==  MCG_S_PLLST_MASK ){};

    // FBE to FEI

    /* selects MCGOUTCLK clock source. output of FLL is selected */
    /* change the FFL clock source. selects the slow internal reference clock */
    mcgC1Reg  =  MCG_C1;
    mcgC1Reg &= ~MCG_C1_CLKS_MASK;
    mcgC1Reg |=  MCG_C1_IREFS_MASK;
    MCG_C1    =  mcgC1Reg;

    /* wait for refernce clock to switch to internal */
    while( (MCG_S & MCG_S_IREFST_MASK) !=  MCG_S_IREFST_MASK ){};

    /* wait for clock status bits to update */
    while( ( ( MCG_S & MCG_S_CLKST_MASK ) >> MCG_S_CLKST_SHIFT ) != 0x0 ){};


}

/********************************** EOF ***************************************/
