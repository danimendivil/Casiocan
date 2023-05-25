#include "app_bsp.h"
#include "app_serial.h"
#include "app_clock.h"
//Add more includes if need them
#define hearth_tick_value   300
#define watchdog_refresh    34

extern void initialise_monitor_handles(void);

static void hearth_init(void);
static void hearth_beat(void);
static void init_watchdog(void);
static void peth_the_dog(void);

/**
* @brief  Variable for concurrent process of watchdog refresh.
*/
static int tick_Dog;

/**
* @brief  Variable for concurrent process of hearth beat.
*/
static int tick_hearth;

/**
* @brief  Variable for watchdog configuration.
*/
static WWDG_HandleTypeDef hwwdg;

int main( void )
{
    HAL_Init();
    Serial_Init();
    Clock_Init();
    hearth_init();
    init_watchdog();
    //Add more initilizations if need them
    
    for( ;; )
    {
        Serial_Task();
        Clock_Task();
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
*   This function toggle the LED on GPIO_PIN_0 every hearth_tick_value wich is 300
*/
void hearth_beat(void)
{
    if( (HAL_GetTick() - tick_hearth) >= hearth_tick_value )
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
*   tWWDG= (1/64000) * 4096 * 2 ^ 4 * (127 + 1) = 131.07ms
*   now we do the calculation with the window value of 94
*   tWWDG= (1/64000) * 4096 * 2 ^ 4 * (94 + 1) = 97.28ms
*   now we substract the timeout value of the counter with the timeout value of the window
*   refresh_min_value = 131.07ms-97.28ms = 33.792ms
*   thats the minimum value to refresh the watchdog.
*/
void init_watchdog(void)
{
    __HAL_RCC_WWDG_CLK_ENABLE( );

    hwwdg.Instance          = WWDG;
    hwwdg.Init.Prescaler    = WWDG_PRESCALER_16;
    hwwdg.Init.Window       = 94;
    hwwdg.Init.Counter      = 127;
    hwwdg.Init.EWIMode      = WWDG_EWI_DISABLE;
    
    HAL_WWDG_Init(&hwwdg);
    
    tick_Dog = HAL_GetTick();
}

/**
* @brief   **Refresh function for Watchdog**
*
*   This function refresh the watchdog every 34ms since this is
*   the value calculated in the init function
*/
void peth_the_dog(void)
{

    if( (HAL_GetTick() - tick_Dog) >= watchdog_refresh)
    {
        tick_Dog = HAL_GetTick(); 
        HAL_WWDG_Refresh(&hwwdg);     
    }
}
