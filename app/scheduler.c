/**
* @file    scheduler.c
* @brief   **scheduler functions**
*
*   This is a reusable driver for a scheduler, this files contains all the functions
*   implementation declared on the scheduler.h file       
*/

#include "scheduler.h"
/**
* @brief   **This function initializes the parameters for the scheduler**
*
*  This function initialize the taskcount to 0, also checks
*  if the other parameters are valid, in case they`re not the program
*  will be sent to a safe state.
* @param   hscheduler[in] Pointer to a Scheduler_HandleTypeDef structure
* 
*/
void HIL_SCHEDULER_Init( Scheduler_HandleTypeDef *hscheduler )
{
    assert_error( (hscheduler->taskPtr != NULL), SCHEDULER_ERROR ); /* cppcheck-suppress misra-c2012-11.8 ; function cannot be modify */
    assert_error( (hscheduler->tasks != FALSE), SCHEDULER_ERROR );  /* cppcheck-suppress misra-c2012-11.8 ; function cannot be modify */
    assert_error( (hscheduler->tick != FALSE), SCHEDULER_ERROR );   /* cppcheck-suppress misra-c2012-11.8 ; function cannot be modify */
    hscheduler->tasksCount = 0;
}