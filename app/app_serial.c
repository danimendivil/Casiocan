#include "app_serial.h"
FDCAN_HandleTypeDef CANHandler;
FDCAN_TxHeaderTypeDef CANTxHeader;
FDCAN_RxHeaderTypeDef CANRxHeader;
FDCAN_FilterTypeDef CANFilter;
uint8_t datar[8]; 

void Serial_Init( void )
{
    CANHandler.Instance                 = FDCAN1;
    CANHandler.Init.Mode                = FDCAN_MODE_NORMAL;
    CANHandler.Init.FrameFormat         = FDCAN_FRAME_CLASSIC;
    CANHandler.Init.ClockDivider        = FDCAN_CLOCK_DIV1;
    CANHandler.Init.TxFifoQueueMode     = FDCAN_TX_FIFO_OPERATION;
    CANHandler.Init.AutoRetransmission  = DISABLE;
    CANHandler.Init.TransmitPause       = DISABLE;
    CANHandler.Init.ProtocolException   = DISABLE;
    CANHandler.Init.ExtFiltersNbr       = 0;
    CANHandler.Init.StdFiltersNbr       = 1;  /*indicamos que usaremos un filtro*/
    CANHandler.Init.NominalPrescaler    = 10;
    CANHandler.Init.NominalSyncJumpWidth = 1;
    CANHandler.Init.NominalTimeSeg1     = 11;
    CANHandler.Init.NominalTimeSeg2     = 4;
    /*setear configuracion del modulo CAN*/
    Status = HAL_FDCAN_Init( &CANHandler );
    assert_error( Status == HAL_OK, FDCAN_Init_ERROR );

    /* Configure reception filter to Rx FIFO 0, este filtro solo aceptara mensajes con el ID 0x111 */
    CANFilter.IdType = FDCAN_STANDARD_ID;
    CANFilter.FilterIndex = 0;
    CANFilter.FilterType = FDCAN_FILTER_MASK;
    CANFilter.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;
    CANFilter.FilterID1 = 0x111;
    
    Status = HAL_FDCAN_ConfigFilter( &CANHandler, &CANFilter );
    assert_error( Status == HAL_OK, FDCAN_Config_ERROR );
    /*indicamos que los mensajes que no vengan con el filtro indicado sean rechazados*/
    Status = HAL_FDCAN_ConfigGlobalFilter(&CANHandler, FDCAN_REJECT, FDCAN_REJECT, FDCAN_FILTER_REMOTE, FDCAN_FILTER_REMOTE);
    assert_error( Status == HAL_OK, FDCAN_Config_Global_ERROR );
    /* Change FDCAN instance from initialization mode to normal mode */
    Status = HAL_FDCAN_Start( &CANHandler);
    assert_error( Status == HAL_OK, FDCAN_Start_ERROR );
    /*activamos la interrupcion por recepcion en el fifo0 cuando llega algun mensaje*/
    HAL_FDCAN_ActivateNotification( &CANHandler, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0 );

     /* Declaramos las opciones para configurar los parametros de transmision CAN */
    CANTxHeader.IdType      = FDCAN_STANDARD_ID;
    CANTxHeader.FDFormat    = FDCAN_CLASSIC_CAN;
    CANTxHeader.TxFrameType = FDCAN_DATA_FRAME;
    CANTxHeader.Identifier  = 0x122;
    CANTxHeader.DataLength  = FDCAN_DLC_BYTES_8;


}


static void CanTp_SingleFrameTx( uint8_t *data, uint8_t *size ) 
{

    *data=*size;
    Status = HAL_FDCAN_AddMessageToTxFifoQ( &CANHandler, &CANTxHeader, data );
    assert_error( Status == HAL_OK, FDCAN_Add_Message_ERROR );

}

static uint8_t CanTp_SingleFrameRx( uint8_t *data, uint8_t *size )
{
    uint8_t x;
    
    if(flag == 1u){
        flag = 0u;
       
        
        HAL_FDCAN_GetRxMessage( &CANHandler, FDCAN_RX_FIFO0, &CANRxHeader, datar );

        *size=*data;

        if(*size > 0u){

            x = 1;

        }
        else{

            x = 0;

        }
    }
    else{
        x = 0;
    }

    return x;

}

void HAL_FDCAN_RxFifo0Callback( FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs ) 
{
    /*A llegado un mensaje via CAN, interrogamos si fue un solo mensaje*/
    if( ( RxFifo0ITs & FDCAN_IT_RX_FIFO0_NEW_MESSAGE ) != 0 )
    {
        flag = 1;  
    }
}