/*****************************************************************************
 *                               INCLUDED HEADERS                            *
 *---------------------------------------------------------------------------*
 * Add to this section all the headers that this module needs to include.    *
 *---------------------------------------------------------------------------*
 *****************************************************************************/

#include "SimplePhy.h"


/*****************************************************************************
 *                               PRIVATE VARIABLES                           *
 *---------------------------------------------------------------------------*
 * Add to this section all the variables and constants that have local       *
 * (file) scope.                                                             *
 * Each of this declarations shall be preceded by the 'static' keyword.      *
 * These variables / constants cannot be accessed outside this module.       *
 *---------------------------------------------------------------------------*
 *****************************************************************************/


/*****************************************************************************
 *                               PUBLIC VARIABLES                            *
 *---------------------------------------------------------------------------*
 * Add to this section all the variables and constants that have global      *
 * (project) scope.                                                          *
 * These variables / constants can be accessed outside this module.          *
 * These variables / constants shall be preceded by the 'extern' keyword in  *
 * the interface header.                                                     *
 *---------------------------------------------------------------------------*
 *****************************************************************************/

/*****************************************************************************
 *                           PRIVATE FUNCTIONS PROTOTYPES                    *
 *---------------------------------------------------------------------------*
 * Add to this section all the functions prototypes that have local (file)   *
 * scope.                                                                    *
 * These functions cannot be accessed outside this module.                   *
 * These declarations shall be preceded by the 'static' keyword.             *
 *---------------------------------------------------------------------------*
 *****************************************************************************/

/*****************************************************************************
 *                                PRIVATE FUNCTIONS                          *
 *---------------------------------------------------------------------------*
 * Add to this section all the functions that have local (file) scope.       *
 * These functions cannot be accessed outside this module.                   *
 * These definitions shall be preceded by the 'static' keyword.              *
 *---------------------------------------------------------------------------*
 *****************************************************************************/

/*****************************************************************************
 *                             PUBLIC FUNCTIONS                              *
 *---------------------------------------------------------------------------*
 * Add to this section all the functions that have global (project) scope.   *
 * These functions can be accessed outside this module.                      *
 * These functions shall have their declarations (prototypes) within the     *
 * interface header file and shall be preceded by the 'extern' keyword.      *
 *---------------------------------------------------------------------------*
 *****************************************************************************/
   
/*---------------------------------------------------------------------------
 * Name: PhyPpSetPromiscuous
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/   
void PhyXtalAdjust(uint8_t u8Value)
{

   MC1324xDrv_IndirectAccessSPIWrite(XTAL_TRIM,u8Value);

}
/*---------------------------------------------------------------------------
 * Name: PhyWakeRequest
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/ 
void PhyWakeRequest(void)
{
   uint8_t phyPwrModesReg = 0;
   phyPwrModesReg = MC1324xDrv_DirectAccessSPIRead(PWR_MODES);
   phyPwrModesReg |= (0x11);     // IDLE MODE, XTALEN = 1, and PMC_MODE = 1
   MC1324xDrv_DirectAccessSPIWrite(PWR_MODES,phyPwrModesReg);
}
/*---------------------------------------------------------------------------
 * Name: PhyGetRfIcVersion
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
 uint8_t PhyGetRfIcVersion(void)
 {  
   return MC1324xDrv_IndirectAccessSPIRead(PART_ID);
 }
/*---------------------------------------------------------------------------
 * Name: PhyPlmeDozeRequest
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
void PhyPlmeDozeRequest(uint32_t u32TimeOut)
{
   uint8_t phyPwrModesReg = 0; 
   (void)u32TimeOut;
   phyPwrModesReg = MC1324xDrv_DirectAccessSPIRead(PWR_MODES);
   phyPwrModesReg |= (0x10);  /* XTALEN = 1 */
   phyPwrModesReg &= (0xFE);  /* PMC_MODE = 0 */
   MC1324xDrv_DirectAccessSPIWrite(PWR_MODES,phyPwrModesReg);
}
/*---------------------------------------------------------------------------
 * Name: PhyHibernateRequest
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
void PhyHibernateRequest(void)
{
   uint8_t phyPwrModesReg = 0;
   phyPwrModesReg = MC1324xDrv_DirectAccessSPIRead(PWR_MODES);
   phyPwrModesReg &= (0xEE);  /* XTALEN = 0, PMC_MODE = 0 */
   MC1324xDrv_DirectAccessSPIWrite(PWR_MODES,phyPwrModesReg);
}