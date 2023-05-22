#include "app_clock.h"
#include <stdio.h>      /* cppcheck-suppress misra-c2012-21.6 ; the stdio.h is necesary */


RTC_HandleTypeDef hrtc = {0};
RTC_DateTypeDef sDate;
RTC_TimeTypeDef sTime;
int tick_1000ms;
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
    

    sTime.Hours   = 0x02;
    sTime.Minutes = 0x00;
    sTime.Seconds = 0x00;
    sTime.SubSeconds = 0x00;
    sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
    sTime.StoreOperation = RTC_STOREOPERATION_RESET;
    
    HAL_RTC_SetTime( &hrtc, &sTime, RTC_FORMAT_BCD );
    

    sDate.WeekDay = RTC_WEEKDAY_MONDAY;
    sDate.Month = RTC_MONTH_APRIL;
    sDate.Date = 0x0F;
    sDate.Year = 0xFF;
    
    HAL_RTC_SetDate( &hrtc, &sDate, RTC_FORMAT_BCD );
    

    tick_1000ms=HAL_GetTick();
   
    
}
void Clock_Task( void )
{ 

}