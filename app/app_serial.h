#ifndef APP_SERIAL_H__
#define APP_SERIAL_H__
/**
* @file    <app_serial.h>
* @brief   **Header file for app_serial.c**
*
* This program has 2 main function Serial_Init and Serial_Task, Serial Init is an initialization 
* function for a CAN Transmition and recepcion, Serial_Task is a function that has a state machine
* where it inrepret and validates the CAN msg and does a task acording to the msg.
* 
*
* @note    Always use the Serial_Init function first
*          
*/
#include "app_bsp.h"
#define CAN_DATA_LENGHT    8

void Serial_Init( void );
void Serial_Task( void );

/** 
  * @defgroup <APP_Messages> machine states for the CAN msg.
  @{ */
#define    SERIAL_MSG_TIME     1u   /*!< State for changing the time of the clock*/
#define    SERIAL_MSG_DATE     2u   /*!< State for changing the date of the clock*/
#define    SERIAL_MSG_ALARM    3u   /*!< State for changing the alarm of the clock*/
/**
  @} */



/** 
  * @defgroup <BOOL> Boolean values.
  @{ */
#define    TRUE     1u   /*!< State for changing the time of the clock*/
#define    FALSE     0u   /*!< State for changing the date of the clock*/
/**
  @} */

/** 
  * @defgroup <CAS_STATES> other machine states 
  @{ */
#define    GETMSG              4u   /*!< State thats get the message from CAN transmition*/
#define    FAILED              5u   /*!< State thats sends a message through CAN when bad information*/
#define    OK                  6u   /*!< State thats sends a message through CAN when information is correct*/
/**
  @} */


/**
  * @brief   Structure for message that are recived by CAN to store time and date in stdlib tm format 
  */
typedef struct _APP_TmTypeDef 
{
    uint32_t tm_sec;         /*!< seconds,  range 0 to 59          */
    uint32_t tm_min;         /*!< minutes, range 0 to 59           */
    uint32_t tm_hour;        /*!< hours, range 0 to 23             */
    uint32_t tm_mday;        /*!< day of the month, range 1 to 31  */
    uint32_t tm_mon;         /*!< month, range 0 to 11             */
    uint32_t tm_year_msb;    /*!< year most significant bits in range 1900 2100       */
    uint32_t tm_year_lsb;    /*!< year least significant bits in range 1900 2100      */
    uint32_t tm_wday;        /*!< day of the week, range 0 to 6    */
    uint32_t tm_yday;        /*!< day in the year, range 0 to 365  */
    uint32_t tm_isdst;       /*!< daylight saving time             */
}APP_TmTypeDef;


/**
  * @brief   structure with APP_TmTypeDef and a message 
  */
typedef struct _APP_MsgTypeDef
{
    uint8_t msg;          /*!< Store the message type to send */
    APP_TmTypeDef tm;     /*!< time and date in stdlib tm format */
}APP_MsgTypeDef;
/**
 * @brief  Variable for CAN configuration
 */
extern FDCAN_HandleTypeDef CANHandler; /* cppcheck-suppress misra-c2012-8.5 ; other declaration is used on ints */
/**
 * @brief  Variable for CAN transmition configuration
 */
extern FDCAN_TxHeaderTypeDef CANTxHeader;
/**
 * @brief  Variable for CAN reception configuration
 */
extern FDCAN_RxHeaderTypeDef CANRxHeader; //Variable for CAN filter configuration
/**
 * @brief  Variable for CAN filter configuration
 */
extern FDCAN_FilterTypeDef CANFilter;
/**
 * @brief  Flag for CAN msg recive interruption
 */
extern uint8_t flag; 
/**
 * @brief  Variable for the size of CAN messages
 */
extern uint8_t CAN_size;
/**
 * @brief  Array for CAN DATA
 */
extern uint8_t CAN_msg[CAN_DATA_LENGHT];  
/**
 * @brief  Variable for Cases of the state machine
 */
extern uint8_t cases;
/**
 * @brief  Variable for MSG send by state machine
 */
extern APP_MsgTypeDef td_message;




#endif