#include "app_bsp.h"
#include "app_serial.h"
#include "app_clock.h"
#include "app_display.h"
#include "scheduler.h"

//Add more includes if need them
/** 
  * @defgroup Tasks and timers periodicity value.
  @{ */
#define HEARTH_TICK_VALUE   300u    /*!<hearth toggle value*/   
#define SERIAL_TASK_TICK    10u     /*!<Serial task periodicity*/  
#define CLOCK_TASK_TICK     50u     /*!<Clock task periodicity*/     
#define DISPLAY_TASK_TICK   100u    /*!<Display task periodicity*/
#define ONE_SEC_TIMER       1000u   /*!<Software timer one second value*/
/**
  @} */

/** 
  * @defgroup Watchdog values.
  @{ */
#define WATCHDOG_REFRESH    70u     /*!<value to refresh the watchdog*/ 
#define WATCHDOG_WINDOW     94      /*!<window watchdog value*/ 
#define WATCHDOG_COUNTER    127     /*!<counter watchdog value*/
/**
  @} */

/** 
  * @defgroup PIN value configuration.
  @{ */  
#define ALL_PINS            0xFF    /*!<value to configure all pins of GPIOC port*/
/**
  @} */

/** 
  * @defgroup Scheduler values configuration.
  @{ */  
#define TASK_NUMBERS          5    /*!<Number of tasks to be handle by the scheduler*/
#define SCHEDULER_TICK        5    /*!<Tick value of the scheduler*/
#define TIMER_NUMBERS         1    /*!<Tick value of the scheduler*/
/**
  @} */

/**
* @brief  Variable for watchdog configuration.
*/
static WWDG_HandleTypeDef hwwdg;

/**
* @brief  Variable for functional safety.
*/
HAL_StatusTypeDef Status;

/**
* @brief  Variable for scheduler.
*/
Scheduler_HandleTypeDef sched;
uint32_t timer_1S; 
static void hearth_init(void);
static void hearth_beat(void);
static void init_watchdog(void);
static void peth_the_dog(void);

/**
* @brief   **Main function**
*
*   In the main function we`re going execute the task with the scheduler so we create
*   a Scheduler_HandleTypeDef and an array of Task_TypeDef with 5 lenght since thats
*   the amount of tasks we are going to execute, also we use a ticks of 5ms since our shortest
*   period is 10ms, then we add the tasks with the HIL_SCHEDULER_RegisterTask
*   and start the scheduler
*/
int main( void )
{
  Task_TypeDef hsche_tasks[TASK_NUMBERS];
  sched.tasks   = TASK_NUMBERS;
  sched.tick    = SCHEDULER_TICK;
  sched.taskPtr = hsche_tasks;
  HIL_SCHEDULER_Init(&sched);

  Timer_TypeDef hsche_timer[TIMER_NUMBERS];
  sched.timers   = TIMER_NUMBERS;
  sched.timerPtr = hsche_timer;
  timer_1S = HIL_SCHEDULER_RegisterTimer( &sched, ONE_SEC_TIMER, Display_msg );
  (void)HIL_SCHEDULER_StartTimer( &sched,timer_1S);

  HAL_Init();

  (void)HIL_SCHEDULER_RegisterTask( &sched,init_watchdog,peth_the_dog,WATCHDOG_REFRESH);
  (void)HIL_SCHEDULER_RegisterTask( &sched,Serial_Init,Serial_Task,SERIAL_TASK_TICK);
  (void)HIL_SCHEDULER_RegisterTask( &sched,Clock_Init,Clock_Task,CLOCK_TASK_TICK);
  (void)HIL_SCHEDULER_RegisterTask( &sched,Display_Init,Display_Task,DISPLAY_TASK_TICK);
  (void)HIL_SCHEDULER_RegisterTask( &sched,hearth_init,hearth_beat,HEARTH_TICK_VALUE);

  HIL_SCHEDULER_Start(&sched);
}

/**
* @brief   **Init function for hearthbeat**
*
*   This function provides the initialization for the GPIOC
*   where the led that is going to be blinking is connected.
*   also initialized the tick_hearth for the concurrent process
*/
void hearth_init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct;

  HAL_Init();                                     
  __HAL_RCC_GPIOC_CLK_ENABLE();                   
  
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1 ;           
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;     
  GPIO_InitStruct.Pull = GPIO_NOPULL;             
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;   
    
  HAL_GPIO_Init( GPIOC, &GPIO_InitStruct );
}

/**
* @brief   **hearth_beat function**
*
*   This function toggle the LED on GPIO_PIN_0 every HEARTH_TICK_VALUE wich is 300ms
*/
void hearth_beat(void)
{    
  HAL_GPIO_TogglePin( GPIOC, GPIO_PIN_0 );      
}

