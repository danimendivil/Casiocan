#include "hel_lcd.h"

void HEL_LCD_MspInit( LCD_HandleTypeDef *hlcd )
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
}

/**
* @brief   **This function initializes the parameters for the LCD and SPI**
*
*  This function asignates the pins to a type LCD_HandleTypeDef structure
*  Also Enables the pins, this function do not initialize the SPI.
*
* @retval  none 
*/
uint8_t HEL_LCD_Init( LCD_HandleTypeDef *hlcd )
{
    uint8_t SPI_state;
    /*Reset pin configuration*/
    hlcd->RstPort     =   GPIOD;
    hlcd->RstPin      =   GPIO_PIN_2;
    /*LCD rs pin configuration*/
    hlcd->RsPort      =   GPIOD;
    hlcd->RsPin       =   GPIO_PIN_4;
    /*Chip select pint configuration*/
    hlcd->CsPort      =   GPIOD;
    hlcd->CsPin       =   GPIO_PIN_3;
    /*Backliht pin configuration*/
    hlcd->BklPort     =   GPIOB;
    hlcd->BklPin      =   GPIO_PIN_4;

    HEL_LCD_MspInit(hlcd);
    /*SPI init*/
    HAL_GPIO_WritePin( hlcd->BklPort, hlcd->BklPin, SET );

    hlcd->SpiHandler->Instance            = SPI1;
    hlcd->SpiHandler->Init.Mode           = SPI_MODE_MASTER;
    hlcd->SpiHandler->Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;
    hlcd->SpiHandler->Init.Direction      = SPI_DIRECTION_2LINES;
    hlcd->SpiHandler->Init.CLKPhase       = SPI_PHASE_2EDGE;
    hlcd->SpiHandler->Init.CLKPolarity    = SPI_POLARITY_HIGH;
    hlcd->SpiHandler->Init.DataSize       = SPI_DATASIZE_8BIT;
    hlcd->SpiHandler->Init.FirstBit       = SPI_FIRSTBIT_MSB;
    hlcd->SpiHandler->Init.NSS            = SPI_NSS_SOFT;
    hlcd->SpiHandler->Init.CRCCalculation = SPI_CRCCALCULATION_DISABLED;
    hlcd->SpiHandler->Init.TIMode         = SPI_TIMODE_DISABLED;
    
    HAL_GPIO_WritePin( hlcd->CsPort, hlcd->CsPin, SET );
    SPI_state=HAL_SPI_Init( hlcd->SpiHandler );
    

    HAL_GPIO_WritePin( GPIOD, hlcd->CsPin, SET );       
    HAL_GPIO_WritePin( GPIOD, hlcd->RstPin, RESET );    

    HAL_GPIO_WritePin( GPIOD, hlcd->RstPin, SET );

    HEL_LCD_Command(hlcd, 0x30 ); /*wakeup*/

    HEL_LCD_Command(hlcd, 0x30 ); /*wakeup*/
    HEL_LCD_Command(hlcd, 0x30 ); /*wakeup*/
    HEL_LCD_Command(hlcd, 0x39 ); /*function set*/
    HEL_LCD_Command(hlcd, 0x14 ); /*internal osc frequency*/
    HEL_LCD_Command(hlcd, 0x56 ); /*power controll*/
    HEL_LCD_Command(hlcd, 0x6d ); /*follower control*/
     
    HEL_LCD_Command(hlcd, 0x70 ); /*constrast*/
    HEL_LCD_Command(hlcd, 0x0D ); /*display on*/
    HEL_LCD_Command(hlcd, 0x06 ); /*entry mode*/
    HEL_LCD_Command(hlcd, 0x01 ); /*clear screen*/

    return SPI_state;
}

uint8_t HEL_LCD_Command( LCD_HandleTypeDef *hlcd, uint8_t cmd )
{
    HAL_GPIO_WritePin( hlcd->RsPort, hlcd->RsPin, RESET );
    HAL_GPIO_WritePin( hlcd->CsPort, hlcd->CsPin, RESET );

    uint8_t LCD_Command_State = HAL_SPI_Transmit( hlcd->SpiHandler, &cmd, 1, 5000 );

    HAL_GPIO_WritePin( hlcd->CsPort, hlcd->CsPin, SET );
    HAL_Delay(30);

    return LCD_Command_State;
}

uint8_t HEL_LCD_Data( LCD_HandleTypeDef *hlcd, uint8_t data )
{
    HAL_GPIO_WritePin( hlcd->RsPort, hlcd->RsPin, SET );
    HAL_GPIO_WritePin( hlcd->CsPort, hlcd->CsPin, RESET );
    
    uint8_t ret = HAL_SPI_Transmit( hlcd->SpiHandler, &data, 1, 5000 );

    HAL_GPIO_WritePin( hlcd->CsPort, hlcd->CsPin, SET );
    
    HAL_Delay(30);

    return ret;
}