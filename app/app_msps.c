/**------------------------------------------------------------------------------------------------
 * Archivo con la funciones de las incilaizaciones auxiliares de la libreria
-------------------------------------------------------------------------------------------------*/
#include "app_bsp.h"
#include "hel_lcd.h"
/**
* @brief   **HAL_MspInit**
*
*   This function initializes the RCC Oscillators according to the specified parameters in the RCC_OscInitTypeDef structure
*   The frequency set is 64MHz with the internal 16MHz HSI oscilator. According to the formulas:
*   fVCO = fPLLIN x ( N / M ) = 16MHz x (8 / 1) = 128MHz
*   fPLLP = fVCO / P = 128MHz / 2 = 64MHz
*   fPLLQ = fVCO / Q = 128MHz / 2 = 64MHz
*   fPLLR = fVCO / R = 128MHz / 2 = 64MHz
*
*/
void HAL_MspInit( void )   /* cppcheck-suppress misra-c2012-8.4 ; this is a library function */
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    __HAL_RCC_SYSCFG_CLK_ENABLE();
    __HAL_RCC_PWR_CLK_ENABLE();

    /** Configure the main internal regulator output voltage*/
    Status = HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);
    assert_error( Status == HAL_OK, PWREX_CONTROL_VOLTAGE_ERROR );  /* cppcheck-suppress misra-c2012-11.8 ; function cannot be modify */

    RCC_OscInitStruct.OscillatorType      = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState            = RCC_HSI_ON;
    RCC_OscInitStruct.HSIDiv              = RCC_HSI_DIV1;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState        = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource       = RCC_PLLSOURCE_HSI;
    RCC_OscInitStruct.PLL.PLLM            = RCC_PLLM_DIV1;
    RCC_OscInitStruct.PLL.PLLN            = 8;
    RCC_OscInitStruct.PLL.PLLP            = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLQ            = RCC_PLLQ_DIV2;
    RCC_OscInitStruct.PLL.PLLR            = RCC_PLLR_DIV2;
    Status = HAL_RCC_OscConfig( &RCC_OscInitStruct );
    assert_error( Status == HAL_OK, RCC_OSC_CONF_ERROR );   /* cppcheck-suppress misra-c2012-11.8 ; function cannot be modify */
    
    /** Initializes the CPU, AHB and APB buses clocks*/
    RCC_ClkInitStruct.ClockType       = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1;
    RCC_ClkInitStruct.SYSCLKSource    = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider   = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider  = RCC_HCLK_DIV2;
    Status = HAL_RCC_ClockConfig( &RCC_ClkInitStruct, FLASH_LATENCY_2 );
    assert_error( Status == HAL_OK, RCC_CLOCK_CONF_ERROR ); /* cppcheck-suppress misra-c2012-11.8 ; function cannot be modify */
    
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

/* cppcheck-suppress misra-c2012-8.4 ; this is a library function */
/* cppcheck-suppress misra-c2012-2.7 ; this declaration is not used */
void HAL_RTC_MspInit( RTC_HandleTypeDef* hrtc )   /* cppcheck-suppress misra-c2012-2.7 ; this is a library function */ 
{
    RCC_OscInitTypeDef        RCC_OscInitStruct ;
    RCC_PeriphCLKInitTypeDef  PeriphClkInitStruct ;

    __HAL_RCC_SYSCFG_CLK_ENABLE();
    __HAL_RCC_PWR_CLK_ENABLE();
    
    /*Eanlble backup domain*/
    Status = HAL_PWREx_ControlVoltageScaling( PWR_REGULATOR_VOLTAGE_SCALE1 );
    assert_error( Status == HAL_OK, PWREX_CONTROL_VOLTAGE_ERROR );  /* cppcheck-suppress misra-c2012-11.8 ; function cannot be modify */
    HAL_PWR_EnableBkUpAccess();
    __HAL_RCC_LSEDRIVE_CONFIG( RCC_LSEDRIVE_LOW );

    /*reset previous RTC source clock*/
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RTC;
    PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_NONE;
    Status = HAL_RCCEx_PeriphCLKConfig( &PeriphClkInitStruct );
    assert_error( Status == HAL_OK, RCCEX_PRIPH_CLK_CONF_ERROR );   /* cppcheck-suppress misra-c2012-11.8 ; function cannot be modify */

    /* Configure LSE/LSI as RTC clock source */
    RCC_OscInitStruct.OscillatorType =  RCC_OSCILLATORTYPE_LSI | RCC_OSCILLATORTYPE_LSE;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
    RCC_OscInitStruct.LSEState = RCC_LSE_ON;
    RCC_OscInitStruct.LSIState = RCC_LSI_OFF;
    Status = HAL_RCC_OscConfig( &RCC_OscInitStruct );
    assert_error( Status == HAL_OK, RCC_OSC_CONF_ERROR );   /* cppcheck-suppress misra-c2012-11.8 ; function cannot be modify */

    /*Set LSE as source clock*/
    PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
    Status = HAL_RCCEx_PeriphCLKConfig( &PeriphClkInitStruct );
    assert_error( Status == HAL_OK, RCCEX_PRIPH_CLK_CONF_ERROR );   /* cppcheck-suppress misra-c2012-11.8 ; function cannot be modify */
      
    /* Peripheral clock enable */
    __HAL_RCC_RTC_ENABLE();
    __HAL_RCC_RTCAPB_CLK_ENABLE();
}

