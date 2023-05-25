#include "app_bsp.h"
#include "app_serial.h"
#include "app_clock.h"
//Add more includes if need them
extern void initialise_monitor_handles(void);
int main( void )
{
    HAL_Init();
    Serial_Init();
    Clock_Init();
    //Add more initilizations if need them
    
    for( ;; )
    {
        Serial_Task();
        Clock_Task();
        //Add another task if need it
    }
}