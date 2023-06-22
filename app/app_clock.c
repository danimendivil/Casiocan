#include "app_clock.h"
#include "hil_queue.h"

/**
 * @brief CLock State machine states.
 *
 * This enumeration represents the various types of states of the machine
 */
typedef enum
/* cppcheck-suppress misra-c2012-2.4 ; enum is used on state machine */
{
    NOT_MESSAGE = 0u,
    CLOCK_ST_CHANGE_TIME,
    CLOCK_ST_CHANGE_DATE,
    CLOCK_ST_CHANGE_ALARM,
} CLOCK_STATES;

/** 
  * @defgroup ms time for clock task periodicity and data for circular buffer.
  @{ */
#define CLOCK_DATA_PER50MS      50    /*!< Max number of clock transmitions per 50 ms*/
/**
  @} */

/**
* @brief  Variable for rtc configuration
*/
RTC_HandleTypeDef hrtc = {0};       /* cppcheck-suppress misra-c2012-5.8 ; other declaration is not used */

/**
 * @brief  Variable for rtc Date configuration
 */
static RTC_DateTypeDef sDate;
/**
 * @brief  Variable for rtc Time configuration
 */
static RTC_TimeTypeDef sTime;

/**
 * @brief  Variable for Alarm configuration
 */
static RTC_AlarmTypeDef sAlarm;

/**
 * @brief  Variable for serial to clock messages
 */
static APP_MsgTypeDef CAN_to_clock_message;

/**
* @brief  Circular buffer variable for CAN msg recived to serial task.
*/
QUEUE_HandleTypeDef CLOCK_queue;

static void Clock_StMachine(uint8_t Clockstate);


/**
 * @brief   **This function intiates the RTC**
 *
 *  The Function set the initialization parameters for the RTC module, which include the 24-hour
 *  format for the clock, the asynchronous prescaler value of 0x7F and the synchronous prescaler value of 0xFF.
 *  The asynchronous and synchronous prescalers divide the input clock to get a frequency of 1hz. 
 *  Then we call the function to initiate the RTC with this parameters. 
 *  Then we set the parameters to set the time to 2:00:00 and date to Monday, April 17, 2023
 *  For the size of the buffer we will take the max amount of msgs that the serial
 *  task can send in 50ms.
 *  the serial task has a max of 10 transmitions per 10 ms, making the conversion 
 *  the max amount of messages that clock task will be reciving is 10 transmitions times 5
 *  to make up the 50 ms so the buffer size is going to be 50.
 *  
 */
 
