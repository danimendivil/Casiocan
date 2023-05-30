#include "hel_lcd.h"
/** 
* @defgroup Contrast conf This values are used fot he contrast configuration
@{ */
#define     max_contrast        16u     /*!< max value of contrast(4 bit register)*/
/**
@} */

/** 
* @defgroup Cursor postion values .
@{ */
#define     cursor_position     0x80u   /*!< cursor position command value*/
/**
@} */

/** 
* @defgroup LCD command values .
@{ */
#define     wakeup              0x30    /*!< wakeup command*/
#define     function_set        0x39    /*!< function set comman */
#define     internal_osc_freq   0x14    /*!< internal osc frequency command */
#define     power_control       0x56    /*!< power control command */
#define     follower_control    0x6d    /*!< follower control command */
#define     contrast_command    0x70    /*!< constrast command */
#define     display_on          0x0D    /*!< display on command */
#define     entry_mode          0x06    /*!< entry mode command */
#define     clear_screen        0x01    /*!< clear screen command */
/**
@} */
    
/**
* @brief   **This function initializes the parameters for the LCD and SPI**
*
*  This function asignates the pins to a type LCD_HandleTypeDef structure
*  Also Enables the pins by calling the HEL_LCD_MspInit function, then configure
*  The SPI and proceeds to do an initialization rutine for the LCD 
*
* @retval  SPI_state[out]    State of the spi initialization
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
    SPI_state = HAL_SPI_Init( hlcd->SpiHandler );
    
     /*LCD initialization rutine*/
    HAL_GPIO_WritePin( GPIOD, hlcd->CsPin, SET );       
    HAL_GPIO_WritePin( GPIOD, hlcd->RstPin, RESET );    

    HAL_GPIO_WritePin( GPIOD, hlcd->RstPin, SET );

    (void) HEL_LCD_Command(hlcd, wakeup ); 

    (void) HEL_LCD_Command(hlcd, wakeup ); 
    (void) HEL_LCD_Command(hlcd, wakeup ); 
    (void) HEL_LCD_Command(hlcd, function_set ); 
    (void) HEL_LCD_Command(hlcd, internal_osc_freq ); 
    (void) HEL_LCD_Command(hlcd, power_control ); 
    (void) HEL_LCD_Command(hlcd, follower_control ); 
     
    (void) HEL_LCD_Command(hlcd, contrast_command ); 
    (void) HEL_LCD_Command(hlcd, display_on ); 
    (void) HEL_LCD_Command(hlcd, entry_mode ); 
    (void) HEL_LCD_Command(hlcd, clear_screen ); 

    return SPI_state;
}

/**
* @brief   **This function sends a command to the LCD**
*
*  This function first put the RsPin on RESET to tell the LCD that a 
*  command is going to be recived, then puts the csPin on RESET that it knows
*  data will be send, then we transmit the message put the Cspin
*  on high to tell the lcd that no more data will be send
*  and we return the status of the spi funcion
*
* @retval  SPI_STATUS[out]    State of the spi transmit function
*/
uint8_t HEL_LCD_Command( LCD_HandleTypeDef *hlcd, uint8_t cmd )
{
    HAL_GPIO_WritePin( hlcd->RsPort, hlcd->RsPin, RESET );
    HAL_GPIO_WritePin( hlcd->CsPort, hlcd->CsPin, RESET );

    uint8_t SPI_STATUS = HAL_SPI_Transmit( hlcd->SpiHandler, &cmd, 1, 5000 );

    HAL_GPIO_WritePin( hlcd->CsPort, hlcd->CsPin, SET );
    HAL_Delay(10);

    return SPI_STATUS;
}

