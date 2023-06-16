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
    CLOCK_ST_DISPLAY_MSG,
    CLOCK_ST_IDLE,
    CLOCK_ST_RECEPTION,
    CLOCK_ST_GET_MSG,
    CLOCK_ST_MESSAGE
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
static RTC_HandleTypeDef hrtc = {0};

/**
 * @brief  Variable for rtc Date configuration
 */
static RTC_DateTypeDef sDate;
/**
 * @brief  Variable for rtc Time configuration
 */
static RTC_TimeTypeDef sTime;
/**
 * @brief  Variable for concurrent process of displaying the date every second
 */
static uint32_t tick_1000ms;

/**
 * @brief  Variable for Alarm configuration
 */
static RTC_AlarmTypeDef sAlarm;

/**
 * @brief  Variable for clock state machine
 */
static uint8_t Clockstate = CLOCK_ST_IDLE; 


/**
* @brief  Circular buffer variable for CAN msg recived to serial task.
*/
QUEUE_HandleTypeDef CLOCK_queue;

static void Clock_StMachine(void);

/**
 * @brief   **This function intiates the RTC and the tick_1000ms variable**
 *
 *  The Function set the initialization parameters for the RTC module, which include the 24-hour
 *  format for the clock, the asynchronous prescaler value of 0x7F and the synchronous prescaler value of 0xFF.
 *  The asynchronous and synchronous prescalers divide the input clock to get a frequency of 1hz. 
 *  Then we call the function to initiate the RTC with this parameters. 
 *  Then we set the parameters to set the time to 2:00:00 and date to Monday, April 17, 2023
 *  And we initialize the tick_1000ms for the concurrent process.
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
    sTime.Seconds    = 0x25;
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
    
    sAlarm.AlarmTime.Hours = 0x00;      
    sAlarm.AlarmTime.Minutes = 0x00;    
    sAlarm.AlarmTime.Seconds = 0x00;    
    sAlarm.AlarmTime.SubSeconds = 0x00; 
    sAlarm.Alarm = RTC_ALARM_A;         

    Status = HAL_RTC_DeactivateAlarm(&hrtc, RTC_ALARM_A);
    assert_error( Status == HAL_OK, RTC_SDESACTIVATE_ALARM_ERROR ); /* cppcheck-suppress misra-c2012-11.8 ; function cannot be modify */
    
    tick_1000ms=HAL_GetTick();

    /*Clock to display buffer*/
    static APP_MsgTypeDef clock_queue_store[CLOCK_DATA_PER50MS];
    CLOCK_queue.Buffer = clock_queue_store;
    CLOCK_queue.Elements = CLOCK_DATA_PER50MS;
    CLOCK_queue.size = sizeof(APP_MsgTypeDef);
    HIL_QUEUE_Init(&CLOCK_queue);

    Timer_TypeDef hsche_timer[1];
    sched.timers   = 1;
    sched.timerPtr = hsche_timer;
    HIL_SCHEDULER_RegisterTimer( &sched, 1000, NULL );
    HIL_SCHEDULER_StartTimer( &sched,1);
}

/**
* @brief   **This function executes the clock state machine**
*
* This functions executes the state machine of the clock task
* every 50ms, we do this because a circular buffer has been implemented on the serial and clock
* task, this means that we do need to execute every time the task since now the 
* information is being stored.     
*
*/
void Clock_Task( void )
{    
    /*poll the state machine until the queue is empty and it return to IDLE*/
    Clockstate = CLOCK_ST_RECEPTION;
    while( Clockstate != (uint8_t)CLOCK_ST_IDLE )
    {
        /*run the state machine to process the messages*/
        Clock_StMachine();
    }
}

