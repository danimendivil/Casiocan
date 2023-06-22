#include "app_display.h"
#include "hel_lcd.h"
#include "hil_queue.h"

/** 
  * @defgroup Alarm state defines .
  @{ */
#define ONE_MINUTE          60      /*!<value for counter to 60 seconds*/    
#define EVEN_SECONDS        2       /*!<value for use for even numbers*/
/**
  @} */
/** 
  * @defgroup PWM defines .
  @{ */
#define PERIOD_1KHZ             3200     /*!<value for counter to 60 seconds*/    
#define PREESCALER_1KHZ         10      /*!<value for counter to 60 seconds*/
#define PWM_50                  500     /*!<value for 50% of pwm*/
#define PWM_0                   0       /*!<value for 0% of pwm*/
/**
  @} */

/**
 * @brief  Variable for LCD configuration
 */
static LCD_HandleTypeDef LCDHandle;

/**
 * @brief  Variable to read the buffer
 */
static APP_MsgTypeDef clock_display;

/**
* @brief  Variable for button state
*/
uint8_t button;

static void month(char *mon,char pos);
static void week(char *week,char pos);
static void Display_StMachine(void);

/**
 * @brief  Variable for the pwm timer
 */
static TIM_HandleTypeDef TimHandle;    
      
/**
 * @brief   **This function intiates the LCD and the SPI **
 *
 *  The Function sets the pins for the SPI and LCD  needed and initialize by 
 *  calling the HEL_LCD_MspInit and configuring  and initializing the SPI
 *  then calls the function HEL_LCD_Init wich is a routine for the LCD.
 *  this function also eneables the pin 7 of the gpio B wich is a button
 *  and eneables the interrupt of the button on falling and rising.
 *  it also eneables a pwm that is conected to a buzzer 
 *  the calculations for the pwm are:
 *  PWM frequency = (timer clock / (Prescaler * Period + 1))
 *  PWM frequency = (32Mhz / (10 * 3200 + 1)) = 1kHz
 */
void Display_Init( void )
{
    static SPI_HandleTypeDef SpiHandle;
    TIM_OC_InitTypeDef sConfig;
    GPIO_InitTypeDef GPIO_InitStruct;

    __HAL_RCC_GPIOB_CLK_ENABLE();

    GPIO_InitStruct.Pin = GPIO_PIN_7;               /*pin to set as output*/
    GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;     /*input on mode faling edge interrupt*/
    GPIO_InitStruct.Pull = GPIO_NOPULL;             /*no pull-up niether pull-down*/
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;    /*pin speed*/
    
    HAL_GPIO_Init( GPIOB, &GPIO_InitStruct );
    
    HAL_NVIC_SetPriority( EXTI4_15_IRQn, 2, 0 );
    HAL_NVIC_EnableIRQ( EXTI4_15_IRQn );
    
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
    Status = HAL_SPI_Init( LCDHandle.SpiHandler );
    assert_error( Status == HAL_OK, SPI_INIT_ERROR );       /* cppcheck-suppress misra-c2012-11.8 ; function cannot be modify */
    Status = HEL_LCD_Init(&LCDHandle );
    assert_error( Status == HAL_OK, SPI_COMMAND_ERROR );        /* cppcheck-suppress misra-c2012-11.8 ; function cannot be modify */

     /* Configuramos el timer TIM1 para generar un periodo de 1ms */
    TimHandle.Instance = TIM14;
    TimHandle.Init.Prescaler     = PREESCALER_1KHZ;
    TimHandle.Init.Period        = PERIOD_1KHZ;
    TimHandle.Init.CounterMode   = TIM_COUNTERMODE_UP;
    HAL_TIM_PWM_Init(&TimHandle);

    /* Configuramos canal para generar una señal PWM con polaridad en alto */
    sConfig.OCMode     = TIM_OCMODE_PWM1;
    sConfig.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfig.OCFastMode = TIM_OCFAST_DISABLE;
    sConfig.Pulse = 0;
    HAL_TIM_PWM_ConfigChannel( &TimHandle, &sConfig, TIM_CHANNEL_1 );
    HAL_TIM_PWM_Start( &TimHandle, TIM_CHANNEL_1 );
}


/**
* @brief   **This function executes the display state machine**
*
* This functions executes the state machine of the display task
* every 100ms, we do this because a circular buffer has been implemented on the serial and clock
* task, this means that we do need to execute every time the task since now the 
* information is being stored, the function waits for the circular buffer to geet data
* and then reads the msg and checks if its DISPLAY_MESSAGE and calls the function  Display_StMachine.   
*
*/void Display_Task( void )
{
    while( HIL_QUEUE_IsEmptyISR(&CLOCK_queue,SPI1_IRQn) == NOT_EMPTY )
    {
        /*Read the first message*/
        (void)HIL_QUEUE_ReadISR(&CLOCK_queue,&clock_display,SPI1_IRQn);
        if( clock_display.msg == DISPLAY_MESSAGE)
        {
            Display_StMachine();
        }
    }
}

