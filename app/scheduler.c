/**
* @file    scheduler.c
* @brief   **scheduler functions**
*
*    This is a reusable driver for a scheduler, this files contains all the functions
*    implementation declared on the scheduler.h file       
*/

#include "scheduler.h"

/**
* @brief   **This function initializes the parameters for the scheduler**
*
*   This function initialize the taskcount to 0, also checks
*   if the other parameters are valid, in case they`re not the program
*   will be sent to a safe state.
*
* @param   hscheduler[in] Pointer to a Scheduler_HandleTypeDef structure 
*/
void HIL_SCHEDULER_Init( Scheduler_HandleTypeDef *hscheduler )
{
    assert_error( (hscheduler->taskPtr != NULL), SCHEDULER_ERROR ); /* cppcheck-suppress misra-c2012-11.8 ; function cannot be modify */
    assert_error( (hscheduler->tasks != FALSE), SCHEDULER_ERROR );  /* cppcheck-suppress misra-c2012-11.8 ; function cannot be modify */
    assert_error( (hscheduler->tick != FALSE), SCHEDULER_ERROR );   /* cppcheck-suppress misra-c2012-11.8 ; function cannot be modify */
    hscheduler->tasksCount = 0;
}

/**
* @brief   **This function register a task for the scheduler**
*
*   this function sets the hscheduler with the address of the function to hold the init routine for the given task
*   and the address for the actual routine that will run as the task, plus the periodicity in milliseconds of the task to register,
*   the Periodicity should not be less than the tick value and always be multiple. 
*
* @param   hscheduler[in] Pointer to a Scheduler_HandleTypeDef structure 
* @param   InitPtr[in] Pointer to the init function 
* @param   TaskPtr[in] Pointer to the task function 
* @param   Period[in] period of the task to be executed 
* @retval  Task_ID Is number from 1 to n task registered if the operation was a success, otherwise, it will return zero. 
*/
uint8_t HIL_SCHEDULER_RegisterTask( Scheduler_HandleTypeDef *hscheduler, void (*InitPtr)(void), void (*TaskPtr)(void), uint32_t Period )
{
    assert_error( (TaskPtr != NULL), SCHEDULER_ERROR ); /* cppcheck-suppress misra-c2012-11.8 ; function cannot be modify */
    assert_error( (hscheduler->taskPtr != NULL), SCHEDULER_ERROR ); /* cppcheck-suppress misra-c2012-11.8 ; function cannot be modify */
    assert_error( (hscheduler->tasks != FALSE), SCHEDULER_ERROR );  /* cppcheck-suppress misra-c2012-11.8 ; function cannot be modify */
    assert_error( (hscheduler->tick != FALSE), SCHEDULER_ERROR );   /* cppcheck-suppress misra-c2012-11.8 ; function cannot be modify */
    uint8_t Task_ID;
    if((Period > hscheduler->tick) && ((Period % (hscheduler->tick)) == FALSE) )
    {
        ((hscheduler->taskPtr) + hscheduler->tasksCount)->period = Period;      /* cppcheck-suppress misra-c2012-18.4 ; operator to pointer is needed */
        ((hscheduler->taskPtr) + hscheduler->tasksCount)->initFunc = InitPtr;   /* cppcheck-suppress misra-c2012-18.4 ; operator to pointer is needed */
        ((hscheduler->taskPtr) + hscheduler->tasksCount)->taskFunc = TaskPtr;   /* cppcheck-suppress misra-c2012-18.4 ; operator to pointer is needed */
        ((hscheduler->taskPtr) + hscheduler->tasksCount)->elapsed = FALSE;      /* cppcheck-suppress misra-c2012-18.4 ; operator to pointer is needed */
        hscheduler->tasksCount++;
        Task_ID = hscheduler->tasksCount + 1u;
    }
    else
    {
        Task_ID = 0;
    }

    return Task_ID;
}

/**
* @brief   **This function stops a task of the scheduler**
*
* this function sets the flag of hscheduler to 1 to stop the task from running
* 
* @param   hscheduler[in] Pointer to a Scheduler_HandleTypeDef structure 
* @param   task[in]   task to be stopped 
* @retval  Task_status will be TRUE if task was stopped otherwise it is FALSE . 
*/
uint8_t HIL_SCHEDULER_StopTask( Scheduler_HandleTypeDef *hscheduler, uint32_t task )
{
    assert_error( (hscheduler->taskPtr != NULL), SCHEDULER_ERROR ); /* cppcheck-suppress misra-c2012-11.8 ; function cannot be modify */
    assert_error( (hscheduler->tasks != FALSE), SCHEDULER_ERROR );  /* cppcheck-suppress misra-c2012-11.8 ; function cannot be modify */
    assert_error( (hscheduler->tick != FALSE), SCHEDULER_ERROR );   /* cppcheck-suppress misra-c2012-11.8 ; function cannot be modify */

    uint8_t Task_status;

    if(((hscheduler->taskPtr)+(task-1u))->stopflag == FALSE)    /* cppcheck-suppress misra-c2012-18.4 ; operator to pointer is needed */
    {
        ((hscheduler->taskPtr)+(task-1u))->stopflag = TRUE;     /* cppcheck-suppress misra-c2012-18.4 ; operator to pointer is needed */
        Task_status = TRUE;
    }
    else 
    {
        Task_status = FALSE;
    }

    return Task_status;
}

