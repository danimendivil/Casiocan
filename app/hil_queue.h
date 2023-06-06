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
     * @defgroup Que state this values represent if the circular buffer is functioning correct
     { */
    #define QUEUE_OK  1u        /*!<queue function has been done*/
    #define QUEUE_NOT_OK 0u     /*!<queue function has not been done*/
    /**
    @} */

    /** 
     * @defgroup Queue all interruptions this values is used for disabling all interruption 
     @{ */
    #define ALL_INTS    0XFFu       /*!<value to disable all interruptions*/
    /**
    @} */

    /** 
     * @brief  QUEUE_HandleTypeDef Elements of the circular buffer structure
     @{ */
    typedef struct
    {
        void        *Buffer;  /*!<puntero al espacio de memoria usado como buffer por la cola*/
        uint32_t    Elements; /*!<numero de elementos a almacenar (tama;o de la cola)*/
        uint8_t     size;     /*!<tamaño del tipo de elementos a almacenar*/
        uint32_t    Head;     /*!<puntero que indica el siguiente espacio a escribir*/
        uint32_t    Tail;     /*!<puntero que indica el siguiente espacio a leer*/
        uint8_t     Empty;    /*!<bandera que indica si no hay elementos a leer*/
        uint8_t     Full;     /*!<bandera que indica si no se puede seguir escribiendo mas elementos*/
        
    } QUEUE_HandleTypeDef;
    /**
    @} */

    void HIL_QUEUE_Init( QUEUE_HandleTypeDef *hqueue );
    uint8_t HIL_QUEUE_Write( QUEUE_HandleTypeDef *hqueue, void *data );
    uint8_t HIL_QUEUE_Read( QUEUE_HandleTypeDef *hqueue, void *data );
    uint8_t HIL_QUEUE_IsEmpty( QUEUE_HandleTypeDef *hqueue );
    void HIL_QUEUE_Flush( QUEUE_HandleTypeDef *hqueue );

    uint8_t HIL_QUEUE_WriteISR( QUEUE_HandleTypeDef *hqueue, void *data, uint8_t isr );
    uint8_t HIL_QUEUE_ReadISR( QUEUE_HandleTypeDef *hqueue, void *data, uint8_t isr );
    uint8_t HIL_QUEUE_IsEmptyISR( QUEUE_HandleTypeDef *hqueue, uint8_t isr );
    void HIL_QUEUE_FlushISR( QUEUE_HandleTypeDef *hqueue, uint8_t isr );

#endif