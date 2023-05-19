#include "app_serial.h"
/** 
  * @defgroup CAN_conf values to use CAN.
  @{ */
#define CAN_DATA_LENGHT    8    /*!< Data size of can */
/**
  @} */

/** 
  * @defgroup CAN byte values for confirmation .
  @{ */
#define OK_CANID        0x55    /*!<correct information*/    
#define FAILED_CANID    0xAA    /*!<incorrect information*/
/**
  @} */

/** 
  * @defgroup months months values 
  @{ */
#define JAN 1u     /*!<JANUARY*/
#define FEB 2u     /*!<FEBRUARY*/
#define MAR 3u     /*!<MARCH*/
#define APR 4u     /*!<APRIL*/
#define MAY 5u     /*!<MAY*/
#define JUN 6u     /*!<JUNE*/
#define JUL 7u     /*!<JULY*/
#define AUG 8u     /*!<AUGUST*/
#define SEP 9u     /*!<SEPTEMBER*/
#define OCT 10u    /*!<OCTOBER*/
#define NOV 11u    /*!<NOVEMBER*/
#define DEC 12u    /*!<DECEMBER*/
/**
  @} */

/**
 * @brief APP Messages.
 *
 * This enumeration represents the various types of messages
 */
typedef enum
/* cppcheck-suppress misra-c2012-2.4 ; enum is used on state machine */
{
    SERIAL_MSG_TIME = 1u,
    SERIAL_MSG_DATE,
    SERIAL_MSG_ALARM,
}APP_Messages;

/**
 * @brief State machine states.
 *
 * This enumeration represents the various types of states of the machine
 */
typedef enum
/* cppcheck-suppress misra-c2012-2.4 ; enum is used on state machine */
{
    STATE_TIME = 1U,
    STATE_DATE,
    STATE_ALARM,
    STATE_GETMSG,
    STATE_FAILED,
    STATE_OK
}States;

/**
 * @brief  Variable for CAN configuration
 */
FDCAN_HandleTypeDef CANHandler; 


/**
 * @brief  Flag for CAN msg recive interruption
 */
static uint8_t flag; 

/**
* @brief  Variable for state machien messages.
*/
APP_MsgTypeDef td_message;  //time and date message

/**
* @brief  Variable for cases of the state mahcine.
*/

/**
* @brief  Variable for the data of the state machine.
*/
static uint8_t Data_msg[CAN_DATA_LENGHT];/* cppcheck-suppress misra-c2012-8.9 ; Function does not work if defined in serial task */

/**
* @brief  Variable for the size of the message recived by CAN.
*/
static uint8_t CAN_size;

static uint8_t valid_date(uint8_t day, uint8_t month, uint8_t yearM, uint8_t yearL);
static uint8_t dayofweek(uint32_t yearM, uint32_t yearL, uint32_t month, uint32_t day);
static uint8_t valid_time(uint8_t hour,uint8_t minutes,uint8_t seconds);
static uint8_t valid_alarm(uint8_t hour,uint8_t minutes);

