#include "app_bsp.h"
#include "app_serial.h"
#include "app_clock.h"
//Add more includes if need them
#define hearth_tick_value   300
extern void initialise_monitor_handles(void);

static void hearth_init(void);
static void hearth_beat(void);

static int tick_hearth;

int main( void )
{
    HAL_Init();
    Serial_Init();
    Clock_Init();
    hearth_init();
    //Add more initilizations if need them
    
    for( ;; )
    {
        Serial_Task();
        Clock_Task();
        hearth_beat();
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
