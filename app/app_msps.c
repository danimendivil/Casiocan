/**------------------------------------------------------------------------------------------------
 * Archivo con la funciones de las incilaizaciones auxiliares de la libreria
-------------------------------------------------------------------------------------------------*/
#include "app_bsp.h"

void HAL_MspInit( void )   /* cppcheck-suppress misra-c2012-8.4 ; this is a library function */
{
    
}
/* cppcheck-suppress misra-c2012-8.4 ; this is a library function */
void HAL_FDCAN_MspInit(FDCAN_HandleTypeDef *hfdcan)  /* cppcheck-suppress misra-c2012-2.7 ; this is a library function */
{
    GPIO_InitTypeDef GpioCanStruct;

    /* Habilitamos los relojes de los perifericos GPIO y CAN */
    __HAL_RCC_FDCAN_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    /* configuramos pin 8(rx) y pin 9(tx) en modo alterno para FDCAN1 */
    GpioCanStruct.Mode = GPIO_MODE_AF_PP;
    GpioCanStruct.Alternate = GPIO_AF3_FDCAN1;
    GpioCanStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1;
    GpioCanStruct.Pull = GPIO_NOPULL;
    GpioCanStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init( GPIOD, &GpioCanStruct );
        
    HAL_NVIC_SetPriority(TIM16_FDCAN_IT0_IRQn, 2, 0);
    HAL_NVIC_EnableIRQ(TIM16_FDCAN_IT0_IRQn);
}