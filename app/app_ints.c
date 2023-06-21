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
    Status = HAL_ERROR;
    assert_error( Status == HAL_OK, HARDFAULT_ERROR );  /* cppcheck-suppress misra-c2012-11.8 ; function cannot be modify */
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

/* cppcheck-suppress misra-c2012-2.7 ; this is a library function */
void HAL_FDCAN_ErrorCallback(FDCAN_HandleTypeDef *hfdcan)   /* cppcheck-suppress misra-c2012-8.4 ; this function can`t be modify */
{
    Status = HAL_ERROR;
    assert_error( Status == HAL_OK, FDCAN_CALLBACK_ERROR ); /* cppcheck-suppress misra-c2012-11.8 ; function cannot be modify */
}

/* cppcheck-suppress misra-c2012-2.7 ; this is a library function */
void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi) /* cppcheck-suppress misra-c2012-8.4 ; this function can`t be modify */
{
    Status = HAL_ERROR;
    assert_error( Status == HAL_OK, SPI_CALLBACK_ERROR );   /* cppcheck-suppress misra-c2012-11.8 ; function cannot be modify */
}

/* cppcheck-suppress misra-c2012-2.7 ; this is a library function */
void HAL_WWDG_EarlyWakeupCallback( WWDG_HandleTypeDef *hwwdg )  /* cppcheck-suppress misra-c2012-8.4 ; this function can`t be modify */
{
    Status = HAL_ERROR;
    assert_error( Status == HAL_OK, WWDG_EARLY_WAKEUP_ERROR );  /* cppcheck-suppress misra-c2012-11.8 ; function cannot be modify */
}

void FLASH_IRQHandler(void)     /* cppcheck-suppress misra-c2012-8.4 ; this function can`t be modify */
{
    
    if (__HAL_FLASH_GET_FLAG(FLASH_FLAG_ECCC) != RESET)
    {
        Status = HAL_ERROR;
        assert_error( Status == HAL_OK, ECC_ONE_ERROR );    /* cppcheck-suppress misra-c2012-11.8 ; function cannot be modify */
    }
    if (__HAL_FLASH_GET_FLAG(FLASH_FLAG_ECCD) != RESET)
    {
        Status = HAL_ERROR;
        assert_error( Status == HAL_OK, ECC_TWO_ERROR );    /* cppcheck-suppress misra-c2012-11.8 ; function cannot be modify */
    }
}

void RTC_TAMP_IRQHandler( void )
{
    Alarm_State = ALARM_ACTIVE 
    HAL_RTC_AlarmIRQHandler( &hrtc );
}