/**
* @brief   **Init function fot serial task(CAN init)**
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
*/
void Serial_Init( void )
{
    
    FDCAN_FilterTypeDef CANFilter;

    CANHandler.Instance                 = FDCAN1;
    CANHandler.Init.Mode                = FDCAN_MODE_NORMAL;
    CANHandler.Init.FrameFormat         = FDCAN_FRAME_CLASSIC;
    CANHandler.Init.ClockDivider        = FDCAN_CLOCK_DIV1;
    CANHandler.Init.TxFifoQueueMode     = FDCAN_TX_FIFO_OPERATION;
    CANHandler.Init.AutoRetransmission  = DISABLE;
    CANHandler.Init.TransmitPause       = DISABLE;
    CANHandler.Init.ProtocolException   = DISABLE;
    CANHandler.Init.ExtFiltersNbr       = 0;
    CANHandler.Init.StdFiltersNbr       = 1;  
    CANHandler.Init.NominalPrescaler    = 10;
    CANHandler.Init.NominalSyncJumpWidth = 1;
    CANHandler.Init.NominalTimeSeg1     = 11;
    CANHandler.Init.NominalTimeSeg2     = 4;
    
    HAL_FDCAN_Init( &CANHandler );

    /* Configure reception filter to Rx FIFO 0, this filter will only show messages of ID 0x111 */
    CANFilter.IdType = FDCAN_STANDARD_ID;
    CANFilter.FilterIndex = 0;
    CANFilter.FilterType = FDCAN_FILTER_MASK;
    CANFilter.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;
    CANFilter.FilterID1 = 0x111;

    HAL_FDCAN_ConfigFilter( &CANHandler, &CANFilter );
    
    /*Messages without the indicaded filter will be rejected*/
    HAL_FDCAN_ConfigGlobalFilter(&CANHandler, FDCAN_REJECT, FDCAN_REJECT, FDCAN_FILTER_REMOTE, FDCAN_FILTER_REMOTE);
    
    /* Change FDCAN instance from initialization mode to normal mode */
    HAL_FDCAN_Start( &CANHandler);
    
    /*we activated the reception interruption in fifo0 when a message arrives*/
    HAL_FDCAN_ActivateNotification( &CANHandler, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0 );
}

/**
* @brief   **Transmit a message to the CAN**
*
*    This function Transmit a message with CanTP_single frame format in wich the first
*    4 bits are 0 and it defines frame type is single frame and the next 4 bits of the
*    first byte are the payload wich cannot be more than 8 bytes, the next 7 bytes
*    are going to be data pointed by data pointer.
*
* @param   *data[in] Pointer of the array that is going to be transmited
* @param   *size[in] Size of the message that is going to be transmited
* @retval  None
*/
static void CanTp_SingleFrameTx( uint8_t *data, uint8_t *size ) 
{
    FDCAN_TxHeaderTypeDef CANTxHeader;
     /* Parameter declaration for CAN transmition */
    CANTxHeader.IdType      = FDCAN_STANDARD_ID;
    CANTxHeader.FDFormat    = FDCAN_CLASSIC_CAN;
    CANTxHeader.TxFrameType = FDCAN_DATA_FRAME;
    CANTxHeader.Identifier  = 0x122;
    CANTxHeader.DataLength  = FDCAN_DLC_BYTES_8;
    uint8_t CAN_msg[CAN_DATA_LENGHT]; 
    if(*size <= 8u )
    {
        CAN_msg[0] = *size;
        for(uint8_t i = 1u; i < 8u; i++)
        {
            CAN_msg[i] = *(data+i-1u);      /* cppcheck-suppress misra-c2012-18.4 ; operators to pointers needed */
        }
        HAL_FDCAN_AddMessageToTxFifoQ( &CANHandler, &CANTxHeader, CAN_msg );
    }
}


/**
* @brief   **Gets a message of the CAN communication**
*
*  The function gets a message from the CAN communication and it checks if the message has the correct format
*  for single frame, first gets the message from CAN and checks if the first 4 bits of the payload are 0
*  to define if it is a single frame message, then we check if the next 4 bits are less or equal than 8 since
*  is goint to be the size of the payload, if those values are true then we give that value to the pointer size
*  and the payload we store it on the CAN_msg variable 
*
* @param   *data[in] Pointer of the array where de data is going to be store
* @param   *size[in] Size of the message that`s been recived
* @retval  Return is 0 if data is unvalid and 1 if it is valid
*/
static uint8_t CanTp_SingleFrameRx( uint8_t *data, uint8_t *size )
{
    uint8_t CAN_msg[CAN_DATA_LENGHT]; 
    uint8_t msg_recived = FALSE;
    FDCAN_RxHeaderTypeDef CANRxHeader;

    HAL_FDCAN_GetRxMessage( &CANHandler, FDCAN_RX_FIFO0, &CANRxHeader, CAN_msg ); 
    if ( ((CAN_msg[0] >> 4u) == 0u) && (CAN_msg[0] <= 8u) )
    {
        *size = CAN_msg[0];
        for(uint8_t i = 0u; i < 7u; i++)
        {
            *(data+i) = CAN_msg[i+1u];      /* cppcheck-suppress misra-c2012-18.4 ; operators to pointers needed */
        }
        msg_recived = TRUE;
    }
   
    return msg_recived;
}


