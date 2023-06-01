#include "app_display.h"
#include "hel_lcd.h"

/** 
  * @defgroup <Lcd state machine> this are defines for the states
  @{ */
#define    STATE_IDLE               0       /*!<Idle state value*/
#define    STATE_PRINTH_MONTH       1       /*!<month state value*/
#define    STATE_PRINTH_DAY         2       /*!<day state value*/
#define    STATE_PRINTH_YEAR        3       /*!<year state value*/
#define    STATE_PRINTH_WDAY        4       /*!<week day state value*/  
#define    STATE_PRINTH_HOUR        5       /*!<hour state value*/
#define    STATE_PRINTH_MINUTES     6       /*!<minutes state value*/
#define    STATE_PRINTH_SECONDS     7       /*!<seconds state value*/
/**
  @} */

/**
 * @brief  Variable for LCD configuration
 */
static LCD_HandleTypeDef LCDHandle;

/**
 * @brief  Variable for SPI configuration
 */
static SPI_HandleTypeDef SpiHandle;     /* cppcheck-suppress misra-c2012-8.9 ; the function stops working if defined in there*/

/**
 * @brief  Variable for displaying the first row of the lcd
 */
/* cppcheck-suppress misra-c2012-8.9 ; the function stops working if defined in there*/
static char fila_1[] =" XXX,XX XXXX XX "; /* cppcheck-suppress misra-c2012-7.4 ; string need to be modify*/

/**
 * @brief  Variable for displaying the second row of the lcd
 */
/* cppcheck-suppress misra-c2012-8.9 ; the function stops working if defined in there*/
static char fila_2[] = "00:00:00";        /* cppcheck-suppress misra-c2012-7.4 ; string need to be modify*/ 

/**
 * @brief  Variable for display state machine 
 */
static uint8_t LCD_State  = STATE_IDLE;    /* cppcheck-suppress misra-c2012-8.9 ; the function stops working if defined in there*/

static void month(char *mon,char pos);
static void week(char *week,char pos);

/**
 * @brief   **This function intiates the LCD and the SPI **
 *
 *  The Function sets the pins for the SPI and LCD  needed and initialize by 
 *  calling the HEL_LCD_MspInit and configuring  and initializing the SPI
 *  then calls the function HEL_LCD_Init wich is a routine for the LCD
 * 
 * @retval  None 
 */
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

    (void)HEL_LCD_Init(&LCDHandle );
}

/**
 * @brief   **Display a message recived by clockMsg on the LCD **
 *
 *  The function waits for a message on state idle, when a message arrives 
 *  it changes to STATE_PRINTH_MONTH where it calls the function month to get
 *  the first 3 letter of fila_1 to be modify with the initials of a month,
 *  then it changes the state to STATE_PRINTH_DAY where it separates the digits
 *  of the day and sums 48 to get the ascii value the next state is STATE_PRINTH_YEAR
 *  where for the msb it calls the function bcdToDecimal because that number is in BCD code
 *  once we got the correct value we do the same as we did on printh day for the msb and lsb
 *  then it goes to STATE_PRINTH_WDAY where it calls the function HEL_LCD_SetCursor to set 
 *  the cursor on the first position and then calls the function week to get the week initials
 *  is does the same as in function month but with other values, then we print the row with
 *  the function HEL_LCD_String, and we pass to STATE_PRINTH_HOUR where we call HEL_LCD_SetCursor
 *  to get the curor on the second row and then we get the values of the hours as we do on 
 *  year state, then we do the same for the next to states that are SSTATE_PRINTH_MINUTES
 *  and STATE_PRINTH_SECONDS and on seconds state we print the string on the lcd with HEL_LCD_String
 *  lastly we change the state to STATE_IDLE
 * 
 * @retval  None 
 */    
