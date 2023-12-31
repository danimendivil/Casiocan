#include "app_serial.h"
#include "hil_queue.h"
/** 
  * @defgroup CAN_conf values to use CAN.
  @{ */
#define CAN_DATA_LENGHT    8    /*!< Data size of can */
#define CAN_DATA_PER10MS   10    /*!< Number of can transmitions per 10 ms*/
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
* @defgroup Data size of states .
@{ */
#define TIME_DATA_SIZE  4U      /*!<Data size needed for time state*/
#define DATE_DATA_SIZE  5U      /*!<Data size needed for date state*/
#define ALARM_DATA_SIZE 3U      /*!<Data size needed for alarm state*/
/**
  @} */

/** 
  * @defgroup months months values 
  @{ */
#define JAN 0x01u     /*!<JANUARY*/
#define FEB 0x02u     /*!<FEBRUARY*/
#define MAR 0x03u     /*!<MARCH*/
#define APR 0x04u     /*!<APRIL*/
#define MAY 0x05u     /*!<MAY*/
#define JUN 0x06u     /*!<JUNE*/
#define JUL 0x07u     /*!<JULY*/
#define AUG 0x08u     /*!<AUGUST*/
#define SEP 0x09u     /*!<SEPTEMBER*/
#define OCT 0x10u    /*!<OCTOBER*/
#define NOV 0x11u    /*!<NOVEMBER*/
#define DEC 0x12u    /*!<DECEMBER*/
/**
  @} */

/** 
  * @defgroup array_pos array positions
  @{ */
#define array_pos_0 0     /*!<array position 0*/
#define array_pos_1 1     /*!<array position 1*/
#define array_pos_2 2     /*!<array position 2*/
#define array_pos_3 3     /*!<array position 3*/
#define array_pos_4 4     /*!<array position 4*/
#define array_pos_5 5     /*!<array position 5*/
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
    STATE_IDLE = 0U,
    STATE_TIME,
    STATE_DATE,
    STATE_ALARM,
    STATE_FAILED,
    STATE_OK,
}States;

/**
 * @brief  Variable for CAN configuration
 */
FDCAN_HandleTypeDef CANHandler; 

/**
* @brief  Variable for state machien messages.
*/
static APP_MsgTypeDef CAN_td_message;  //time and date message

/**
* @brief  Circular buffer variable for CAN msg recived to serial task.
*/
static QUEUE_HandleTypeDef CAN_queue;

/**
* @brief  Circular buffer variable for CAN msg recived to serial task.
*/
QUEUE_HandleTypeDef SERIAL_queue;

