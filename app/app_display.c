#include "app_display.h"
#include "hel_lcd.h"
#include "hil_queue.h"

/** 
  * @defgroup Alarm state defines .
  @{ */
#define ONE_MINUTE          60      /*!<value for counter to 60 seconds*/    
#define PWM_50              800     /*!<value for 50% of pwm*/
#define PWM_0               0       /*!<value for 0% of pwm*/
#define EVEN_SECONDS        2       /*!<value for use for even numbers*/
/**
  @} */

/**
 * @brief  Variable for LCD configuration
 */
static LCD_HandleTypeDef LCDHandle;

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
 *  then calls the function HEL_LCD_Init wich is a routine for the LCD
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
    TimHandle.Init.Prescaler     = 10;
    TimHandle.Init.Period        = 1600*2;
    TimHandle.Init.CounterMode   = TIM_COUNTERMODE_UP;
    HAL_TIM_PWM_Init(&TimHandle);

    /* Configuramos canal para generar una señal PWM con polaridad en alto */
    sConfig.OCMode     = TIM_OCMODE_PWM1;
    sConfig.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfig.OCFastMode = TIM_OCFAST_DISABLE;
    sConfig.Pulse = 160;
    HAL_TIM_PWM_ConfigChannel( &TimHandle, &sConfig, TIM_CHANNEL_1 );
    HAL_TIM_PWM_Start( &TimHandle, TIM_CHANNEL_1 );
    __HAL_TIM_SET_COMPARE( &TimHandle, TIM_CHANNEL_1, 0 );
}

static APP_MsgTypeDef clock_display;
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
 *  the fisrt part is STATE_PRINTH_MONTH where it calls the function month to get
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

void HAL_GPIO_EXTI_Falling_Callback( uint16_t GPIO_Pin )
{
    if(Alarm_State == ALARM_ACTIVE)
    {
       Alarm_Flag = TRUE; 
    }
    button = TRUE;
}

/* when the pin changes from low to high this function will be called by HAL_GPIO_EXTI_IRQHandler
which is in turn called from the EXTI4_15_IRQHandler interrupt vector*/
void HAL_GPIO_EXTI_Rising_Callback( uint16_t GPIO_Pin )
{
    Status = HEL_LCD_SetCursor(&LCDHandle,SECOND_ROW,0 );
    assert_error( Status == HAL_OK, SPI_SET_CURSOR_ERROR ); /* cppcheck-suppress misra-c2012-11.8 ; function cannot be modify */
    Status = HEL_LCD_String(&LCDHandle, "               ");
    assert_error( Status == HAL_OK, SPI_STRING_ERROR ); /* cppcheck-suppress misra-c2012-11.8 ; function cannot be modify */
    button = FALSE; 
}
    