/**
* @brief   **This function sends a Data to the LCD**
*
*  This function first put the RsPin on SET to tell the LCD that 
*  data is going to be recived, then puts the csPin on RESET so that it knows
*  data will be send, then we transmit the message put the Cspin
*  on high to tell the lcd that no more data will be send
*  and we return the status of the spi funcion
*
* @retval  SPI_STATUS[out]    State of the spi transmit function
*/
uint8_t HEL_LCD_Data( LCD_HandleTypeDef *hlcd, uint8_t data ) /* cppcheck-suppress misra-c2012-8.7 ; function will later be used*/
{
    HAL_GPIO_WritePin( hlcd->RsPort, hlcd->RsPin, SET );
    HAL_GPIO_WritePin( hlcd->CsPort, hlcd->CsPin, RESET );
    
    uint8_t SPI_STATUS = HAL_SPI_Transmit( hlcd->SpiHandler, &data, 1, 5000 );

    HAL_GPIO_WritePin( hlcd->CsPort, hlcd->CsPin, SET );
    

    return SPI_STATUS;
}

/**
* @brief   **This function sends a string to the lcd**
*
*  This function use the strlen to see how long is the string 
*  and then sends each byte of the string with a loop using 
*  the HAL_LCD_Data function, if this function fails
*  the loop is finished.
*
* @retval  Data_status[out]    State of the HEL_LCD_Data function
*/
uint8_t HEL_LCD_String( LCD_HandleTypeDef *hlcd, char *str )
{
    uint8_t str_lenght = strlen(str);
    uint8_t Data_status = FALSE;

    for(uint8_t i = 0; i < str_lenght;i++)
    {
        Data_status=HEL_LCD_Data(hlcd, str[i] ); 
         
        if(Data_status == HAL_ERROR)
        {
            i = str_lenght+1u;   /* cppcheck-suppress misra-c2012-14.2 ; loop needs to be stoped if condition is met*/
        }
    }
    return Data_status;
}

/**
* @brief   **This function sets the cursor on the LCD**
*
*   The function asigns the variable pos the value of cursor_position
*   wich is 0x80, thats the value of the command,
*   set cursor in this LCD and adds the values of the position and row
* 
* @retval  Status for HAL_SPI_Transmit 
*
* @note Use the defines for row positions
*/
uint8_t HEL_LCD_SetCursor( LCD_HandleTypeDef *hlcd, uint8_t row, uint8_t col )
{
    uint8_t pos = cursor_position;
    pos=(pos | row) + col;

    uint8_t Cursor_Status=HEL_LCD_Command(hlcd, pos );

    return Cursor_Status;
}

/**
* @brief   **This function sets the LCD backlight**
*
*   The function changes the value of the pin assign.
*   if toogle is the value then the current value is toogle
*
* @retval  none 
*
* @note Use the defines SCREEN STATE
*/
void HEL_LCD_Backlight( LCD_HandleTypeDef *hlcd, uint8_t state )
{
    if(state == (uint8_t)TOGGLE)
    {
        /* cppcheck-suppress misra-c2012-10.3 ; operator is needed for the function*/
        state = !(hlcd->screen);    /* cppcheck-suppress misra-c2012-17.8 ; parameter usage leads to better code*/
    }
    HAL_GPIO_WritePin( hlcd->BklPort, hlcd->BklPin, state );
    hlcd->screen=state;
}

/**
* @brief   **This function sets the contrast of the LCD**
*
*   The function asigns the variable contrast_level the value of contrast_command
*   wich is 0x70, thats the value of the command, then checks if
*   the value of the contrast is less than max_contrast wich is 16
*   since thats the max value that the lcd contrast has, if it is 
*   less then this value is added to the contrast_level and is transmited 
*   with the HEL_LCD_Command function  
* 
* @retval  contrast_state Status of contrast function
*
*/
uint8_t HEL_LCD_Contrast( LCD_HandleTypeDef *hlcd, uint8_t contrast )
{
    uint8_t contrast_state = FALSE;
    uint8_t contrast_level = contrast_command;
    if (contrast < max_contrast )
    {
        contrast_state = HEL_LCD_Command(hlcd,(contrast_level + contrast));
    }
    else
    {
       contrast_state = FALSE;  
    }
    return contrast_state;   
}