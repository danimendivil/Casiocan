#ifndef APP_SERIAL_H__
#define APP_SERIAL_H__
/**
* @file    app_serial.h
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
extern uint8_t CAN_msg[8];  
/**
 * @brief  Variable for Cases of the state machine
 */





#endif