static uint8_t valid_date(uint8_t day, uint8_t month, uint8_t yearM, uint8_t yearL);
static uint8_t dayofweek(uint32_t yearM, uint32_t yearL, uint32_t month, uint32_t day);
static uint8_t valid_time(uint8_t hour,uint8_t minutes,uint8_t seconds);
static uint8_t valid_alarm(uint8_t hour,uint8_t minutes);
static uint8_t bcdToDecimal(uint8_t bcdValue); 
static void Serial_StMachine(uint8_t cases );
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
*   since the CAN transmition speed is 100kbps the buffer array will be of 10 position considering 
*   the following calculations:
*   Number of can messages per second = speed of can transmition / can lenght
*   Number of can messages per second = 100Kbps / 108 bits = 925 transmitions per second
*   the serial task will be executed every 10ms so now we need the transmitions per 10ms
*   tansmition per 10ms = (10ms * 925transmitions) / 1000ms = 9.25 transmitions.
*   we round upwards so the array will be of 10 positions.
*/
void Serial_Init( void )
{
    FDCAN_FilterTypeDef CANFilter;
    CAN_td_message.tm.tm_year_msb = 20;

    CANHandler.Instance                 = FDCAN1;
    CANHandler.Init.Mode                = FDCAN_MODE_NORMAL;
    CANHandler.Init.FrameFormat         = FDCAN_FRAME_CLASSIC;
    CANHandler.Init.ClockDivider        = FDCAN_CLOCK_DIV2;
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
    
    Status = HAL_FDCAN_Init( &CANHandler );
    assert_error( Status == HAL_OK, FDCAN_CONFIG_ERROR );   /* cppcheck-suppress misra-c2012-11.8 ; function cannot be modify */
    /* Configure reception filter to Rx FIFO 0, this filter will only show messages of ID 0x111 */
    CANFilter.IdType = FDCAN_STANDARD_ID;
    CANFilter.FilterIndex = 0;
    CANFilter.FilterType = FDCAN_FILTER_MASK;
    CANFilter.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;
    CANFilter.FilterID1 = 0x111;

    Status = HAL_FDCAN_ConfigFilter( &CANHandler, &CANFilter );
    assert_error( Status == HAL_OK, FDCAN_CONFIG_FILTER_ERROR );    /* cppcheck-suppress misra-c2012-11.8 ; function cannot be modify */
    
    /*Messages without the indicaded filter will be rejected*/
    Status = HAL_FDCAN_ConfigGlobalFilter(&CANHandler, FDCAN_REJECT, FDCAN_REJECT, FDCAN_FILTER_REMOTE, FDCAN_FILTER_REMOTE);
    assert_error( Status == HAL_OK, FDCAN_CONFIG_GLOBAL_FILTER_ERROR ); /* cppcheck-suppress misra-c2012-11.8 ; function cannot be modify */

    /* Change FDCAN instance from initialization mode to normal mode */
    Status = HAL_FDCAN_Start( &CANHandler);
    assert_error( Status == HAL_OK, FDCAN_START_ERROR );    /* cppcheck-suppress misra-c2012-11.8 ; function cannot be modify */
    
    /*we activated the reception interruption in fifo0 when a message arrives*/
    Status = HAL_FDCAN_ActivateNotification( &CANHandler, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0 );
    assert_error( Status == HAL_OK, FDCAN_ACTIVATE_NOTIFICATION_ERROR );    /* cppcheck-suppress misra-c2012-11.8 ; function cannot be modify */

    /*CAN Buffer configuration*/
    static uint64_t can_queue_store[CAN_DATA_PER10MS];
    CAN_queue.Buffer = can_queue_store;
    CAN_queue.Elements = CAN_DATA_PER10MS;
    CAN_queue.size = sizeof(uint64_t);
    HIL_QUEUE_Init(&CAN_queue);

    /*Serial to clock Buffer configuration*/
    static APP_MsgTypeDef serial_queue_store[CAN_DATA_PER10MS];
    SERIAL_queue.Buffer = serial_queue_store;
    SERIAL_queue.Elements = CAN_DATA_PER10MS;
    SERIAL_queue.size = sizeof(APP_MsgTypeDef);
    HIL_QUEUE_Init(&SERIAL_queue);
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
        Status = HAL_FDCAN_AddMessageToTxFifoQ( &CANHandler, &CANTxHeader, CAN_msg );
        assert_error( Status == HAL_OK, FDCAN_ADDMESSAGE_ERROR );   /* cppcheck-suppress misra-c2012-11.8 ; function cannot be modify */
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
    uint8_t msg_recived = FALSE;
    
    if ( ((*data >> 4u) == 0u) && (*data <= 8u) )
    {
        *size = *data;
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
    FDCAN_RxHeaderTypeDef CANRxHeader;
    
    /*A llegado un mensaje via CAN, interrogamos si fue un solo mensaje*/
    if( ( RxFifo0ITs & FDCAN_IT_RX_FIFO0_NEW_MESSAGE ) != 0 )
    {
        uint8_t Canmsg[CAN_DATA_LENGHT];
        Status = HAL_FDCAN_GetRxMessage( &CANHandler, FDCAN_RX_FIFO0, &CANRxHeader, Canmsg ); 
        assert_error( Status == HAL_OK, FDCAN_GETMESSAGE_ERROR );   /* cppcheck-suppress misra-c2012-11.8 ; function cannot be modify */
        (void)HIL_QUEUE_Write( &CAN_queue, Canmsg );
        
    }
}

/**
 * @brief   **This function gets the decimal value of a bcd  **
 *
 *  In this function, the input bcdValue is assumed to be an 8-bit value representing a number in BCD format. 
 *  The function extracts the tens digit and ones digit from the BCD value using bitwise operations and masks. 
 *  Then, it calculates the decimal value by multiplying the tens digit by 10 and adding the ones digit.
 *
 * @param   bcdValue[in] Indicates the bcd value that we want on decimal
 *
 * @retval  is a uint8_t can be any value        
 */
uint8_t bcdToDecimal(uint8_t bcdValue) 
{
    uint8_t tens = (bcdValue >> 4u) & 0x0Fu;  // Extract the tens digit
    uint8_t ones = bcdValue & 0x0Fu;         // Extract the ones digit
    uint8_t decimalValue = (tens * 10u) + ones;  // Compute the decimal value

    return decimalValue;
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

    if ((day > 0x00u ) && (day <= 0x031u) && (month <= DEC) && (month >= JAN) && (yearM >= 0x19u) && (yearM <= 0x20u)) 
    {

        flagd = TRUE;

        if ((month == JAN) || (month == MAR) || (month == MAY) || (month == JUL) || (month == AUG) || (month == OCT) || (month == DEC)) 
        {
            if (day > 0x31u)
            {
                flagd = FALSE;
            }
        } 
        else if ((month == APR) || (month == JUN) || (month == SEP) || (month == NOV)) 
        {
            if (day > 0x30u) 
            {
                flagd = FALSE;
            }

        } 
        else if (month == FEB) 
        {
            if ((((year % 4u) == 0u) && ((year % 100u) != 0u)) || ((year % 400u) == 0u)) 
            {
                if (day > 0x29u) 
                {
                    flagd = FALSE;
                }
            } 
            else 
            {
                if (day > 0x28u) 
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
    if((hour < 0x24u) && (minutes < 0x60u) && (seconds < 0x60u))
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

    if((hour < 0x24u) && (minutes < 0x60u))
    {
        Time_is_valid = TRUE;
    }
    return Time_is_valid;
}

static uint8_t Data_msg[CAN_DATA_LENGHT];
static uint8_t CAN_size;
/**
* @brief   **This function executes the serial state machine**
*
*   This functions executes the state machine of the serial task
*   every 10ms, we do this because a circular buffer has been implemented on the serial
*   task, this means that we do need to execute every time the task since now the 
*   information is being stored.
*   will be using the HIL_QUEUE_IsEmpty to see if the circular buffer has any message and if it does
*   then it will call the function CanTp_SingleFrameRx to see if the message readed by the circular buffer
*   is a single frame format, if it is then it will give cases the data of position 1 of the array
*   wich tells wath action is needed, if it is not then cases will be STATE_FAILED so that when Serial_StMachine
*   is called it will enter the STATE_FAILED.
*/
void Serial_Task( void )
{     
    /*poll the state machine until the queue is empty and it return to IDLE*/
    while( HIL_QUEUE_IsEmptyISR( &CAN_queue, TIM16_FDCAN_IT0_IRQn ) == FALSE )
    {
        /*Read the first message*/
        (void)HIL_QUEUE_ReadISR( &CAN_queue, &Data_msg, TIM16_FDCAN_IT0_IRQn );
        if((CanTp_SingleFrameRx( Data_msg, &CAN_size )) == TRUE)
        {
            Serial_StMachine(Data_msg[array_pos_1]); 
        }
        
        else
        {
            Data_msg[array_pos_1] = STATE_FAILED;
            Serial_StMachine(Data_msg[array_pos_1]);
        }
    }
}

/**
* @brief   **Serial state machine function**
*     
*   the function will only be called after the message readed by the circular buffer
*   is compatible with the can single frame format, if the value of cases is equal to 
*   SERIAL_MSG_TIME it validates the values and if the values are correct they are store on the CAN_td_message variable
*   and the variable is send to a queue with HIL_QUEUE_Write and cases value is change to STATE_OK. if they are not then
*   cases will be STATE_FAILED.
*   If cases is SERIAL_MSG_DATE it validates the values with the valid_date() function and if the date is valid
*   it also calls the dayofweek function to get the day of the week if they are valid then they are store on the CAN_td_message variable 
*   and the variable is send to a queue with HIL_QUEUE_Write and cases value is change to STATE_OK. if they are not then
*   cases will be STATE_FAILED.
*   if the value is SERIAL_MSG_ALARM it validates the data and if they are correct are store on the CAN_td_message variable and 
*   the variable is send to a queue with HIL_QUEUE_Write and cases value is change to STATE_OK. if they are not then
*   cases will be STATE_FAILED.
*   then if cases is STATE_FAILED a message will be send in can with an id that indicates that the message was not compatible
*   and if cases is STATE_OK a message will be send in can with an id that indicates that the message correct.
*   when an alarm is active this function will not send any message instead it will trigger the alarm flag
*   so that the alarm stops but only if the message arrive is a STATE_TIME,STATE_DATE or STATE_ALARM.
*/
static void Serial_StMachine(uint8_t cases )
{
    static uint8_t Event[CAN_DATA_LENGHT];
    switch(cases)
    {
        case STATE_TIME:

            if(CAN_size == TIME_DATA_SIZE)
            {
                if( valid_time(Data_msg[array_pos_2],Data_msg[array_pos_3],Data_msg[array_pos_4]) == TRUE)
                {
                    CAN_td_message.tm.tm_hour=Data_msg[array_pos_2];
                    CAN_td_message.tm.tm_min=Data_msg[array_pos_3];
                    CAN_td_message.tm.tm_sec=Data_msg[array_pos_4];
                    CAN_td_message.msg=SERIAL_MSG_TIME;
                    Event[array_pos_0] = TRUE; 
                    Event[array_pos_1] = STATE_OK; 
                    (void)HIL_QUEUE_WriteISR( &CAN_queue, Event, TIM16_FDCAN_IT0_IRQn  );
                }
                else
                {
                    Event[array_pos_0] = TRUE; 
                    Event[array_pos_1] = STATE_FAILED; 
                    (void)HIL_QUEUE_WriteISR( &CAN_queue, Event, TIM16_FDCAN_IT0_IRQn  );
                }
            }
        break;

        case STATE_DATE:
            if(CAN_size == DATE_DATA_SIZE)
            {
                if(valid_date(Data_msg[array_pos_2],Data_msg[array_pos_3], Data_msg[array_pos_4],Data_msg[array_pos_5]) == TRUE)
                {
                    CAN_td_message.tm.tm_mday = Data_msg[array_pos_2];
                    CAN_td_message.tm.tm_mon = Data_msg[array_pos_3];
                    CAN_td_message.tm.tm_year_msb = bcdToDecimal(Data_msg[array_pos_4]);
                    CAN_td_message.tm.tm_year_lsb = Data_msg[array_pos_5];
                    CAN_td_message.tm.tm_wday = dayofweek(CAN_td_message.tm.tm_year_msb,CAN_td_message.tm.tm_year_lsb, CAN_td_message.tm.tm_mon, CAN_td_message.tm.tm_mday);
                    CAN_td_message.msg = SERIAL_MSG_DATE;
                    Event[array_pos_0] = TRUE; 
                    Event[array_pos_1] = STATE_OK; 
                    (void)HIL_QUEUE_WriteISR( &CAN_queue, Event, TIM16_FDCAN_IT0_IRQn  );
                }
                else
                {
                    Event[array_pos_0] = TRUE; 
                    Event[array_pos_1] = STATE_FAILED; 
                    (void)HIL_QUEUE_WriteISR( &CAN_queue, Event, TIM16_FDCAN_IT0_IRQn  );
                }
            }
        break;

        case STATE_ALARM:
            if(CAN_size == ALARM_DATA_SIZE)
            {
                if(valid_alarm( Data_msg[array_pos_2],Data_msg[array_pos_3]) == TRUE)
                {
                    CAN_td_message.tm.tm_hour=Data_msg[array_pos_2];
                    CAN_td_message.tm.tm_min=Data_msg[array_pos_3];
                    CAN_td_message.msg = SERIAL_MSG_ALARM;
                    Event[array_pos_0] = TRUE; 
                    Event[array_pos_1] = STATE_OK; 
                    (void)HIL_QUEUE_WriteISR( &CAN_queue, Event, TIM16_FDCAN_IT0_IRQn  );
                }
                else
                {
                    Event[array_pos_0] = TRUE; 
                    Event[array_pos_1] = STATE_FAILED; 
                    (void)HIL_QUEUE_WriteISR( &CAN_queue, Event, TIM16_FDCAN_IT0_IRQn  );
                }
            }
        break;

        case STATE_OK:
            (void)HIL_QUEUE_WriteISR( &SERIAL_queue, &CAN_td_message, TIM16_FDCAN_IT0_IRQn);
            Data_msg[array_pos_0]=OK_CANID;
            CAN_size=TRUE;
            CanTp_SingleFrameTx( &Data_msg[array_pos_0],&CAN_size); 
        break;

        case STATE_FAILED:
            Data_msg[array_pos_0]=FAILED_CANID;
                CAN_size=TRUE;
            CanTp_SingleFrameTx( &Data_msg[array_pos_0],&CAN_size);
        break;

        default:
        break;
    }
}