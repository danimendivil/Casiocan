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


#endif