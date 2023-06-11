/**
* @file    <hil_queue.h>
* @brief   **Header file for circular buffer**
*
* This file contains global variables, structures or defines 
* necesary for the circular buffer
*         
*/
#ifndef HIL_QUEUE_H__
#define HIL_QUEUE_H__
    
    #include "app_bsp.h"
    
    /** 
     * @defgroup QUEUE queue structure values 
    @{ */
    #define FIRS_POS    0u      /*!<first position of the queue*/
    #define EMPTY       1u      /*!<queue is empty*/
    #define NOT_EMPTY   0u      /*!<queue is not empty*/
    #define NOT_FULL    0u      /*!<queue is not full*/
    #define IS_FULL     1u      /*!<queue is full*/
    /**
    @} */
    
    /** 
     * @defgroup Que state this values represent if the circular buffer is functioning correct
     { */
    #define QUEUE_OK  1u        /*!<queue function has been done*/
    #define QUEUE_NOT_OK 0u     /*!<queue function has not been done*/
    /**
    @} */

    /** 
     * @defgroup Queue all interruptions this values is used for disabling all interruption 
     @{ */
     /* cppcheck-suppress misra-c2012-2.5 ; define will later be used */
    #define QUEUE_ALL_INTS    0XFFu       /*!<value to disable all interruptions*/  
    /**
    @} */

    /** 
     * @brief  QUEUE_HandleTypeDef Elements of the circular buffer structure
     @{ */
    typedef struct
    {
        void        *Buffer;  /*!<Pointer to the memory space used as a buffer by the queue*/
        uint32_t    Elements; /*!<Number of elements to store (size of the queue)*/
        uint8_t     size;     /*!<Size of the elements to store*/
        uint32_t    Head;     /*!<Pointer indicating the next space to write*/
        uint32_t    Tail;     /*!<Pointer indicating the next space to read*/
        uint8_t     Empty;    /*!<Flag indicating if there are no elements to read*/
        uint8_t     Full;     /*!<Flag indicating if no more elements can be written*/
        
    } QUEUE_HandleTypeDef;

    void HIL_QUEUE_Init( QUEUE_HandleTypeDef *hqueue );
    uint8_t HIL_QUEUE_Write( QUEUE_HandleTypeDef *hqueue, void *data );
    uint8_t HIL_QUEUE_Read( QUEUE_HandleTypeDef *hqueue, void *data );
    uint8_t HIL_QUEUE_IsEmpty( QUEUE_HandleTypeDef *hqueue );
    void HIL_QUEUE_Flush( QUEUE_HandleTypeDef *hqueue );

    uint8_t HIL_QUEUE_WriteISR( QUEUE_HandleTypeDef *hqueue, void *data, uint8_t isr );
    uint8_t HIL_QUEUE_ReadISR( QUEUE_HandleTypeDef *hqueue, void *data, uint8_t isr );
    uint8_t HIL_QUEUE_IsEmptyISR( QUEUE_HandleTypeDef *hqueue, uint8_t isr );
    void HIL_QUEUE_FlushISR( QUEUE_HandleTypeDef *hqueue, uint8_t isr );

    /**
    * @brief  Circular buffer variable for CAN msg recived to serial task.
    */
    extern QUEUE_HandleTypeDef SERIAL_queue;

#endif