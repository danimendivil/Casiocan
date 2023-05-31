#include "hel_lcd.h"
/** 
* @defgroup Contrast conf This values are used fot he contrast configuration
@{ */
#define     MAX_CONTRAST        16u     /*!< max value of contrast(4 bit register)*/
/**
@} */

/** 
* @defgroup Cursor postion values .
@{ */
#define     CURSOR_POSITION     0x80u   /*!< cursor position command value*/
/**
@} */

/** 
* @defgroup LCD command values .
@{ */
#define     WAKEUP              0x30    /*!< wakeup command*/
#define     FUNCTION_SET        0x39    /*!< function set comman */
#define     INTERNAL_OSC_FREQ   0x14    /*!< internal osc frequency command */
#define     POWER_CONTROL       0x56    /*!< power control command */
#define     FOLLOWER_CONTROL    0x6d    /*!< follower control command */
#define     CONTRAST_COMMAND    0x70    /*!< constrast command */
#define     DISPLAT_ON          0x0D    /*!< display on command */
#define     ENTRY_MODE          0x06    /*!< entry mode command */
#define     CLEAR_SCREEN        0x01    /*!< clear screen command */
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
    /*LCD initialization rutine*/
    HAL_GPIO_WritePin( GPIOD, hlcd->CsPin, SET );       
    HAL_GPIO_WritePin( GPIOD, hlcd->RstPin, RESET );    

    HAL_GPIO_WritePin( GPIOD, hlcd->RstPin, SET );
    HAL_Delay(1);
        /*SPI STATE WILL LATER BE USED ON FUNCTIONAL SAFETY*/
    SPI_state =  HEL_LCD_Command(hlcd, WAKEUP ); 

    SPI_state =  HEL_LCD_Command(hlcd, WAKEUP ); 
    SPI_state =  HEL_LCD_Command(hlcd, WAKEUP ); 
    SPI_state =  HEL_LCD_Command(hlcd, FUNCTION_SET ); 
    SPI_state =  HEL_LCD_Command(hlcd, INTERNAL_OSC_FREQ ); 
    SPI_state =  HEL_LCD_Command(hlcd, POWER_CONTROL ); 
    SPI_state =  HEL_LCD_Command(hlcd, FOLLOWER_CONTROL ); 
     
    SPI_state =  HEL_LCD_Command(hlcd, CONTRAST_COMMAND ); 
    SPI_state =  HEL_LCD_Command(hlcd, DISPLAT_ON ); 
    SPI_state =  HEL_LCD_Command(hlcd, ENTRY_MODE ); 
    SPI_state =  HEL_LCD_Command(hlcd, CLEAR_SCREEN ); 
    HAL_Delay(1);
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
*   The function asigns the variable pos the value of CURSOR_POSITION
*   wich is 0x80, thats the value of the command,
*   set cursor in this LCD and adds the values of the position and row
* 
* @retval  Status for HAL_SPI_Transmit 
*
* @note Use the defines for row positions
*/
uint8_t HEL_LCD_SetCursor( LCD_HandleTypeDef *hlcd, uint8_t row, uint8_t col )
{
    uint8_t pos = CURSOR_POSITION;
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
*   The function asigns the variable contrast_level the value of CONTRAST_COMMAND
*   wich is 0x70, thats the value of the command, then checks if
*   the value of the contrast is less than MAX-CONTRAST wich is 16
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
    uint8_t contrast_level = CONTRAST_COMMAND;
    if (contrast < MAX_CONTRAST )
    {
        contrast_state = HEL_LCD_Command(hlcd,(contrast_level + contrast));
    }
    else
    {
       contrast_state = FALSE;  
    }
    return contrast_state;   
}