void Display_Task( void )
{
    switch(LCD_State)
    {
        case STATE_IDLE:
            if( ClockMsg.msg == DISPLAY_MESSAGE)
            {
                LCD_State = STATE_PRINTH_MONTH;
                ClockMsg.msg = NO_DISPLAY_MESSAGE;
            }
        break;

        case STATE_PRINTH_MONTH:
            month(&fila_1[1],ClockMsg.tm.tm_mon);
            LCD_State = STATE_PRINTH_DAY;
        break;

        case STATE_PRINTH_DAY:
            fila_1[5] = ((ClockMsg.tm.tm_mday / 10u) + 48u);
            fila_1[6] = ((ClockMsg.tm.tm_mday % 10u) + 48u);
            LCD_State =  STATE_PRINTH_YEAR;
        break;

        case STATE_PRINTH_YEAR:
            fila_1[8]   = ( (ClockMsg.tm.tm_year_msb / 10u) + 48u);
            fila_1[9]   = ( (ClockMsg.tm.tm_year_msb % 10u) + 48u);
            fila_1[10]  = ( (ClockMsg.tm.tm_year_lsb / 10u) + 48u);
            fila_1[11]  = ( (ClockMsg.tm.tm_year_lsb % 10u) + 48u);
            LCD_State = STATE_PRINTH_WDAY;
        break;

        case STATE_PRINTH_WDAY:
            (void)HEL_LCD_SetCursor(&LCDHandle,FIRST_ROW,0);
            week(&fila_1[13],ClockMsg.tm.tm_wday);
            (void)HEL_LCD_String(&LCDHandle, fila_1);
            LCD_State = STATE_PRINTH_HOUR;
        break;
        
        case STATE_PRINTH_HOUR:
            (void)HEL_LCD_SetCursor(&LCDHandle,SECOND_ROW,3 );
            fila_2[0] = ((ClockMsg.tm.tm_hour / 10u) + 48u);
            fila_2[1] = ((ClockMsg.tm.tm_hour % 10u) + 48u);
            LCD_State = STATE_PRINTH_MINUTES;
        break;

        case STATE_PRINTH_MINUTES:
            fila_2[3] = ((ClockMsg.tm.tm_min / 10u) + 48u);
            fila_2[4] = ((ClockMsg.tm.tm_min % 10u) + 48u);
            LCD_State = STATE_PRINTH_SECONDS;
        break;

        case STATE_PRINTH_SECONDS:
            fila_2[6] = ((ClockMsg.tm.tm_sec / 10u) + 48u);
            fila_2[7] = ((ClockMsg.tm.tm_sec % 10u) + 48u);
            (void)HEL_LCD_String(&LCDHandle, fila_2);
            
            LCD_State = STATE_IDLE;
        break;

        default:
            LCD_State = STATE_IDLE;
        break;
    }
}

/**
 * @brief   **This function gets a 3 letter abreviation of a month **
 *
 *  The Function recived the pointer *mon where its going to modify the string
 *  and the char pos that is the month abreviation that we want, then we substract
 *  1 to the pos since in the array 0 is JAN then we multiply by 4 to get the
 *  position of were we want the abrevation and we asigned the values with a for loop 
 *
 * @param   mon[in] Indicates the string that we want to modify
 * @param   pos[in] Indicates wich month abreviation we want
 *
 * @retval  None          
 */
void month(char *mon,char pos)
{
   char position = (pos-1)*4;
   char *mes = "JAN,FEB,MAR,APR,MAY,JUN,JUL,AUG,SEP,OCT,NOV,DEC," ; /* cppcheck-suppress misra-c2012-7.4 ; string need to be modify*/
   for(int i=0;i<4;i++)
   {
        *(mon+i)=*(mes+i+position);         /* cppcheck-suppress misra-c2012-18.4 ; operators to pointer is needed*/
   }
}

/**
 * @brief   **This function gets a 2 letter abreviation of the week **
 *
 *  The Function recived the pointer *mon where its going to modify the string
 *  and the char pos that is the week abreviation that we want, then we substract
 *  1 to the pos since in the array 0 is MO (monday) then we multiply by 3 to get the
 *  position of were we want the abrevation and we asigned the values with a for loop 
 *
 * @param   mon[in] Indicates the string that we want to modify
 * @param   pos[in] Indicates wich month abreviation we want
 *
 * @retval  None          
 */
void week(char *week,char pos)
{
  char position = (pos-1)*3;
   char *sem = "MO TU WE TH FR SA SU" ;     /* cppcheck-suppress misra-c2012-7.4 ; string need to be modify*/
   for(int i=0;i<3;i++)
   {
        *(week+i)=*(sem+i+position);        /* cppcheck-suppress misra-c2012-18.4 ; operators to pointer is needed*/
   }
}

