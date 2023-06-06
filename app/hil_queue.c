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

/**
* @brief   **This function reads a value on the circular buffer queue**
*
*  First we need to check if the circular buffer isn`t empty; 
*  This function reads a value on the circular buffer using the memcpy function 
*  this value is store in the data parameter ,the second parameter 
*  is the address were the data is going to be read and then we add the Tail value 
*  times the size of the array to get the correct position, and the last parameter is the size of the data to  be store.
*  after reading something we use the memcpy function but this time we put the second parameter on the first
*  and on the second parameter we put the address of x wich has a value of 0, the third parameter
*  remains the same, we made this to erased the value that was read.
*  after readinb something we add 1 to the Tail value so the next time we use the Read function 
*  the position will be the next one, we also use the % operator with the Elements beacuse this will 
*  reset the position of the Tail once it reaches the last one.
*  Then we give the Queue_Status to QUEUE_OK wich indicates if a value was read.
*  If the tail reaches the head this means that the circular buffer is empty.
*  And if the circular buffer was full we need to change this value to 0 since is no longer full.
*   
* @param   <*data>[in] Pointer of a value to be store
* @param   <*hqueue>[in] Pointer to a QUEUE_HandleTypeDef structure
* @retval  Queue_Status indicates if the circular buffer wrote something 
*/

uint8_t HIL_QUEUE_Read( QUEUE_HandleTypeDef *hqueue, void *data )
{
    uint8_t Queue_Status = QUEUE_NOT_OK;
    uint8_t x=0;
    if(hqueue->Empty != 1)
    {
      memcpy(data,((hqueue->Buffer) + ((hqueue->Tail)*(hqueue->size))),hqueue->size);
      memcpy(((hqueue->Buffer) + ((hqueue->Tail)*(hqueue->size))), &x,hqueue->size);
      
      hqueue->Tail = ((hqueue->Tail) + 1) % (hqueue->Elements);
      Queue_Status = QUEUE_OK;
    }

    if(hqueue->Tail==hqueue->Head)
    {
        hqueue->Empty = 1;
    }

    if(hqueue->Full == 1)
    {
        hqueue->Full = 0;
    }
    return Queue_Status;
}

/**
* @brief   **This function tell us if the queue is empty**
*
*  This function returns the value of the empty variable on the QUEUE_HandleTypeDef structure
*
* @param   <*hqueue>[in] Pointer to a QUEUE_HandleTypeDef structure
* @retval  hqueue->Empty 
*/
uint8_t HIL_QUEUE_IsEmpty( QUEUE_HandleTypeDef *hqueue )
{
    return hqueue->Empty;
}

/**
* @brief   **This function erase all the values of the circular buffer**
*
*  We use a for loop wich will run Elements value times,
*  and each loop we will asign the value of 0 to the circular buffer queue
*  after that we call the HIL_QUEUE_Init function to reset the queue.
*
* @param   <*hqueue>[in] Pointer to a QUEUE_HandleTypeDef structure
* 
* @retval  none
*/

void HIL_QUEUE_Flush( QUEUE_HandleTypeDef *hqueue )
{
   int x=0;
    for(int i = 0; i < hqueue->Elements ;i++ )
    {
      memcpy(((hqueue->Buffer) + (i*hqueue->size)),&x,hqueue->size);
    }
    HIL_QUEUE_Init(hqueue);
}

/**
* @brief   **This function disable an interruption and then writes a value on the circular buffer queue**
*
*  The function disable an interruption, the uses the HIL_QUEUE_Write function 
*  and after writing on the queue it enables the interruption.
*
* @param   hqueue[in]   Pointer to a QUEUE_HandleTypeDef structure
* @param   data[in]     Pointer of a value to be store
* @param   isr[in]      Value of an interruption to be disable
*
* @retval  Queue_Status indicates if the circular buffer wrote something 
* @note     to disable all interruption use value 0xFF
*/
uint8_t HIL_QUEUE_WriteISR( QUEUE_HandleTypeDef *hqueue, void *data, uint8_t isr )
{
    if( isr == ALL_INTS )
    { 
        __disable_irq();
    }
    else
    {
        HAL_NVIC_DisableIRQ(isr);
    }

    uint8_t Queue_Status = HIL_QUEUE_Write(hqueue, data);

    if( isr == ALL_INTS )
    { 
        __enable_irq();
    }
    else
    {
        HAL_NVIC_EnableIRQ(isr);
    }

    return Queue_Status;
}

/**
* @brief   **This function disable an interruption and then reads a value from the circular buffer queue**
*
*  The function disable an interruption, the uses the HIL_QUEUE_Read function 
*  and after writing on the queue it enables the interruption.
*
* @param   hqueue[in]   Pointer to a QUEUE_HandleTypeDef structure
* @param   data[in]     Pointer of a value to be store
* @param   isr[in]      Value of an interruption to be disable
* 
* @retval  Queue_Status indicates if the circular buffer wrote something 
* @note     to disable all interruption use value 0xFF
*/
uint8_t HIL_QUEUE_ReadISR( QUEUE_HandleTypeDef *hqueue, void *data, uint8_t isr )
{
    if( isr == ALL_INTS )
    { 
        __disable_irq();
    }
    else
    {
        HAL_NVIC_DisableIRQ(isr);
    }

    uint8_t Queue_Status = HIL_QUEUE_Read(hqueue, data);

    if( isr == ALL_INTS )
    { 
        __enable_irq();
    }
    else
    {
        HAL_NVIC_EnableIRQ(isr);
    }

    return Queue_Status;
}
