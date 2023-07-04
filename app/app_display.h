/**
* @file    app_display.h
* @brief   **Header file for app_display.c**
*
*   This file contains the declaration for the functions on the .c file
*   And also has the declarations of the variables that we need.
*   To use this aplication you need to first use the Display_Init function
*   and then you can call the Display_Task.   
*          
*/
#ifndef APP_DISPLAY_H__
#define APP_DISPLAY_H__

    #include "app_bsp.h"

    void Display_Init( void );
    void Display_Task( void );
    void Display_LcdTask( void );

#endif