/**
* @brief   **Display a message recived by clock_display on the LCD **
*
*   this function starts by displaying the date on the first row of the lcd
*   the data show by the lcd is read through the circular buffer on the display task function
*   alse when data is going to be display we add 48 to get the correct ascii character,
*   once the first row is displayed we check if the alarm is not actived, if its not active
*   then we need to check if the button its not pressed, we checked if an alarm has been set, 
*   if it has been set we print an A on the lcd if not then we print nothing, then we print the time
*   like we did with the date, if the button was pressed then instead of printing the time on the second row
*   we print if an alarm has been set or the time of the alarm that has been set.
*   And if the alarm is actived we print alarm!!! and set the pwm to 800 wich is 50% of the pwm to make a sound
*   and since this function is called every second we add a counter to see how long the alarm is going to run
*   and on even numbers we turned off the pwm and odd numbers we turn on the pwm.
*   the alarm part of the function will run till the counter gets to 60 seconds or the flag value is changed
*   to true that can be done with the button or by sending a message in CAN, and when the alarm is turned off
*   we change the value of the alarm state to OFF, reset the counter value and the alarm flag.
* @retval  None 
*/
void Display_StMachine(void)
{
    static char fila_2[] = "00:00:00 ";  /* cppcheck-suppress misra-c2012-7.4 ; string need to be modify*/
    static char fila_1[] =" XXX,XX XXXX XX ";   /* cppcheck-suppress misra-c2012-7.4 ; string need to be modify*/
    static uint8_t alarm_counter = 0;
    //DISPLAY_STATE_PRINTH_MONTH:
    month(&fila_1[1],clock_display.tm.tm_mon);
            
    //DISPLAY_STATE_PRINTH_DAY:
    fila_1[5] = ((clock_display.tm.tm_mday / 10u) + 48u);
    fila_1[6] = ((clock_display.tm.tm_mday % 10u) + 48u);
            
    //DISPLAY_STATE_PRINTH_YEAR:
    fila_1[8]   = ( (clock_display.tm.tm_year_msb / 10u) + 48u);
    fila_1[9]   = ( (clock_display.tm.tm_year_msb % 10u) + 48u);
    fila_1[10]  = ( (clock_display.tm.tm_year_lsb / 10u) + 48u);
    fila_1[11]  = ( (clock_display.tm.tm_year_lsb % 10u) + 48u);
            
    //DISPLAY_STATE_PRINTH_WDAY:
    Status = HEL_LCD_SetCursor(&LCDHandle,FIRST_ROW,0);
    assert_error( Status == HAL_OK, SPI_SET_CURSOR_ERROR ); /* cppcheck-suppress misra-c2012-11.8 ; function cannot be modify */
    week(&fila_1[13],clock_display.tm.tm_wday);
    Status = HEL_LCD_String(&LCDHandle, fila_1);
    assert_error( Status == HAL_OK, SPI_STRING_ERROR ); /* cppcheck-suppress misra-c2012-11.8 ; function cannot be modify */

    if(Alarm_State != ALARM_ACTIVE)
    {
        if (button == FALSE)
        {
            if(Alarm_State == ALARM_ON)  
            {
                Status = HEL_LCD_SetCursor(&LCDHandle,SECOND_ROW,0 );
                assert_error( Status == HAL_OK, SPI_SET_CURSOR_ERROR ); /* cppcheck-suppress misra-c2012-11.8 ; function cannot be modify */
                HEL_LCD_Data( &LCDHandle, 'A' );
            }  
            fila_2[5] =':';
            //DISPLAY_STATE_PRINTH_HOUR:
            Status = HEL_LCD_SetCursor(&LCDHandle,SECOND_ROW,3 );
            assert_error( Status == HAL_OK, SPI_SET_CURSOR_ERROR ); /* cppcheck-suppress misra-c2012-11.8 ; function cannot be modify */
            fila_2[0] = ((clock_display.tm.tm_hour / 10u) + 48u);
            fila_2[1] = ((clock_display.tm.tm_hour % 10u) + 48u);
                    
            //DISPLAY_STATE_PRINTH_MINUTES:
            fila_2[3] = ((clock_display.tm.tm_min / 10u) + 48u);
            fila_2[4] = ((clock_display.tm.tm_min % 10u) + 48u);
                    
            //DISPLAY_STATE_PRINTH_SECONDS:
            fila_2[6] = ((clock_display.tm.tm_sec / 10u) + 48u);
            fila_2[7] = ((clock_display.tm.tm_sec % 10u) + 48u);
            Status = HEL_LCD_String(&LCDHandle, fila_2);
            assert_error( Status == HAL_OK, SPI_STRING_ERROR ); /* cppcheck-suppress misra-c2012-11.8 ; function cannot be modify */
        }
        else
        {
            if (Alarm_State == ALARM_OFF)
            {
            Status = HEL_LCD_SetCursor(&LCDHandle,SECOND_ROW,0 );
            assert_error( Status == HAL_OK, SPI_SET_CURSOR_ERROR ); /* cppcheck-suppress misra-c2012-11.8 ; function cannot be modify */
            Status = HEL_LCD_String(&LCDHandle, "ALARM NO CONFIG");
            assert_error( Status == HAL_OK, SPI_STRING_ERROR ); /* cppcheck-suppress misra-c2012-11.8 ; function cannot be modify */
            }
            else
            {
                fila_2[0] = ((clock_display.tm.tm_hour_alarm / 10u) + 48u);
                fila_2[1] = ((clock_display.tm.tm_hour_alarm % 10u) + 48u);
                fila_2[3] = ((clock_display.tm.tm_min_alarm / 10u) + 48u);
                fila_2[4] = ((clock_display.tm.tm_min_alarm % 10u) + 48u);
                fila_2[5] =' ';
                fila_2[6] =' ';
                Status = HEL_LCD_SetCursor(&LCDHandle,SECOND_ROW,0 );
                assert_error( Status == HAL_OK, SPI_SET_CURSOR_ERROR ); /* cppcheck-suppress misra-c2012-11.8 ; function cannot be modify */
                Status = HEL_LCD_String(&LCDHandle, "   ALARM=");
                assert_error( Status == HAL_OK, SPI_STRING_ERROR ); /* cppcheck-suppress misra-c2012-11.8 ; function cannot be modify */
                Status = HEL_LCD_String(&LCDHandle, fila_2);
                assert_error( Status == HAL_OK, SPI_STRING_ERROR ); /* cppcheck-suppress misra-c2012-11.8 ; function cannot be modify */
            }
        }
    }
    else
    {
        alarm_counter++;
        Status = HEL_LCD_SetCursor(&LCDHandle,SECOND_ROW,0 );
        assert_error( Status == HAL_OK, SPI_SET_CURSOR_ERROR ); /* cppcheck-suppress misra-c2012-11.8 ; function cannot be modify */
        Status = HEL_LCD_String(&LCDHandle, "    ALARM!!!");
        assert_error( Status == HAL_OK, SPI_STRING_ERROR ); /* cppcheck-suppress misra-c2012-11.8 ; function cannot be modify */
        HEL_LCD_Backlight(&LCDHandle, TOGGLE);

        if (alarm_counter % EVEN_SECONDS == FALSE)
        {
            __HAL_TIM_SET_COMPARE( &TimHandle, TIM_CHANNEL_1, PWM_50 );
        }
        else
        {
            __HAL_TIM_SET_COMPARE( &TimHandle, TIM_CHANNEL_1, PWM_0 );
        }
        if(Alarm_Flag == TRUE)
        {
            Alarm_Flag = FALSE;
            alarm_counter = ONE_MINUTE;
        }
        if(alarm_counter == ONE_MINUTE)
        {
            HEL_LCD_Backlight(&LCDHandle, ON);
            alarm_counter = FALSE;
            Alarm_State = ALARM_OFF;
            Status = HEL_LCD_SetCursor(&LCDHandle,SECOND_ROW,0 );
            assert_error( Status == HAL_OK, SPI_SET_CURSOR_ERROR ); /* cppcheck-suppress misra-c2012-11.8 ; function cannot be modify */
            Status = HEL_LCD_String(&LCDHandle, "               ");
            assert_error( Status == HAL_OK, SPI_STRING_ERROR ); /* cppcheck-suppress misra-c2012-11.8 ; function cannot be modify */
            __HAL_TIM_SET_COMPARE( &TimHandle, TIM_CHANNEL_1, PWM_0 );
        }  
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


/**
 * @brief   **Interruption for falling gpio pin 7 **
 *
 *  This function will be called as an interruption when a falling event 
 *  happens on the gpio pin 7 B.
 *  if the alarm is active this function will tell the alarm flag to stop the alarm
 *  also puts the button value on true to display the alarm status on the second row. 
 *         
 */
void HAL_GPIO_EXTI_Falling_Callback( uint16_t GPIO_Pin )
{
    if(Alarm_State == ALARM_ACTIVE)
    {
       Alarm_Flag = TRUE; 
    }
    button = TRUE;
}

/**
* @brief   **Interruption for falling gpio pin 7 **
*
*  This function will be called as an interruption when a rising event 
*  happens on the gpio pin 7 B.
*  this function will ereased the second row of the lcd, this is because
*  no mather when this button is pressed ereasing the second row is necesary
*  also puts the button on false wich will tell other functions that the button is not pressed        
*/
void HAL_GPIO_EXTI_Rising_Callback( uint16_t GPIO_Pin )
{
    Status = HEL_LCD_SetCursor(&LCDHandle,SECOND_ROW,0 );
    assert_error( Status == HAL_OK, SPI_SET_CURSOR_ERROR ); /* cppcheck-suppress misra-c2012-11.8 ; function cannot be modify */
    Status = HEL_LCD_String(&LCDHandle, "               ");
    assert_error( Status == HAL_OK, SPI_STRING_ERROR ); /* cppcheck-suppress misra-c2012-11.8 ; function cannot be modify */
    button = FALSE; 
}
    



