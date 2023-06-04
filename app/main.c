#include "app_bsp.h"
#include "app_serial.h"
#include "app_clock.h"
#include "app_display.h"

//Add more includes if need them
/** 
  * @defgroup Hearth tick value.
  @{ */
#define HEARTH_TICK_VALUE   300u    /*!<hearth toggle value*/        
/**
  @} */

/** 
  * @defgroup Watchdog values.
  @{ */
#define WATCHDOG_REFRESH    68u     /*!<value to refresh the watchdog*/ 
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

static void hearth_init(void);
static void hearth_beat(void);
static void init_watchdog(void);
static void peth_the_dog(void);

/**
* @brief  Variable for concurrent process of watchdog refresh.
*/
static uint32_t tick_Dog;

/**
* @brief  Variable for concurrent process of hearth beat.
*/
static uint32_t tick_hearth;

/**
* @brief  Variable for watchdog configuration.
*/
static WWDG_HandleTypeDef hwwdg;

/**
* @brief  Variable for functional safety.
*/
HAL_StatusTypeDef Status;

int main( void )
{
 
  HAL_Init();
  Serial_Init();
  Display_Init();  
  Clock_Init();
  hearth_init();
  init_watchdog();
  //Add more initilizations if need them
 
  for( ;; )
  {
    Serial_Task();
    Clock_Task();
    Display_Task();
    hearth_beat();
    peth_the_dog();
    //Add another task if need it
  }
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
  
    GPIO_InitStruct.Pin = GPIO_PIN_0;           
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;     
    GPIO_InitStruct.Pull = GPIO_NOPULL;             
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;   
    
    HAL_GPIO_Init( GPIOC, &GPIO_InitStruct );
    
    tick_hearth = HAL_GetTick();
}

/**
* @brief   **hearth_beat function**
*
*   This function toggle the LED on GPIO_PIN_0 every HEARTH_TICK_VALUE wich is 300ms
*/
void hearth_beat(void)
{
    if( (HAL_GetTick() - tick_hearth) >= HEARTH_TICK_VALUE )
    {    
        HAL_GPIO_TogglePin( GPIOC, GPIO_PIN_0 );   
        tick_hearth = HAL_GetTick(); 
    }  
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
*   refresh_min_value = 194.56ms-194.56ms = 67.584ms
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
    hwwdg.Init.EWIMode      = WWDG_EWI_DISABLE;
    
    Status = HAL_WWDG_Init(&hwwdg);
    assert_error( Status == HAL_OK, WWDG_INIT_ERROR );
    __HAL_WWDG_ENABLE_IT(&hwwdg, WWDG_IT_EWI);
    tick_Dog = HAL_GetTick();

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

    if( (HAL_GetTick() - tick_Dog) >= WATCHDOG_REFRESH)
    {
        tick_Dog = HAL_GetTick(); 
        Status = HAL_WWDG_Refresh(&hwwdg); 
        assert_error( Status == HAL_OK, WWDG_REFRESH_ERROR );    
    }
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
    
    while( 1u )
    {
        /*Waiting for the user to press the reset button, 
        you can also set a break point here and using 
        the debugger you can visualize the three parameters file, line and error*/
    }
}