#include "hil_queue.h"

/**
* @brief   **This function initializes the parameters for the circular buffer**
*
*  This function put values of Head,Tail to 0 wich indicates their positions
*  Empty to 1 wich indicates that the circular buffer is empty and has no values in it
*  And full to 0 wich indicates that the circular buffer is not full
* @param   <*hqueue>[in] Pointer to a QUEUE_HandleTypeDef structure
* 
* @retval  none 
*/
void HIL_QUEUE_Init( QUEUE_HandleTypeDef *hqueue )
{
    hqueue->Head    = 0;
    hqueue->Tail    = 0;
    hqueue->Empty   = 1;
    hqueue->Full    = 0;
}