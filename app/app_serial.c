#include "app_serial.h"

FDCAN_HandleTypeDef CANHandler; /* cppcheck-suppress misra-c2012-8.5 ; other declaration is used on ints */
FDCAN_TxHeaderTypeDef CANTxHeader;
FDCAN_FilterTypeDef CANFilter;

uint8_t CAN_msg[CAN_DATA_LENGHT]; 
uint8_t CAN_size;

uint8_t cases;
uint8_t flag; 
APP_MsgTypeDef td_message;  //time and date message

static uint8_t valid_date(uint8_t day, uint8_t month, uint8_t yearM, uint8_t yearL);
static uint8_t dayofweek(uint8_t yearM, uint8_t yearL, uint8_t month, uint8_t day);

/**
 * @brief   **Provide a brief fucntion description (just one line)**
 *
 *   This function provides the initialization for the CAN comunication on CAN Clasic,
 *   no prescaling is applied to the clock,the transmit queue will operate automatically,
 *   the message will only be transmitted once, there will be no delay between transmissions,
 *   one filter will be used.
 *   The time quanta calculation is:
 *   Ntq = fCAN / CANbaudrate
 *   Ntq = 1.6Mhz / 100Kbps = 16 .
 *   The sample point is:
 *   Sp = ( CANHandler.Init.NominalTimeSeg1 +  1 / Ntq ) * 100
 *   Sp = ( ( 11 + 1 ) / 16 ) * 100 = 75%
 *   The filter is configurate so that it only accept messages with ID 0x111
 *   The transmition is configurate with ID 0x122
 *
 * 
 *          
 *
 * 
 */
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
    HAL_FDCAN_Init( &CANHandler );
    

    /* Configure reception filter to Rx FIFO 0, este filtro solo aceptara mensajes con el ID 0x111 */
    CANFilter.IdType = FDCAN_STANDARD_ID;
    CANFilter.FilterIndex = 0;
    CANFilter.FilterType = FDCAN_FILTER_MASK;
    CANFilter.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;
    CANFilter.FilterID1 = 0x111;
    
    HAL_FDCAN_ConfigFilter( &CANHandler, &CANFilter );
    
    /*indicamos que los mensajes que no vengan con el filtro indicado sean rechazados*/
    HAL_FDCAN_ConfigGlobalFilter(&CANHandler, FDCAN_REJECT, FDCAN_REJECT, FDCAN_FILTER_REMOTE, FDCAN_FILTER_REMOTE);
    
    /* Change FDCAN instance from initialization mode to normal mode */
    HAL_FDCAN_Start( &CANHandler);
    
    /*activamos la interrupcion por recepcion en el fifo0 cuando llega algun mensaje*/
    HAL_FDCAN_ActivateNotification( &CANHandler, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0 );

     /* Declaramos las opciones para configurar los parametros de transmision CAN */
    CANTxHeader.IdType      = FDCAN_STANDARD_ID;
    CANTxHeader.FDFormat    = FDCAN_CLASSIC_CAN;
    CANTxHeader.TxFrameType = FDCAN_DATA_FRAME;
    CANTxHeader.Identifier  = 0x122;
    CANTxHeader.DataLength  = FDCAN_DLC_BYTES_8;


}

/**
 * @brief   **Transmit a message to the CAN**
 *
 * This function Transmit the message pointed by the variable data, beore sending the message
 * we make the first value of the data pointer equal to the size pointer since this indicates
 * the size of the message that is going to be transmited. 
 * 
 *
 * @param   <*data>[in] Pointer of the array that is going to be transmited
 * @param   <*size>[in] Size of the message that is going to be transmited
 *
 * @retval  None
 *          
 *
 * 
 */
static void CanTp_SingleFrameTx( uint8_t *data, uint8_t *size ) 
{

    *data=*size;
    HAL_FDCAN_AddMessageToTxFifoQ( &CANHandler, &CANTxHeader, data );
    

}