/**
* @brief   **This is an interruption function for the CAN  **
*
* this function is an interruption that is called when a message is recived throught the CAN,
* when a message is recived we call CanTp_SingleFrameRx to check if the data is correct.
*
* @param   *hfdcan[in] structure of CAN.
* @param   *RxFifo0ITs[in] .
* @param   flag[out] flag for new message.
* @retval  None
*/
/* cppcheck-suppress misra-c2012-2.7 ; this is a library function */
void HAL_FDCAN_RxFifo0Callback( FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs ) /* cppcheck-suppress misra-c2012-8.4 ; this is a library function */
{
    /*A llegado un mensaje via CAN, interrogamos si fue un solo mensaje*/
    if( ( RxFifo0ITs & FDCAN_IT_RX_FIFO0_NEW_MESSAGE ) != 0 )
    {
        
        if(CanTp_SingleFrameRx( Data_msg,&CAN_size) == TRUE)
        {
            flag = TRUE;  
        }
    }
}


/**
* @brief   **Function that checks if the date is valid**
*
* The fucntion checks the parameters so that they are a valid date
*
* @param   day[in]       day of the month
* @param   month[in]     month of the year
* @param   year[in]      year of the date
*
* @retval  if the day is valid it will return 1 otherwise a 0.
*/
uint8_t valid_date(uint8_t day, uint8_t month, uint8_t yearM, uint8_t yearL)
{
    uint32_t year = ((uint32_t)(yearM) * 100u) + (uint32_t)yearL;
    uint32_t flagd = TRUE;

    if ((day > 0u ) && (day <= 31u) && (month <= DEC) && (month >= JAN) && (year >= 1900u) && (year <= 2100u)) 
    {

        flagd = TRUE;

        if ((month == JAN) || (month == MAR) || (month == MAY) || (month == JUL) || (month == AUG) || (month == OCT) || (month == DEC)) 
        {
            if (day > 31u)
            {
                flagd = FALSE;
            }
        } 
        else if ((month == APR) || (month == JUN) || (month == SEP) || (month == NOV)) 
        {
            if (day > 30u) 
            {
                flagd = FALSE;
            }

        } 
        else if (month == FEB) 
        {
            if ((((year % 4u) == 0u) && ((year % 100u) != 0u)) || ((year % 400u) == 0u)) 
            {
                if (day > 29u) 
                {
                    flagd = FALSE;
                }
            } 
            else 
            {
                if (day > 28u) 
                {
                    flagd = FALSE;
                }
            }
        } else{}
    } 
    else 
    {
        flagd = FALSE;
    }

    return flagd;
}


/**
* @brief   **Gets the day of the week**
*
* this function take the date and uses the zeller's congruence formula to get the day of the week.
*
* @param   day[in]       day of the month
* @param   month[in]     month of the year
* @param   year[in]      year of the date
*
* @retval  returns a value of the day of the week defines. 
*/
uint8_t dayofweek(uint32_t yearM, uint32_t yearL, uint32_t month, uint32_t day)
{
    uint32_t year = ((uint32_t)(yearM) * 100u) + (uint32_t)yearL;
    uint32_t m = month;
    uint32_t c;
    uint32_t y;
    uint32_t d;
    uint32_t w;
   if (m < 3u) {
        m += 12u;
        year--;
    }
    c = year / 100u;
    y = year - (100u * c);
    d = day;
    w = (( ((13u * m) + 3u) / 5u) + d + y + (y / 4u) + (c / 4u) - (2u * c));
    w %= 7;
    if (w < 0u)
    {
        w += 7u;
    }
    return w;
}