/* cppcheck-suppress misra-c2012-8.4 ; this is a library function */
void HAL_SPI_MspInit( SPI_HandleTypeDef *hspi )     /* cppcheck-suppress misra-c2012-2.7 ; this is a library function */
{
    /*pines B13, B14 y B15 en funcion alterna spi1 */
    GPIO_InitTypeDef GPIO_InitStruct;
    __GPIOD_CLK_ENABLE();
    __SPI1_CLK_ENABLE();

    GPIO_InitStruct.Pin = GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_8;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF1_SPI1;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

    HAL_NVIC_SetPriority(SPI1_IRQn, 2, 0);
    HAL_NVIC_EnableIRQ(SPI1_IRQn);
}

/* cppcheck-suppress misra-c2012-8.4 ; this is a library function */
/**
* @brief   **This function initializes the GPIO ports for the LCD**
*
*  This function initializate the RstPin,CsPin,RsPin and BklPin
*  this pins has to be previusly assigned.
*
* @retval  none
*/
void HEL_LCD_MspInit( LCD_HandleTypeDef *hlcd ) /* cppcheck-suppress misra-c2012-8.7 ; function will later be used*/
{
    HAL_Init(); 
    GPIO_InitTypeDef GPIO_InitStruct;
    __GPIOD_CLK_ENABLE(); /* habilitamos reloj del puerto C */

    GPIO_InitStruct.Pin   = hlcd->RstPin|hlcd->CsPin|hlcd->RsPin;              /*pines a configurar*/
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP; /*salida tipo push-pull*/
    GPIO_InitStruct.Pull  = GPIO_NOPULL;        /*pin sin pull-up ni pull-down*/
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;     /*pin a baja velocidad*/
    /*inicializamos pines con los parametros anteriores*/
    HAL_GPIO_Init( hlcd->RstPort, &GPIO_InitStruct );

    __GPIOB_CLK_ENABLE(); /* habilitamos reloj del puerto C */

    GPIO_InitStruct.Pin   = hlcd->BklPin;              /*pines a configurar*/
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP; /*salida tipo push-pull*/
    GPIO_InitStruct.Pull  = GPIO_NOPULL;        /*pin sin pull-up ni pull-down*/
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;     /*pin a baja velocidad*/
    /*inicializamos pines con los parametros anteriores*/
    HAL_GPIO_Init( hlcd->BklPort, &GPIO_InitStruct );
    HAL_GPIO_WritePin( hlcd->BklPort, hlcd->BklPin, SET );
}

/* cppcheck-suppress misra-c2012-8.4 ; there is no need for external linkage*/
void HAL_TIM_PWM_MspInit( TIM_HandleTypeDef *htim ) /* cppcheck-suppress misra-c2012-2.7 ; this is a library function */
{
    GPIO_InitTypeDef   GPIO_InitStruct;

    __TIM14_CLK_ENABLE();  /*activamos reloj en TIM1*/
    __GPIOC_CLK_ENABLE(); /*activamos reloj del puerto A*/

    GPIO_InitStruct.Pin = GPIO_PIN_12; /*pin C8 como TIM1_CH1*/
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF2_TIM1;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
}