void Clock_Init( void )      
{
    /*declare as global variable or static*/
    hrtc.Instance             = RTC;
    hrtc.Init.HourFormat      = RTC_HOURFORMAT_24;
    hrtc.Init.AsynchPrediv    = 0x7F;
    hrtc.Init.SynchPrediv     = 0xFF;
    hrtc.Init.OutPut          = RTC_OUTPUT_DISABLE;
    /* initilize the RTC with 24 hour format and no output signal enble */
    Status = HAL_RTC_Init( &hrtc );
    assert_error( Status == HAL_OK, RTC_INIT_ERROR );   /* cppcheck-suppress misra-c2012-11.8 ; function cannot be modify */

    sTime.Hours      = 0x02;
    sTime.Minutes    = 0x20;
    sTime.Seconds    = 0x55;
    sTime.SubSeconds = 0x00;
    sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
    sTime.StoreOperation = RTC_STOREOPERATION_RESET;
    
    Status = HAL_RTC_SetTime( &hrtc, &sTime, RTC_FORMAT_BCD );
    assert_error( Status == HAL_OK, RTC_SETTIME_ERROR );    /* cppcheck-suppress misra-c2012-11.8 ; function cannot be modify */
    
    sDate.WeekDay = RTC_WEEKDAY_MONDAY;
    sDate.Month   = RTC_MONTH_APRIL;
    sDate.Date    = 0x11;
    sDate.Year    = 0x22;
    
    Status = HAL_RTC_SetDate( &hrtc, &sDate, RTC_FORMAT_BCD );
    assert_error( Status == HAL_OK, RTC_SETDATE_ERROR );    /* cppcheck-suppress misra-c2012-11.8 ; function cannot be modify */
    
    sAlarm.AlarmTime.Hours          = FALSE;
    sAlarm.AlarmTime.Minutes        = FALSE;
    sAlarm.AlarmTime.Seconds        = FALSE;
    sAlarm.AlarmTime.SubSeconds     = FALSE;
    sAlarm.AlarmTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
    sAlarm.AlarmTime.StoreOperation = RTC_STOREOPERATION_RESET;
    sAlarm.AlarmMask                = RTC_ALARMMASK_DATEWEEKDAY;;
    sAlarm.AlarmDateWeekDaySel      = RTC_ALARMDATEWEEKDAYSEL_DATE;
    sAlarm.AlarmDateWeekDay         = RTC_WEEKDAY_MONDAY;
    sAlarm.Alarm                    = RTC_ALARM_A;        
    
    
    Status = HAL_RTC_DeactivateAlarm(&hrtc, RTC_ALARM_A);
    assert_error( Status == HAL_OK, RTC_SDESACTIVATE_ALARM_ERROR ); /* cppcheck-suppress misra-c2012-11.8 ; function cannot be modify */
    
    /*Clock to display buffer*/
    static APP_MsgTypeDef clock_queue_store[CLOCK_DATA_PER50MS];
    CLOCK_queue.Buffer = clock_queue_store;
    CLOCK_queue.Elements = CLOCK_DATA_PER50MS;
    CLOCK_queue.size = sizeof(APP_MsgTypeDef);
    HIL_QUEUE_Init(&CLOCK_queue);

    Alarm_State = ALARM_OFF;
}

/**
* @brief   **This function executes the clock state machine**
*
*   This functions executes the state machine of the clock task
*   every 50ms, we do this because a circular buffer has been implemented on the serial and clock
*   task, this means that we do need to execute every time the task since now the 
*   information is being stored on the circular buffer.
*   will be using the HIL_QUEUE_IsEmpty to see if the circular buffer has any message and if it does
*   then it will read the message and call the function Clock_StMachine with the value
*   CAN_to_clock_message.msg wich is the action to be taken.      
*
*/
void Clock_Task( void )
{    
    while( HIL_QUEUE_IsEmptyISR( &SERIAL_queue, RTC_TAMP_IRQn ) == NOT_EMPTY )
    {
        /*Read the first message*/
        (void)HIL_QUEUE_ReadISR( &SERIAL_queue, &CAN_to_clock_message, RTC_TAMP_IRQn);
        Clock_StMachine(CAN_to_clock_message.msg);
    }
}

