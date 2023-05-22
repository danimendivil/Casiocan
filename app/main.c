#include "app_bsp.h"
#include "app_serial.h"
//Add more includes as needed

int main( void )
{
    HAL_Init();
    Serial_Init();
    //Add more initilizations as needed
    
    for( ;; )
    {
        Serial_Task();
        //Add another task as needed
    }
}