/**
* @brief   **The fucntion validates the parameters for time**
*
* @param   hour[in]        hour to be validated
* @param   minutes[in]     minutes to be validated
* @param   seconds[in]     seconds to be validated
*
* @retval  Time_is_valid[out]    if 0 if data is unvalid and 1 if it is valid
*/
uint8_t valid_time(uint8_t hour,uint8_t minutes,uint8_t seconds)
{
    uint8_t Time_is_valid = FALSE;
    if((hour < 24u) && (minutes < 60u) && (seconds < 60u))
    {
        Time_is_valid = TRUE;
    }
    return Time_is_valid;
}

/**
* @brief   **The fucntion validates the parameters for alarm**
*
* @param   hour[in]        hour to be validated
* @param   minutes[in]     minutes to be validated
*
* @retval  Time_is_valid[out]    if 0 if data is unvalid and 1 if it is valid
*/
uint8_t valid_alarm(uint8_t hour,uint8_t minutes)
{
    uint8_t Time_is_valid = FALSE;

    if((hour < 24u) && (minutes < 60u))
    {
        Time_is_valid = TRUE;
    }
    return Time_is_valid;
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
*/
void Serial_Task( void )
{
    static uint8_t cases = STATE_GETMSG ; /* cppcheck-suppress misra-c2012-8.9 ; Function does not work if defined in serial task */
    switch(cases)
    {

        case STATE_GETMSG:

            if (flag == TRUE)
            {
                flag = FALSE;
                if(Data_msg[0] == (uint8_t)SERIAL_MSG_TIME)
                {
                    cases = (uint8_t)STATE_TIME;
                }
                else if(Data_msg[0] == (uint8_t)SERIAL_MSG_DATE)
                {
                    cases = (uint8_t)STATE_DATE;
                }
                else if(Data_msg[0] == (uint8_t)SERIAL_MSG_ALARM)
                {
                    cases = (uint8_t)STATE_ALARM;
                }  
                else{}
            } 
            else{}

            break;

       
        case STATE_TIME:
            
            if( valid_time(Data_msg[1],Data_msg[2],Data_msg[3]) == TRUE)
            {
                td_message.tm.tm_hour=Data_msg[1];
                td_message.tm.tm_min=Data_msg[2];
                td_message.tm.tm_sec=Data_msg[3];
                td_message.msg=SERIAL_MSG_TIME;
                cases = STATE_OK;
            }
            else
            {
                cases = STATE_FAILED;
            }
             break;

        case STATE_DATE:

            if(valid_date(Data_msg[1],Data_msg[2], Data_msg[3],Data_msg[4]) == TRUE)
            {
                td_message.tm.tm_mday = Data_msg[1];
                td_message.tm.tm_mon = Data_msg[2];
                td_message.tm.tm_year_msb = Data_msg[3];
                td_message.tm.tm_year_lsb = Data_msg[4];
                td_message.tm.tm_wday = dayofweek(td_message.tm.tm_year_msb,td_message.tm.tm_year_lsb, td_message.tm.tm_mon, td_message.tm.tm_mday);
                td_message.msg = SERIAL_MSG_DATE;
                cases = STATE_OK;
            }
            else
            {
                cases = STATE_FAILED;
            }
            break;

        case STATE_ALARM:

            if(valid_alarm( Data_msg[1],Data_msg[2]) == TRUE)
            {
                td_message.tm.tm_hour=Data_msg[1];
                td_message.tm.tm_min=Data_msg[2];
                td_message.msg = SERIAL_MSG_ALARM;
                cases = STATE_OK;
            }
            else
            {
                cases = STATE_FAILED;
            }
            break;
        
        case STATE_FAILED:
            
            Data_msg[0]=FAILED_CANID;
            CAN_size=1;
            CanTp_SingleFrameTx( &Data_msg[0],&CAN_size);
            cases = STATE_GETMSG;
            break;

        case STATE_OK:
            
            Data_msg[0]=OK_CANID;
            CAN_size=1;
            CanTp_SingleFrameTx( &Data_msg[0],&CAN_size);
            cases = STATE_GETMSG;
            break;
 
        default:

            cases = STATE_GETMSG;
            break;
    }
}