/**
* @brief   **This function runs a machine state to change parameters and display the time every second**
*
*  This function configures the rtc values depending on the message clockstate recived,
*  the values can be CLOCK_ST_CHANGE_TIME to change the time, CLOCK_ST_CHANGE_DATE
*  to change the date and  CLOCK_ST_CHANGE_ALARM to change the alarm, if any other value is recived the 
*  function will do nothing also after changing the values of the rtc it gives the variable Display
*  a true value wich will call Display_msg function.
*   
*/
void Clock_StMachine(uint8_t Clockstate)
{
    uint8_t Display = FALSE;
    switch(Clockstate)
    {   
        case CLOCK_ST_CHANGE_TIME:
        {
            sTime.Hours          = CAN_to_clock_message.tm.tm_hour;
            sTime.Minutes        = CAN_to_clock_message.tm.tm_min;
            sTime.Seconds        = CAN_to_clock_message.tm.tm_sec;
            sTime.SubSeconds     = 0x00;
            sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
            sTime.StoreOperation = RTC_STOREOPERATION_RESET;
            
            Status = HAL_RTC_SetTime( &hrtc, &sTime, RTC_FORMAT_BCD );
            assert_error( Status == HAL_OK, RTC_SETTIME_ERROR );    /* cppcheck-suppress misra-c2012-11.8 ; function cannot be modify */
            
            Display = TRUE;
            break;
        }
        
        case CLOCK_ST_CHANGE_DATE:
        {
            sDate.WeekDay   = CAN_to_clock_message.tm.tm_wday;
            sDate.Month     = CAN_to_clock_message.tm.tm_mon;
            sDate.Date      = CAN_to_clock_message.tm.tm_mday;
            sDate.Year      = CAN_to_clock_message.tm.tm_year_lsb;

            Status = HAL_RTC_SetDate( &hrtc, &sDate, RTC_FORMAT_BCD );
            assert_error( Status == HAL_OK, RTC_SETDATE_ERROR );        /* cppcheck-suppress misra-c2012-11.8 ; function cannot be modify */

            Display = TRUE;
            break;
        }
        
        case CLOCK_ST_CHANGE_ALARM:
        {
            sAlarm.AlarmTime.Hours      = CAN_to_clock_message.tm.tm_hour;      
            sAlarm.AlarmTime.Minutes    = CAN_to_clock_message.tm.tm_min;              
            Status = HAL_RTC_DeactivateAlarm(&hrtc, RTC_ALARM_A);
            assert_error( Status == HAL_OK, RTC_SDESACTIVATE_ALARM_ERROR ); /* cppcheck-suppress misra-c2012-11.8 ; function cannot be modify */
            Status = HAL_RTC_SetAlarm_IT(&hrtc, &sAlarm, RTC_FORMAT_BCD);
            assert_error( Status == HAL_OK, RTC_SET_ALARM_ERROR );  /* cppcheck-suppress misra-c2012-11.8 ; function cannot be modify */
            Alarm_State = ALARM_ON;
            Display = TRUE;
            break;
        }
        default:
            break;
        
    }
    if(Display == TRUE)
    {
        Display_msg();
    }
}

/**
* @brief   **This function send a message to app_display**
*
*  The function first gets the data of the rtc and stores it on the ClockMsg
*  variable then gives ClockMsg.msg the DISPLAY_MESSAGE value wich tells the 
*  app_display to display data on the lcd and sends this message through the 
*  circular buffer. 
*  this function will also be called every second by the software timer configured
*  on the main function.   
*  if the button is pressed this function will also send the alarm data
*/
void Display_msg(void)
{
    APP_MsgTypeDef ClockMsg;

    /* Get the RTC current Time */
    Status = HAL_RTC_GetTime( &hrtc, &sTime, RTC_FORMAT_BIN );
    assert_error( Status == HAL_OK, RTC_GET_TIME_ERROR );       /* cppcheck-suppress misra-c2012-11.8 ; function cannot be modify */
    /* Get the RTC current Date */
    Status = HAL_RTC_GetDate( &hrtc, &sDate, RTC_FORMAT_BIN );
    assert_error( Status == HAL_OK, RTC_GET_DATE_ERROR );       /* cppcheck-suppress misra-c2012-11.8 ; function cannot be modify */
    ClockMsg.tm.tm_year_msb = CAN_to_clock_message.tm.tm_year_msb;
    ClockMsg.tm.tm_mon = sDate.Month;
    ClockMsg.tm.tm_mday = sDate.Date;
    ClockMsg.tm.tm_year_lsb = sDate.Year;
    ClockMsg.tm.tm_wday = sDate.WeekDay;

    ClockMsg.tm.tm_hour = sTime.Hours;
    ClockMsg.tm.tm_min = sTime.Minutes;
    ClockMsg.tm.tm_sec = sTime.Seconds;

    if (button == TRUE)
    {
        HAL_RTC_GetAlarm(&hrtc, &sAlarm, RTC_ALARM_A, RTC_FORMAT_BIN);
        ClockMsg.tm.tm_hour_alarm = sAlarm.AlarmTime.Hours;
        ClockMsg.tm.tm_min_alarm = sAlarm.AlarmTime.Minutes;
    }
    ClockMsg.msg = DISPLAY_MESSAGE;
    (void)HIL_QUEUE_WriteISR( &CLOCK_queue, &ClockMsg, RTC_TAMP_IRQn );
}