/**
* @brief   **This function starts a task of the scheduler**
*
*   this function sets the flag of hscheduler to 0 to start the task from running
* 
* @param   hscheduler[in] Pointer to a Scheduler_HandleTypeDef structure 
* @param   task[in] task to be start
* @retval  Task_status will be TRUE if task was start otherwise it is FALSE . 
*/
uint8_t HIL_SCHEDULER_StartTask( Scheduler_HandleTypeDef *hscheduler, uint32_t task )
{
    assert_error( (hscheduler->taskPtr != NULL), SCHEDULER_ERROR ); /* cppcheck-suppress misra-c2012-11.8 ; function cannot be modify */
    assert_error( (hscheduler->tasks != FALSE), SCHEDULER_ERROR );  /* cppcheck-suppress misra-c2012-11.8 ; function cannot be modify */
    assert_error( (hscheduler->tick != FALSE), SCHEDULER_ERROR );   /* cppcheck-suppress misra-c2012-11.8 ; function cannot be modify */

    uint8_t Task_status;

    if(((hscheduler->taskPtr)+(task-1u))->stopflag == TRUE)     /* cppcheck-suppress misra-c2012-18.4 ; operator to pointer is needed */
    {
        ((hscheduler->taskPtr)+(task-1u))->stopflag = FALSE;    /* cppcheck-suppress misra-c2012-18.4 ; operator to pointer is needed */
        Task_status = TRUE;
    }
    else 
    {   
        Task_status = FALSE;
    }
    return Task_status;
}

/**
* @brief   **This function changes te period of a task**
*
*   the function sets the period of the task if the new periodicity
*   is a multiple of the tick value.
* 
* @param   hscheduler[in] Pointer to a Scheduler_HandleTypeDef structure 
* @param   task[in] Task to be changed 
* @param   period[in] New period value 
* @retval  Task_status will be TRUE if task was start otherwise it is FALSE . 
*/
uint8_t HIL_SCHEDULER_PeriodTask( Scheduler_HandleTypeDef *hscheduler, uint32_t task, uint32_t period )
{
    uint8_t Task_status;

    if((period > hscheduler->tick) && ( (period % (hscheduler->tick)) == FALSE ))
    {
        ((hscheduler->taskPtr)+(task-1u))->period = period; /* cppcheck-suppress misra-c2012-18.4 ; operator to pointer is needed */
        Task_status = TRUE;
    }
    else 
    {
        Task_status = FALSE;
    }

    return Task_status;
}

/**
* @brief   **This stars the scheduler**
*
*   This is the function in charge of running the task init functions one single time and actual
*   run each registered task according to their periodicity in an infinite loop, 
*   the function will never return at least something wrong happens, but this will be considered a malfunction.
*   the function will be checking each tick if the period of a task has passed to be executed
* 
* @param   hscheduler[in] Pointer to a Scheduler_HandleTypeDef structure 
*/
void HIL_SCHEDULER_Start( Scheduler_HandleTypeDef *hscheduler )
{
    for (uint32_t i = 0; i < hscheduler->tasks; i++)
    {
        ((hscheduler->taskPtr)+i)->initFunc();      /* cppcheck-suppress misra-c2012-18.4 ; operator to pointer is needed */
    }

    for (;;)
    {
        if( HAL_GetTick() - (hscheduler->elapsed_time ) >= hscheduler->tick )
        {
            hscheduler->elapsed_time = HAL_GetTick();/*volvemos a obtener la cuenta actual*/
            for (uint32_t i = 0; i < hscheduler->tasks;i++)
            {
                if( (HAL_GetTick() - ((hscheduler->taskPtr)+i)->elapsed ) >= ((hscheduler->taskPtr)+i)->period)     /* cppcheck-suppress misra-c2012-18.4 ; operator to pointer is needed */
                {
                    ((hscheduler->taskPtr)+i)->elapsed = HAL_GetTick();     /* cppcheck-suppress misra-c2012-18.4 ; operator to pointer is needed */
                    ((hscheduler->taskPtr)+i)->taskFunc();                  /* cppcheck-suppress misra-c2012-18.4 ; operator to pointer is needed */
                }
            }
        }
    }
}