#ifndef HEL_LCD_H_
#define HEL_LCD_H_

    #include "app_bsp.h"
    #include <string.h>

    uint8_t HEL_LCD_Init( LCD_HandleTypeDef *hlcd );
    void HEL_LCD_MspInit( LCD_HandleTypeDef *hlcd )__attribute__((weak));
    uint8_t HEL_LCD_Command( LCD_HandleTypeDef *hlcd, uint8_t cmd );
    uint8_t HEL_LCD_Data( LCD_HandleTypeDef *hlcd, uint8_t data );
    uint8_t HEL_LCD_String( LCD_HandleTypeDef *hlcd, char *str );
    uint8_t HEL_LCD_SetCursor( LCD_HandleTypeDef *hlcd, uint8_t row, uint8_t col );
    void HEL_LCD_Backlight( LCD_HandleTypeDef *hlcd, uint8_t state );
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



#endif