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

/**
* @brief   **This function writes a value on the circular buffer queue**
*
*  First we need to check if the circular buffer isn`t full; 
*  This function writes a value on the circular buffer using the memcpy function 
*  this value is store in the Buffer array previously intitializate,the first parameter 
*  is the address were the data is going to be stored and then we add the Head value 
*  times the size of the array to get the correct position, the next parameter is the 
*  address of the data to be write, and the last parameter is the size of the data to  be store.
*  after writing something we add 1 to the Head value so the next time we use the write function 
*  the position will be the next one, we also use the % operator with the Elements beacuse this will 
*  reset the position of the head once it reaches the last one.
*  if after adding one to the head has the same value as the Tail this indicates that the queue is full
*  and if the que was empty before writing a value we need to change it because it is no longer empty
*
* @param   <*hqueue>[in] Pointer to a QUEUE_HandleTypeDef structure
* @param   <*data>[in] Pointer of a value to be store
* 
* @retval  Queue_Status indicates if the circular buffer wrote something 
*/

uint8_t HIL_QUEUE_Write( QUEUE_HandleTypeDef *hqueue, void *data )
{
    uint8_t Queue_Status = QUEUE_NOT_OK;

    if(hqueue->Full==0)
    {
        memcpy(((hqueue->Buffer) +  ((hqueue->Head)*(hqueue->size)) ), data,hqueue->size);
        hqueue->Head = ( ( ( hqueue->Head ) + 1 ) ) % (hqueue->Elements);
        Queue_Status = QUEUE_OK;
    }

    if((hqueue->Head) == (hqueue->Tail))
    {
        hqueue->Full=1;  
    }

    if(hqueue->Empty == 1)
    {
        hqueue->Empty= 0;
    }

    return Queue_Status;
}