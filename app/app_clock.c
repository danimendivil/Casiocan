


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
   
    
}
void Clock_Task( void )
{ 
    
}