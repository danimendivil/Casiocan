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

int main( void )
{
  
  HAL_Init();
  Display_Init();  
  Clock_Init();
  Display_Init();
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
*/
void init_watchdog(void)
{
    __HAL_RCC_WWDG_CLK_ENABLE( );

    hwwdg.Instance          = WWDG;
    hwwdg.Init.Prescaler    = WWDG_PRESCALER_16;
    hwwdg.Init.Window       = WATCHDOG_WINDOW;
    hwwdg.Init.Counter      = WATCHDOG_COUNTER;
    hwwdg.Init.EWIMode      = WWDG_EWI_DISABLE;
    
    HAL_WWDG_Init(&hwwdg);
    
    tick_Dog = HAL_GetTick();
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
        HAL_WWDG_Refresh(&hwwdg);     
    }
}
