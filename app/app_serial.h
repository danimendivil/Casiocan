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

void Serial_Init( void );
void Serial_Task( void );

/** 
  * @defgroup <APP_Messages> machine states for the CAN msg.
  @{ */
#define    SERIAL_MSG_TIME     1u
#define    SERIAL_MSG_DATE     2u
#define    SERIAL_MSG_ALARM    3u
/**
  @} */

/** 
  * @defgroup <CAS_STATES> other machine states 
  @{ */
#define    GETMSG              4u
#define    FAILED              5u
#define    OK                  6u
/**
  @} */


typedef struct _APP_TmTypeDef 
{
    uint32_t tm_sec;         /* seconds,  range 0 to 59          */
    uint32_t tm_min;         /* minutes, range 0 to 59           */
    uint32_t tm_hour;        /* hours, range 0 to 23             */
    uint32_t tm_mday;        /* day of the month, range 1 to 31  */
    uint32_t tm_mon;         /* month, range 0 to 11             */
    uint32_t tm_year_msb;        /* years in rage 1900 2100          */
    uint32_t tm_year_lsb;
    uint32_t tm_wday;        /* day of the week, range 0 to 6    */
    uint32_t tm_yday;        /* day in the year, range 0 to 365  */
    uint32_t tm_isdst;       /* daylight saving time             */
}APP_TmTypeDef;

typedef struct _APP_MsgTypeDef
{
    uint8_t msg;          /*!< Store the message type to send */
    APP_TmTypeDef tm;     /*!< time and date in stdlib tm format */
}APP_MsgTypeDef;
/**
 * @brief  Variable for CAN configuration
 */
extern FDCAN_HandleTypeDef CANHandler;
/**
 * @brief  Variable for CAN transmition configuration
 */
extern FDCAN_TxHeaderTypeDef CANTxHeader;
/**
 * @brief  Variable for CAN reception configuration
 */
extern FDCAN_RxHeaderTypeDef CANRxHeader;
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
extern uint8_t sizer;
/**
 * @brief  Array for CAN DATA
 */
extern uint8_t datar[8];  
/**
 * @brief  Variable for Cases of the state machine
 */
extern uint8_t cases;
/**
 * @brief  Variable for MSG send by state machine
 */
extern APP_MsgTypeDef mtm;





#endif