/**
* @file    <app_analog.h>
* @brief   **Header file for app_analog.c**
*
*   This file contains the declaration for the functions on the .c file
*   And also has the declarations of the variables that we need.
*   To use this aplication you need to first use the clock init function
*   and then you can call the Task function.
* @note    
*          
*/
#ifndef APP_ANALOG_H__
#define APP_ANALOG_H__

#include "app_bsp.h"
#include "hel_lcd.h"

void Analogs_Init( void );
int8_t Analogs_GetTemperature( void );
void Display_LcdTask( void );

#endif