/**
* @brief   **This function runs a machine state to change parameters and display the time every second**
*
*  The firs state of the machine is the IDLE state where it cheks if a msg is recive or if
*  a second has already pass, if a second has pass the state changes to DIPLAY_MSG where 
*  the time is display in the terminal with semi-hosting and then go back to IDLE state.
*  if a msg is recive Clockstate changes to GET_MSG where it compares the value to see what msg
*  arrives, if the msg is equal to CHANGE_TIME clockstate is change to CHANGE_TIME where it changes the
*  value of the time on the RTC and them changes clockstate to DISPLAY_MSG, The same is done on CHANGE_DATE and
*  CHANGE_ALARM but with date and alarm parameters.
*   
*/
void Clock_StMachine(void)
{
    static APP_MsgTypeDef ClockMsg;
    static APP_MsgTypeDef CAN_to_clock_message;
     switch(Clockstate)
    {
        case CLOCK_ST_IDLE:
            break;

        case CLOCK_ST_RECEPTION:
            if( HIL_SCHEDULER_GetTimer( &sched, 1 ) == 0)
            {
                Clockstate  = CLOCK_ST_DISPLAY_MSG;
            }
            else if(HIL_QUEUE_IsEmpty(&SERIAL_queue) == NOT_EMPTY)
            {
                (void)HIL_QUEUE_Read(&SERIAL_queue,&CAN_to_clock_message);
                if(CAN_to_clock_message.msg != (uint8_t)NOT_MESSAGE)
                {
                    Clockstate = CLOCK_ST_GET_MSG;
                }
            }
            else
            {
                Clockstate  = CLOCK_ST_IDLE;
            }
            break;
        case CLOCK_ST_GET_MSG:
        {

            if(CAN_to_clock_message.msg==(uint8_t)CLOCK_ST_CHANGE_TIME)
            {
                Clockstate = CLOCK_ST_CHANGE_TIME;
            }
            else if (CAN_to_clock_message.msg==(uint8_t)CLOCK_ST_CHANGE_DATE)
            {
                Clockstate = CLOCK_ST_CHANGE_DATE;
            }
            else if (CAN_to_clock_message.msg==(uint8_t)CLOCK_ST_CHANGE_ALARM)
            {
                Clockstate = CLOCK_ST_CHANGE_ALARM;
            }
            else
            {
             CAN_to_clock_message.msg = NOT_MESSAGE;
            }
            break;

        }
            
        case CLOCK_ST_DISPLAY_MSG:
        {
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

            ClockMsg.msg = DISPLAY_MESSAGE;
            (void)HIL_QUEUE_Write( &CLOCK_queue, &ClockMsg );
            if(HIL_QUEUE_IsEmpty(&SERIAL_queue) == NOT_EMPTY)
            {
                Clockstate = CLOCK_ST_RECEPTION;
            }
            else
            {
                Clockstate = CLOCK_ST_IDLE;  
            }
            break;
        }

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
            Clockstate=CLOCK_ST_DISPLAY_MSG;
            CAN_to_clock_message.msg  = NOT_MESSAGE;
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

            Clockstate          = CLOCK_ST_DISPLAY_MSG;
            CAN_to_clock_message.msg  = NOT_MESSAGE;
            break;
        }
        
        case CLOCK_ST_CHANGE_ALARM:
        {
            sAlarm.AlarmTime.Hours      = CAN_to_clock_message.tm.tm_hour;      
            sAlarm.AlarmTime.Minutes    = CAN_to_clock_message.tm.tm_min;    
            sAlarm.AlarmTime.Seconds    = CAN_to_clock_message.tm.tm_sec;    
            sAlarm.AlarmTime.SubSeconds = 0x00; 
            sAlarm.Alarm = RTC_ALARM_A;         
            Status = HAL_RTC_DeactivateAlarm(&hrtc, RTC_ALARM_A);
            assert_error( Status == HAL_OK, RTC_SDESACTIVATE_ALARM_ERROR ); /* cppcheck-suppress misra-c2012-11.8 ; function cannot be modify */
            Status = HAL_RTC_SetAlarm_IT(&hrtc, &sAlarm, RTC_FORMAT_BCD);
            assert_error( Status == HAL_OK, RTC_SET_ALARM_ERROR );  /* cppcheck-suppress misra-c2012-11.8 ; function cannot be modify */
            CAN_to_clock_message.msg = NOT_MESSAGE;
            Clockstate  = CLOCK_ST_DISPLAY_MSG;
            break;
        }
        default:
        {
            Clockstate = CLOCK_ST_IDLE;
            CAN_to_clock_message.msg = NOT_MESSAGE;
            break;
        }
    }

}