#include "app_display.h"
#include "hel_lcd.h"
LCD_HandleTypeDef LCDHandle;
SPI_HandleTypeDef SpiHandle;

void Display_Init( void )
{
    LCDHandle.SpiHandler  =   &SpiHandle;
    /*Reset pin configuration*/
    LCDHandle.RstPort     =   GPIOD;
    LCDHandle.RstPin      =   GPIO_PIN_2;
    /*LCD rs pin configuration*/
    LCDHandle.RsPort      =   GPIOD;
    LCDHandle.RsPin       =   GPIO_PIN_4;
    /*Chip select pint configuration*/
    LCDHandle.CsPort      =   GPIOD;
    LCDHandle.CsPin       =   GPIO_PIN_3;
    /*Backliht pin configuration*/
    LCDHandle.BklPort     =   GPIOB;
    LCDHandle.BklPin      =   GPIO_PIN_4;

    HEL_LCD_MspInit(&LCDHandle);

    LCDHandle.SpiHandler->Instance            = SPI1;
    LCDHandle.SpiHandler->Init.Mode           = SPI_MODE_MASTER;
    LCDHandle.SpiHandler->Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;
    LCDHandle.SpiHandler->Init.Direction      = SPI_DIRECTION_2LINES;
    LCDHandle.SpiHandler->Init.CLKPhase       = SPI_PHASE_2EDGE;
    LCDHandle.SpiHandler->Init.CLKPolarity    = SPI_POLARITY_HIGH;
    LCDHandle.SpiHandler->Init.DataSize       = SPI_DATASIZE_8BIT;
    LCDHandle.SpiHandler->Init.FirstBit       = SPI_FIRSTBIT_MSB;
    LCDHandle.SpiHandler->Init.NSS            = SPI_NSS_SOFT;
    LCDHandle.SpiHandler->Init.CRCCalculation = SPI_CRCCALCULATION_DISABLED;
    LCDHandle.SpiHandler->Init.TIMode         = SPI_TIMODE_DISABLED;
    
    HAL_GPIO_WritePin( LCDHandle.CsPort, LCDHandle.CsPin, SET );
    HAL_SPI_Init( LCDHandle.SpiHandler );

    HEL_LCD_Init(&LCDHandle );
}