/**
 * @brief   **Gets a message of the CAN communication**
 *
 *  The function first check if a message has arrive with the flag variable that is 1 when there is a message.
 *  if a message is recived the flag value is cleared.
 *  then gets the message and stores it in *data then checks if size value is greater than 1 and if a message 
 *  is valid it returns 1 if it`s not it returns 0
 * 
 *
 * @param   <*data>[in] Pointer of the array where de data is going to be store
 * @param   <*size>[in] Size of the message that`s been recived
 *
 * @retval  Return is 0 if data is unvalid and 1 if it is valid
 *
 * 
 */
static uint8_t CanTp_SingleFrameRx( uint8_t *data, uint8_t *size )
{
    uint8_t x;
    FDCAN_RxHeaderTypeDef CANRxHeader; 
    
    if(flag == TRUE){
        flag = FALSE;
       
        
        HAL_FDCAN_GetRxMessage( &CANHandler, FDCAN_RX_FIFO0, &CANRxHeader, CAN_msg );

        *size=*data;

        if(*size > 0u){

            x = TRUE;

        }
        else{

            x = FALSE;

        }
    }
    else{
        x = FALSE;
    }

    return x;

}


/**
 * @brief   **This is an interruption function for the CAN  **
 *
 * this function is an interruption that is called when a message is recived throught the CAN,
 * when a message is recived the flag variable is turn to 1.
 * 
 *
 * @param   <*hfdcan>[in] structure of CAN.
 * @param   <*RxFifo0ITs>[in] .
 * @param   <flag>[out] flag for new message.
 * @retval  None
 *
 * 
 */
/* cppcheck-suppress misra-c2012-2.7 ; this is a library function */
void HAL_FDCAN_RxFifo0Callback( FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs ) /* cppcheck-suppress misra-c2012-8.4 ; this is a library function */
{
    /*A llegado un mensaje via CAN, interrogamos si fue un solo mensaje*/
    if( ( RxFifo0ITs & FDCAN_IT_RX_FIFO0_NEW_MESSAGE ) != 0 )
    {
        flag = TRUE;  
    }
}


/**
* @brief   **Function that checks if the date is valid**
*
* The fucntion checks the parameters so that they are a valid date
* 
*
* @param   <day>[in]       day of the month
* @param   <month>[in]     month of the year
* @param   <year>[in]      year of the date
* @retval  if the day is valid it will return 1 otherwise a 0.
*          
*
* @note This is optional, just in case something very special needs to be take into account
*/
uint8_t valid_date(uint8_t day, uint8_t month, uint8_t yearM, uint8_t yearL)
{
    uint32_t year = ((uint32_t)(yearM) * 100u) + (uint32_t)yearL;
    uint32_t flagd = TRUE;

    if ((day > 0u ) && (day <= 31u) && (month <= DEC) && (month >= JAN) && (year >= 1900u) && (year <= 2100u)) {

        flagd = TRUE;

        if ((month == JAN) || (month == MAR) || (month == MAY) || (month == JUL) || (month == AUG) || (month == OCT) || (month == DEC)) {

            if (day > 31u) {
                flagd = FALSE;
            }

        } else if ((month == APR) || (month == JUN) || (month == SEP) || (month == NOV)) {

            if (day > 30u) {
                flagd = FALSE;
            }

        } else if (month == FEB) {

            if (((year % 4u == 0u) && (year % 100u != 0u)) || ((year % 400u) == 0u)) {

                if (day > 29u) {
                    flagd = FALSE;
                }

            } else {

                if (day > 28u) {
                    flagd = FALSE;
                }

            }

        }

    } else {
        flagd = FALSE;
    }

    return flagd;
}


/**
* @brief   **Gets the day of the week**
*
* this function take the date and uses the zeller's congruence formula to get the day of the week.
* 
*
* @param   <day>[in]       day of the month
* @param   <month>[in]     month of the year
* @param   <year>[in]      year of the date
*
* @retval  returns a value of the day of the week defines. 
*          
*
* 
*/
uint8_t dayofweek(uint8_t yearM, uint8_t yearL, uint8_t month, uint8_t day){

    uint32_t year = ((uint32_t)(yearM) * 100u) + (uint32_t)yearL;

   if (month < 3) {
        month += 12;
        year--;
    }
    int c = year / 100;
    int y = year - 100 * c;
    int m = month;
    int d = day;
    int w = ((13 * m + 3) / 5 + d + y + y / 4 + c / 4 - 2 * c);
    w %= 7;
    if (w < 0) {
        w += 7;
    }
    return w;

}

