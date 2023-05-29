#ifndef HEL_LCD_H_
#define HEL_LCD_H_

    #include "app_bsp.h"
    #include <string.h>

/**
  * @brief   LCD_HandleTypedef Pins and ports of the lcd
  */
    typedef struct LCD_HandleTypeDef
    {
        SPI_HandleTypeDef   *SpiHandler; /*!< SPI handler address of the spi to use with the LCD */
        GPIO_TypeDef        *RstPort;    /*!< Port where the pin to control the LCD reset pin is */
        uint32_t            RstPin;      /*!< Pin to control the LCD reset pin */
        GPIO_TypeDef        *RsPort;     /*!< Port where the pin to control the LCD RS pin is */
        uint32_t            RsPin;       /*!< Pin to control the LCD RS pin */
        GPIO_TypeDef        *CsPort;     /*!< Port where the pin to control the LCD chip select is */
        uint32_t            CsPin;       /*!< Pin to control the LCD chip select pin */
        GPIO_TypeDef        *BklPort;    /*!< Port where the pin to control the LCD backlight is */
        uint32_t            BklPin;      /*!< Pin to control the LCD backlight pin */
        uint8_t             screen;      /*!< State of the LCD screen */
        // Add more elements if needed
    } LCD_HandleTypeDef;    
    
    
    uint8_t HEL_LCD_Init( LCD_HandleTypeDef *hlcd );
    void HEL_LCD_MspInit( LCD_HandleTypeDef *hlcd )__attribute__((weak));
    uint8_t HEL_LCD_Command( LCD_HandleTypeDef *hlcd, uint8_t cmd );
    uint8_t HEL_LCD_Data( LCD_HandleTypeDef *hlcd, uint8_t data );
    uint8_t HEL_LCD_String( LCD_HandleTypeDef *hlcd, char *str );
    uint8_t HEL_LCD_SetCursor( LCD_HandleTypeDef *hlcd, uint8_t row, uint8_t col );
    void HEL_LCD_Backlight( LCD_HandleTypeDef *hlcd, uint8_t state );
    uint8_t HEL_LCD_Contrast( LCD_HandleTypeDef *hlcd, uint8_t contrast );
    /** 
     * @defgroup Row_positions this are the values of the first and second row of the lcd
     @{ */
    #define first_row       0       /*!< first_row value */     /* cppcheck-suppress misra-c2012-2.5 ; tags will later be used*/
    #define second_row      0x40    /*!< second_row value */    /* cppcheck-suppress misra-c2012-2.5 ; tags will later be used*/
    /**
       @} */

    /** 
    * @defgroup Screen_state this are defines for the states of the screen
    @{ */
    #define OFF             0       /*!< OFF value */   /* cppcheck-suppress misra-c2012-2.5 ; tags will later be used*/
    #define ON              1       /*!< ON value */     /* cppcheck-suppress misra-c2012-2.5 ; tags will later be used*/
    #define TOGGLE          2       /*!< TOGGLE value */   /* cppcheck-suppress misra-c2012-2.5 ; tags will later be used*/
    /**
        @} */



#endif