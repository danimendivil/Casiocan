/**
* @file    <app_clock.h>
* @brief   **Header file for app_clock.c**
*
*   This file contains the declaration for the functions on the .c file
*   And also has the declarations of the variables that we need.
*   To use this aplication you need to first use the clock init function
*   and then you can call the Task function.
* @note    
*          
*/
#ifndef APP_CLOCK_H__
#define APP_CLOCK_H__

#include "app_bsp.h"
#include "scheduler.h"

void Clock_Init( void );
void Clock_Task( void );
void Display_msg(void); 

/**
  * @brief  Variable for scheduler.
  */
  extern Scheduler_HandleTypeDef sched;  

#endif