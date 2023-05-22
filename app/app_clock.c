#include "app_clock.h"
#include <stdio.h>      /* cppcheck-suppress misra-c2012-21.6 ; the stdio.h is necesary */

typedef enum
/* cppcheck-suppress misra-c2012-2.4 ; enum is used on state machine */
{
    NOT_MESSAGE = 0u,
    DISPLAY_MSG,
    CHANGE_TIME,
    CHANGE_DATE,
    CHANGE_ALARM,
    IDLE,
    GET_MSG,
    MESSAGE
}CLOCK_STATES;

RTC_HandleTypeDef hrtc = {0};
RTC_DateTypeDef sDate;
RTC_TimeTypeDef sTime;
int tick_1000ms;
int Clockstate = IDLE;

/**
 * @brief   **This function intiates the RTC and the tick_1000ms variable**
 *
 *  The Function set the initialization parameters for the RTC module, which include the 24-hour
 *  format for the clock, the asynchronous prescaler value of 0x7F and the synchronous prescaler value of 0xFF.
 *  The asynchronous and synchronous prescalers divide the input clock to get a frequency of 1hz. 
 *  Then we call the function to initiate the RTC with this parameters. 
 *  Then we set the parameters to set the time to 2:00:00 and date to Monday, April 17, 2023
 *  And we initialize the tick_1000ms for the concurrent process
 * 
 * @retval  none 
 */
 
void Clock_Init( void )      
{
     HAL_Init(); 
    
    
    /*declare as global variable or static*/
    

    hrtc.Instance             = RTC;
    hrtc.Init.HourFormat      = RTC_HOURFORMAT_24;
    hrtc.Init.AsynchPrediv    = 0x7F;
    hrtc.Init.SynchPrediv     = 0xFF;
    hrtc.Init.OutPut          = RTC_OUTPUT_DISABLE;
    /* initilize the RTC with 24 hour format and no output signal enble */
    HAL_RTC_Init( &hrtc );
    

    sTime.Hours      = 0x02;
    sTime.Minutes    = 0x00;
    sTime.Seconds    = 0x00;
    sTime.SubSeconds = 0x00;
    sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
    sTime.StoreOperation = RTC_STOREOPERATION_RESET;
    
    HAL_RTC_SetTime( &hrtc, &sTime, RTC_FORMAT_BCD );
    

    sDate.WeekDay = RTC_WEEKDAY_MONDAY;
    sDate.Month   = RTC_MONTH_APRIL;
    sDate.Date    = 0x11;
    sDate.Year    = 0x22;
    
    HAL_RTC_SetDate( &hrtc, &sDate, RTC_FORMAT_BCD );
    

    tick_1000ms=HAL_GetTick();
   
    
}

/**
* @brief   **This function runs a machine state to change parameters and display the time every second**
*
*  The firs state of the machine is the IDLE state where it cheks if a msg is recive or if
*    a second has already pass, if a second has pass the state changes to DIPLAY_MSG where 
*    the time is display in the terminal with semi-hosting and then go back to IDLE state.
*    if a msg is recive Clockstate changes to GET_MSG where it compares the value to see what msg
*    arrives, if the msg is equal to CHANGE_TIME clockstate is change to CHANGE_TIME where it changes the
*    value of the time on the RTC and them changes clockstate to DISPLAY_MSG, The same is done on CHANGE_DATE and
*    CHANGE_ALARM but with date and alarm parameters
*   
* @retval  none 
*/
void Clock_Task( void )
{
    switch(Clockstate){

        case IDLE:
        {
            if( CAN_td_message.msg != NOT_MESSAGE)
            {
                Clockstate = GET_MSG;
            }
            if( (HAL_GetTick() - tick_1000ms) >= 1000)
            {
                tick_1000ms = HAL_GetTick();
                Clockstate  = DISPLAY_MSG;
            }
            break;
        }
        case GET_MSG:
        {

            if(CAN_td_message.msg==CHANGE_TIME)
            {
                Clockstate = CHANGE_TIME;
            }
            else if (CAN_td_message.msg==CHANGE_DATE)
            {
                Clockstate = CHANGE_DATE;
            }
            else if (CAN_td_message.msg==CHANGE_ALARM)
            {
                Clockstate = CHANGE_ALARM;
            }
            else
            {
             CAN_td_message.msg = NOT_MESSAGE;
            }
            break;

        }
            
        case DISPLAY_MSG:
        {
            /* Get the RTC current Time */
            HAL_RTC_GetTime( &hrtc, &sTime, RTC_FORMAT_BIN );
            /* Get the RTC current Date */
            HAL_RTC_GetDate( &hrtc, &sDate, RTC_FORMAT_BIN );

            printf("%d,%d,%d \n\r",sDate.Year,sDate.Date,sDate.Month);
            printf("%d,%d,%d \n\r",sTime.Hours,sTime.Minutes,sTime.Seconds);

            Clockstate=IDLE;
            break;
        }

        case CHANGE_TIME:
        {
            sTime.Hours          = CAN_td_message.tm.tm_hour;
            sTime.Minutes        = CAN_td_message.tm.tm_min;
            sTime.Seconds        = CAN_td_message.tm.tm_sec;
            sTime.SubSeconds     = 0x00;
            sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
            sTime.StoreOperation = RTC_STOREOPERATION_RESET;
            
            HAL_RTC_SetTime( &hrtc, &sTime, RTC_FORMAT_BCD );
            Clockstate=DISPLAY_MSG;
            break;
        }
        
        case CHANGE_DATE:
        {
            sDate.WeekDay   = CAN_td_message.tm.tm_wday;
            sDate.Month     = CAN_td_message.tm.tm_mon;
            sDate.Date      = CAN_td_message.tm.tm_mday;
            sDate.Year      = CAN_td_message.tm.tm_year_lsb;

            HAL_RTC_SetDate( &hrtc, &sDate, RTC_FORMAT_BCD );

            Clockstate          = DISPLAY_MSG;
            CAN_td_message.msg  = 0;
            break;
        }
        case CHANGE_ALARM:
        {
            CAN_td_message.msg = 0;
            break;
        }
        default:
        {
            CAN_td_message.msg = 0;
            break;
        }

    }

}