/**
* @brief   **This function validates and stores messages recived through CAN**
*
* The first state of the state machine is the GETMSG were we use the funtion Can_Tp_SingleFrameRx to see 
* if a message has been recived, if a message has been recived it compares the values to APP_Messages defines
* to see what is going to be the next state, if the next state is SERIAL_MSG_TIME it validates the values and 
* if the values are correct they are store on the td_message variable, and the state is change to the OK state where it sends
* a confirmation message if the values are wrong then the next state is FAILED where it sends an error message and the 
* state is changed to GETMSG.
* if the next state is SERIAL_MSG_DATE it validates the values with the valid_date() function and if the date is valid
* it also calls the dayofweek function to get the day of the week if they are valid then they are store on the td_message variable 
* an the state will be change to OK otherwise state will be FAILED
* if the next state is SERIAL_MSG_ALARM it validates the data and if they are correct are store on the td_message variable and 
* state is changed to ok, otherwise state will be FAILED
* 
*
*          
*/
void Serial_Task( void )
{
    switch(cases){

        case GETMSG:

            if (CanTp_SingleFrameRx(  &CAN_msg[0], &CAN_size ) == TRUE){

                if(CAN_msg[1]==SERIAL_MSG_TIME){

                    cases=SERIAL_MSG_TIME;

                }
                else if(CAN_msg[1]==SERIAL_MSG_DATE){

                    cases=SERIAL_MSG_DATE;

                }
                else if(CAN_msg[1]==SERIAL_MSG_ALARM){

                    cases=SERIAL_MSG_ALARM;
                }
                else{}

                }
                else{}
            
            break;

       
        case SERIAL_MSG_TIME:

            if( (CAN_msg[2] < 24u) && (CAN_msg[3] < 60u) && (CAN_msg[4] <60u)){

                td_message.tm.tm_hour=CAN_msg[2];
                td_message.tm.tm_min=CAN_msg[3];
                td_message.tm.tm_sec=CAN_msg[4];
                td_message.msg=SERIAL_MSG_TIME;

                cases=OK;

                }
            else{
                cases=FAILED;
                }
            
             break;

        case SERIAL_MSG_DATE:

            if(valid_date(CAN_msg[2],CAN_msg[3], CAN_msg[4],CAN_msg[5]) == 1u){

                td_message.tm.tm_mday = CAN_msg[2];
                td_message.tm.tm_mon = CAN_msg[3];
                td_message.tm.tm_year_msb = CAN_msg[4];
                td_message.tm.tm_year_lsb = CAN_msg[5];
                td_message.tm.tm_wday = dayofweek(td_message.tm.tm_year_msb,td_message.tm.tm_year_lsb, td_message.tm.tm_mon, td_message.tm.tm_mday);
                td_message.msg = SERIAL_MSG_DATE;
                cases = OK;
                }
            else{
                    cases=FAILED;
                }
            break;

        case SERIAL_MSG_ALARM:

            if((CAN_msg[2] < 24u) && (CAN_msg[3] < 60u)){
                td_message.tm.tm_hour=CAN_msg[2];
                td_message.tm.tm_min=CAN_msg[3];
                td_message.msg = SERIAL_MSG_ALARM;
                cases = OK;
                
                }
            else{
                   cases=FAILED;
                }
            break;
        
        case FAILED:
            
            CAN_msg[1]=0xAA;
            CAN_size=1;
            CanTp_SingleFrameTx( &CAN_msg[0],&CAN_size);
            cases=GETMSG;
            break;

        case OK:
            
            CAN_msg[1]=0x55;
            CAN_size=1;
            CanTp_SingleFrameTx( &CAN_msg[0],&CAN_size);
            cases=GETMSG;
            break;

        default:

            cases=GETMSG;
            break;

    }
}