/**
* @brief   **Init function for Watchdog**
*
*   This function provides the initialization for the watchdog,
*   The watchdog timeout calculation is:
*   tWWDG= tPCLK * 4096 * 2 ^ WDGTB[1:0] * (T 5:0[ ]+ 1) 
*   using a prescaler of 16 and a counter value of 126 the timeout is:
*   tWWDG= (1/32000) * 4096 * 2 ^ 4 * (127 + 1) = 262.614ms
*   now we do the calculation with the window value of 94
*   tWWDG= (1/32000) * 4096 * 2 ^ 4 * (94 + 1) = 194.56ms
*   now we substract the timeout value of the counter with the timeout value of the window
*   refresh_min_value = 194.56ms-194.56ms = 67.584ms.
*   We round up the value to 70 since the scheduler work on ticks of 5 ms.
*   thats the minimum value to refresh the watchdog.
*   @note   this function also enables flash interrupts
*/
void init_watchdog(void)
{
  __HAL_RCC_WWDG_CLK_ENABLE();

  hwwdg.Instance          = WWDG;
  hwwdg.Init.Prescaler    = WWDG_PRESCALER_16;
  hwwdg.Init.Window       = WATCHDOG_WINDOW;
  hwwdg.Init.Counter      = WATCHDOG_COUNTER;
  hwwdg.Init.EWIMode      = WWDG_EWI_ENABLE;
    
  Status = HAL_WWDG_Init(&hwwdg);
  assert_error( Status == HAL_OK, WWDG_INIT_ERROR );  /* cppcheck-suppress misra-c2012-11.8 ; function cannot be modify */
  __HAL_WWDG_ENABLE_IT(&hwwdg, WWDG_IT_EWI);

  HAL_NVIC_EnableIRQ(FLASH_IRQn);
}

/**
* @brief   **Refresh function for Watchdog**
*
*   This function refresh the watchdog since if the watchdog is not refresh
*   before it`s timeout it will trigger a reset on the microcontroller,
*   because if it`s not refresh it would mean that the program is not working properly
*   in this case the reset is every 34ms that value is previusly calculated on the init_watchdog
*   function.
*/
void peth_the_dog(void)
{   
  Status = HAL_WWDG_Refresh(&hwwdg); 
  assert_error( Status == HAL_OK, WWDG_REFRESH_ERROR );     /* cppcheck-suppress misra-c2012-11.8 ; function cannot be modify */
}

/**
* @brief   **safe state function**
*
*   This function is a safe state for the application, when an error occurs
*   it will call this function where first is going to disable all the interruptions
*   and clocks, it will enable all port C were leds are conected to show the error
*   that called the function.
*
* @param   *file[in] Pointer to the file that trigger the function
* @param   line[in]   value of the line that trigger the function
* @param   error[in] value of the error that trigger the function
* @retval  None
*/
/* cppcheck-suppress misra-c2012-2.7 ;  function cannot be modify */
void safe_state( uint8_t *file, uint32_t line, uint8_t error )
{
  GPIO_InitTypeDef GPIO_InitStruct;
  /*disable all maskable interrupts*/
  HAL_Init();
  __disable_irq();
    
  __HAL_RCC_FDCAN_CLK_DISABLE();
  __HAL_RCC_GPIOD_CLK_DISABLE();
  __GPIOB_CLK_DISABLE();

  __HAL_RCC_SYSCFG_CLK_DISABLE();
  __HAL_RCC_PWR_CLK_DISABLE();

  __HAL_RCC_RTC_DISABLE();
  __HAL_RCC_RTCAPB_CLK_DISABLE();

  __SPI1_CLK_DISABLE();


  __HAL_RCC_GPIOC_CLK_ENABLE(); 

  HAL_SuspendTick();



  GPIO_InitStruct.Pin = ALL_PINS; 
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;     
  GPIO_InitStruct.Pull = GPIO_NOPULL;             
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;    
    
  HAL_GPIO_Init( GPIOC, &GPIO_InitStruct );
    /*set all outputs to a safe state, you must think what will be the so called safe state 
    for the pins and peripherals*/
    
    /*disable all timers included the WWDG*/
    
    /*output the error code using the leds connected to port C*/
    HAL_GPIO_WritePin( GPIOC, ALL_PINS , RESET );
    HAL_GPIO_WritePin( GPIOC, error , SET );        
    
    for( ;; )
    {
        /*Waiting for the user to press the reset button, 
        you can also set a break point here and using 
        the debugger you can visualize the three parameters file, line and error*/
    }
}
