#ifndef HEL_LCD_H_
#define HEL_LCD_H_

    #include "app_bsp.h"
    #include <string.h>

    /** 
    * @defgroup Row positions this are the values of the first and second row of the lcd
    @{ */
    #define first_row       0
    #define second_row      0x40
    /**
    @} */

    /** 
    * @defgroup Screen state this are defines for the states of the screen
    @{ */
    #define OFF             0
    #define ON              1
    #define TOGGLE          2
    /**
    @} */

    /** 
    * @defgroup <LCD HandleTypedef> Pins and ports of the lcd
    @{ */
    typedef struct
    {
        SPI_HandleTypeDef    *SpiHandler; /* SPI handler address of the spi to use with the LCD*/
        GPIO_TypeDef        *RstPort;   /*port where the pin to control the LCD reset pin is*/
        uint32_t            RstPin; /*pin to control the LCD reset pin */
        GPIO_TypeDef        *RsPort; /*port where the pin to control the LCD RS pin*/
        uint32_t            RsPin;  /*pin to control the LCD RS pin */
        GPIO_TypeDef        *CsPort; /*port where the pin to control the LCD chip select is*/
        uint32_t            CsPin;  /*pin to control the LCD chip select pin */
        GPIO_TypeDef        *BklPort; /*port where the pin to control the LCD backlight is*/
        uint32_t            BklPin;  /*pin to control the LCD backlight pin */
        uint8_t             screen   /*state of lcd screen */
        //agregar más elementos si se requieren
    } LCD_HandleTypeDef;
    /**
    @} */


#endif