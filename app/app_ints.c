/**------------------------------------------------------------------------------------------------
 * Archivo con la funciones de interrupcion del micrcontroladores, revisar archivo startup_stm32g0b1.S
-------------------------------------------------------------------------------------------------*/
#include "app_bsp.h"


/**------------------------------------------------------------------------------------------------
Brief.- Punto de entrada del programa
-------------------------------------------------------------------------------------------------*/
void NMI_Handler( void ) /* cppcheck-suppress misra-c2012-8.4 ; this function can`t be modify */
{

}

/**------------------------------------------------------------------------------------------------
Brief.- Punto de entrada del programa
-------------------------------------------------------------------------------------------------*/
void HardFault_Handler( void ) /* cppcheck-suppress misra-c2012-8.4 ; this function can`t be modify */
{
    assert_param( 0u );
}

/**------------------------------------------------------------------------------------------------
Brief.- Punto de entrada del programa
-------------------------------------------------------------------------------------------------*/
void SVC_Handler( void )  /* cppcheck-suppress misra-c2012-8.4 ; this function can`t be modify */
{

}

/**------------------------------------------------------------------------------------------------
Brief.- Punto de entrada del programa
-------------------------------------------------------------------------------------------------*/
void PendSV_Handler( void )  /* cppcheck-suppress misra-c2012-8.4 ; this function can`t be modify */
{

}

/**------------------------------------------------------------------------------------------------
Brief.- Punto de entrada del programa
-------------------------------------------------------------------------------------------------*/
void SysTick_Handler( void )   /* cppcheck-suppress misra-c2012-8.4 ; this function can`t be modify */
{
    HAL_IncTick( );
}
/* cppcheck-suppress misra-c2012-8.5 ; this function can`t be modify */

    
void TIM16_FDCAN_IT0_IRQHandler( void )  /* cppcheck-suppress misra-c2012-8.4 ; this function can`t be modify */
{
    HAL_FDCAN_IRQHandler( &CANHandler );
}

