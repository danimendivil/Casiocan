/**
* @file    scheduler.c
* @brief   **scheduler functions**
*
*    This is a reusable driver for a scheduler, this files contains all the functions
*    implementation declared on the scheduler.h file       
*/

#include "scheduler.h"
/** 
* @defgroup TIM conf values.
@{ */
#define    MAX_VALUE           0xFFFF   /*!< State for changing the time of the clock*/
#define    PREESCALER_VALUE    32000   /*!< State for changing the date of the clock*/
#define    TEN_PERCENT         10u   /*!< State for changing the date of the clock*/
/**
@} */

/** 
* @defgroup NUM DEFINES.
@{ */
#define    ZERO           0u   /*!< Define for number 0*/  
#define    ONE           1u    /*!< Define for number 1*/       
/**
@} */


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
    hscheduler->tasksCount = ZERO;
    hscheduler->timerCount = ZERO;
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
    uint8_t Task_ID = FALSE;
    if((Period > hscheduler->tick) && ((Period % (hscheduler->tick)) == FALSE) )
    {
        ((hscheduler->taskPtr) + hscheduler->tasksCount)->period = Period;      /* cppcheck-suppress misra-c2012-18.4 ; operator to pointer is needed */
        ((hscheduler->taskPtr) + hscheduler->tasksCount)->initFunc = InitPtr;   /* cppcheck-suppress misra-c2012-18.4 ; operator to pointer is needed */
        ((hscheduler->taskPtr) + hscheduler->tasksCount)->taskFunc = TaskPtr;   /* cppcheck-suppress misra-c2012-18.4 ; operator to pointer is needed */
        ((hscheduler->taskPtr) + hscheduler->tasksCount)->elapsed = FALSE;      /* cppcheck-suppress misra-c2012-18.4 ; operator to pointer is needed */
        ((hscheduler->taskPtr) + hscheduler->tasksCount)->stopflag = FALSE;     /* cppcheck-suppress misra-c2012-18.4 ; operator to pointer is needed */
        hscheduler->tasksCount++;
        Task_ID = hscheduler->tasksCount + ONE;
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

    uint8_t Task_status = FALSE;

    if(((hscheduler->taskPtr)+(task-ONE))->stopflag == FALSE)    /* cppcheck-suppress misra-c2012-18.4 ; operator to pointer is needed */
    {
        ((hscheduler->taskPtr)+(task-ONE))->stopflag = TRUE;     /* cppcheck-suppress misra-c2012-18.4 ; operator to pointer is needed */
        Task_status = TRUE;
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

    uint8_t Task_status = FALSE;

    if(((hscheduler->taskPtr)+(task-ONE))->stopflag == TRUE)     /* cppcheck-suppress misra-c2012-18.4 ; operator to pointer is needed */
    {
        ((hscheduler->taskPtr)+(task-ONE))->stopflag = FALSE;    /* cppcheck-suppress misra-c2012-18.4 ; operator to pointer is needed */
        Task_status = TRUE;
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
    assert_error( (hscheduler->taskPtr != NULL), SCHEDULER_ERROR ); /* cppcheck-suppress misra-c2012-11.8 ; function cannot be modify */
    assert_error( (hscheduler->tasks != FALSE), SCHEDULER_ERROR );  /* cppcheck-suppress misra-c2012-11.8 ; function cannot be modify */
    assert_error( (hscheduler->tick != FALSE), SCHEDULER_ERROR );   /* cppcheck-suppress misra-c2012-11.8 ; function cannot be modify */
    uint8_t Task_status = FALSE;

    if((period > hscheduler->tick) && ( (period % (hscheduler->tick)) == FALSE ))
    {
        ((hscheduler->taskPtr)+(task-ONE))->period = period; /* cppcheck-suppress misra-c2012-18.4 ; operator to pointer is needed */
        Task_status = TRUE;
    }

    return Task_status;
}

/**
* @brief   **scheduler error function**
*
*   this function creates an array initialize with error values,
*   the positions of the return_error are given by the register of the functions on the main function 
*   and this function is supoused to be used as a functional safety measure 
*
* @param   error[in] task that cause the error   
* @retval  return_error will be the value of the error on the enum for functional safety . 
*/
static uint32_t shceduler_error(uint32_t error)
{
    uint32_t return_error[5]= {SHCEDULER_WATCHDOG_ERROR, SHCEDULER_SERIAL_ERROR, SHCEDULER_CLOCK_ERROR, SHCEDULER_DISPLAY_ERROR, SHCEDULER_HEARTH_ERROR};
    return return_error[error];
}

/**
* @brief   **This stars the scheduler**
*
*   This is the function in charge of running the task init functions one single time and actual
*   run each registered task according to their periodicity in an infinite loop, 
*   the function will never return at least something wrong happens, but this will be considered a malfunction.
*   the function will be checking each tick if the period of a task has passed to be executed
*   the task also has a functional safety measure where it checks with the basic timer 6
*   if the task has not been called in more time than the period plus 10%
*   we want the preescaler on the miliseconds frequency so the calculations are:
*   Prescaler = (Clock frequency / Desired frequency) 
*   Prescaler = (32,000,000 / 1,000) = 32000
* 
* @param   hscheduler[in] Pointer to a Scheduler_HandleTypeDef structure 
*/
void HIL_SCHEDULER_Start( Scheduler_HandleTypeDef *hscheduler )
{
    assert_error( (hscheduler->taskPtr != NULL), SCHEDULER_ERROR ); /* cppcheck-suppress misra-c2012-11.8 ; function cannot be modify */
    assert_error( (hscheduler->tasks != FALSE), SCHEDULER_ERROR );  /* cppcheck-suppress misra-c2012-11.8 ; function cannot be modify */
    assert_error( (hscheduler->tick != FALSE), SCHEDULER_ERROR );   /* cppcheck-suppress misra-c2012-11.8 ; function cannot be modify */
    
    static TIM_HandleTypeDef TIM6_Handler = {0};
    TIM6_Handler.Instance = TIM6;                          /*Timer TIM to configure*/
    TIM6_Handler.Init.Prescaler = PREESCALER_VALUE;                    /*preescaler Tfrec / Prescaler*/
    TIM6_Handler.Init.CounterMode = TIM_COUNTERMODE_UP;    /*count from 0 to overflow value*/
    TIM6_Handler.Init.Period =  MAX_VALUE;                      /*Max value*/
    /*use the previous parameters to set configuration on TIM6*/
    HAL_TIM_Base_Init( &TIM6_Handler );
    HAL_TIM_Base_Start_IT( &TIM6_Handler );

    uint32_t i;
    uint32_t time;
    uint32_t time_diff;
    uint32_t period_plus_10p;

    for (i = ZERO; i < hscheduler->tasks; i++)
    {
        ((hscheduler->taskPtr)+i)->initFunc();      /* cppcheck-suppress misra-c2012-18.4 ; operator to pointer is needed */
        ((hscheduler->taskPtr)+i)-> tick_count = __HAL_TIM_GET_COUNTER(&TIM6_Handler);  /* cppcheck-suppress misra-c2012-18.4 ; operator to pointer is needed */
    }

    for (;;)
    {
        if( HAL_GetTick() - (hscheduler->elapsed_time ) >= hscheduler->tick )
        {
            hscheduler->elapsed_time = HAL_GetTick();
            for (i = ZERO; i < hscheduler->tasks;i++)
            {
                if( (HAL_GetTick() - ((hscheduler->taskPtr)+i)->elapsed ) >= ((hscheduler->taskPtr)+i)->period)     /* cppcheck-suppress misra-c2012-18.4 ; operator to pointer is needed */
                {
                    time = __HAL_TIM_GET_COUNTER(&TIM6_Handler);
                    time_diff = time - ((hscheduler->taskPtr)+i)-> tick_count;                                              /* cppcheck-suppress misra-c2012-18.4 ; operator to pointer is needed */
                    period_plus_10p = ((hscheduler->taskPtr)+i)->period + (((hscheduler->taskPtr)+i)->period /TEN_PERCENT); /* cppcheck-suppress misra-c2012-18.4 ; operator to pointer is needed */
                    assert_error( time_diff <= period_plus_10p, shceduler_error(i));                                        /* cppcheck-suppress misra-c2012-11.8 ; function cannot be modify */
                    ((hscheduler->taskPtr)+i)-> tick_count = __HAL_TIM_GET_COUNTER(&TIM6_Handler);                          /* cppcheck-suppress misra-c2012-18.4 ; operator to pointer is needed */
                    ((hscheduler->taskPtr)+i)->elapsed = HAL_GetTick();                                                     /* cppcheck-suppress misra-c2012-18.4 ; operator to pointer is needed */
                    
                    if(((hscheduler->taskPtr) + i)->stopflag == FALSE)                                                      /* cppcheck-suppress misra-c2012-18.4 ; operator to pointer is needed */
                    {
                        ((hscheduler->taskPtr)+i)->taskFunc();                                                              /* cppcheck-suppress misra-c2012-18.4 ; operator to pointer is needed */
                    }
                                                                                      
                }
            }
            for(i = ZERO; i < hscheduler->timers;i++)
            {
                if(((hscheduler->timerPtr)+i)->Count == ZERO )                                                               /* cppcheck-suppress misra-c2012-18.4 ; operator to pointer is needed */
                {
                    ((hscheduler->timerPtr)+i)->Count = ((hscheduler->timerPtr)+i)->Timeout;                                 /* cppcheck-suppress misra-c2012-18.4 ; operator to pointer is needed */
                }
                ((hscheduler->timerPtr)+i)->Count = ((hscheduler->timerPtr)+i)->Count - hscheduler->tick;                    /* cppcheck-suppress misra-c2012-18.4 ; operator to pointer is needed */
                if(((hscheduler->timerPtr)+i)->Count == ZERO )                                                               /* cppcheck-suppress misra-c2012-18.4 ; operator to pointer is needed */
                {
                    if(((hscheduler->timerPtr)+i)->callbackPtr != NULL)                                                      /* cppcheck-suppress misra-c2012-18.4 ; operator to pointer is needed */
                    {
                        ((hscheduler->timerPtr)+i)->callbackPtr();                                                           /* cppcheck-suppress misra-c2012-18.4 ; operator to pointer is needed */
                    }
                }
            }
        }
    }
}

/**
* @brief   **This function register a timer for the scheduler**
*
*   this function sets the hscheduler timer with the address of the callback function,
*   The timeout value, and puts the startflag on FALSE, once a function is registred 
*   it will return an ID value wich will be necesary for the usage of following functions
*   this timer also accepts NULL callbacks functions
*
* @param   hscheduler[in] Pointer to a Scheduler_HandleTypeDef structure 
* @param   Timeout[in] Timeout value of the software timer 
* @param   CallbackPtr[in] Pointer to a callback function 
* @retval  Timer_ID Is number from 1 to n task registered if the operation was a success, otherwise, it will return zero. 
*/
uint8_t HIL_SCHEDULER_RegisterTimer( Scheduler_HandleTypeDef *hscheduler, uint32_t Timeout, void (*CallbackPtr)(void) )
{
    assert_error( (Timeout > FALSE), SCHEDULER_ERROR ); /* cppcheck-suppress misra-c2012-11.8 ; function cannot be modify */
    assert_error( (hscheduler->timerPtr != NULL), SCHEDULER_ERROR ); /* cppcheck-suppress misra-c2012-11.8 ; function cannot be modify */
    assert_error( (hscheduler->timers != FALSE), SCHEDULER_ERROR );  /* cppcheck-suppress misra-c2012-11.8 ; function cannot be modify */
    assert_error( (hscheduler->tick != FALSE), SCHEDULER_ERROR );   /* cppcheck-suppress misra-c2012-11.8 ; function cannot be modify */
    uint8_t Timer_ID = FALSE;
    if((Timeout > hscheduler->tick) && ((Timeout % (hscheduler->tick)) == FALSE) )
    {
        ((hscheduler->timerPtr) + hscheduler->timerCount)->Timeout = Timeout;      /* cppcheck-suppress misra-c2012-18.4 ; operator to pointer is needed */
        ((hscheduler->timerPtr) + hscheduler->timerCount)->StartFlag = FALSE;   /* cppcheck-suppress misra-c2012-18.4 ; operator to pointer is needed */
        if(CallbackPtr != NULL)
        {
            ((hscheduler->timerPtr) + hscheduler->timerCount)->callbackPtr = CallbackPtr;   /* cppcheck-suppress misra-c2012-18.4 ; operator to pointer is needed */
        }
        Timer_ID = hscheduler->tasksCount + ONE;
    }

    return Timer_ID;

}

/**
* @brief   **This function gets the pending time of the timer**
*
*   the function first checks if the timer has been register by comparing it with the timercount
*   and then returns the count value
*
* @param   hscheduler[in] Pointer to a Scheduler_HandleTypeDef structure 
* @param   Timer[in] Timer to get the count  
* @retval  current_time Count value of the timer. 
*/
uint32_t HIL_SCHEDULER_GetTimer( Scheduler_HandleTypeDef *hscheduler, uint32_t Timer )
{
    assert_error( (hscheduler->timerPtr != NULL), SCHEDULER_ERROR ); /* cppcheck-suppress misra-c2012-11.8 ; function cannot be modify */
    assert_error( (hscheduler->timers != FALSE), SCHEDULER_ERROR );  /* cppcheck-suppress misra-c2012-11.8 ; function cannot be modify */
    assert_error( (hscheduler->tick != FALSE), SCHEDULER_ERROR );   /* cppcheck-suppress misra-c2012-11.8 ; function cannot be modify */
    uint32_t current_time = ZERO;

    if ((Timer < hscheduler->timerCount) && (Timer > ZERO))    /* cppcheck-suppress misra-c2012-18.4 ; operator to pointer is needed */
    {
        current_time = ((hscheduler->timerPtr)+(Timer-ONE))->Count;     /* cppcheck-suppress misra-c2012-18.4 ; operator to pointer is needed */
    }

    return current_time;
}

/**
* @brief   **This function reloads the timer with a new timeout**
*
*   the function first checks if the timer has been register by comparing it with the timercount,
*   checks if the timeout value is valid and then changes the timeout value and starts the timer
*
* @param   hscheduler[in] Pointer to a Scheduler_HandleTypeDef structure 
* @param   Timer[in] Timer to reload
* @param   Timeout[in] new timeout value
* @retval  Timer_Status returns if the operation was a success. 
*/
uint8_t HIL_SCHEDULER_ReloadTimer( Scheduler_HandleTypeDef *hscheduler, uint32_t Timer, uint32_t Timeout )
{
    assert_error( (hscheduler->timerPtr != NULL), SCHEDULER_ERROR ); /* cppcheck-suppress misra-c2012-11.8 ; function cannot be modify */
    assert_error( (hscheduler->timers != FALSE), SCHEDULER_ERROR );  /* cppcheck-suppress misra-c2012-11.8 ; function cannot be modify */
    assert_error( (hscheduler->tick != FALSE), SCHEDULER_ERROR );   /* cppcheck-suppress misra-c2012-11.8 ; function cannot be modify */
    uint8_t Timer_Status = FALSE;

    if((Timeout > hscheduler->tick) && ((Timeout % (hscheduler->tick)) == FALSE) )
    {
        if ((Timer < hscheduler->timerCount) && (Timer > ZERO))    /* cppcheck-suppress misra-c2012-18.4 ; operator to pointer is needed */
        {
            ((hscheduler->timerPtr)+(Timer-ONE))->Timeout = Timeout;     /* cppcheck-suppress misra-c2012-18.4 ; operator to pointer is needed */
            ((hscheduler->timerPtr)+(Timer-ONE))->StartFlag = TRUE;      /* cppcheck-suppress misra-c2012-18.4 ; operator to pointer is needed */
            ((hscheduler->timerPtr)+(Timer-ONE))->Count = Timeout;       /* cppcheck-suppress misra-c2012-18.4 ; operator to pointer is needed */
            Timer_Status = TRUE;
        }
    }
    return Timer_Status;
}

/**
* @brief   **This function starts a timer**
*
*   the function first checks if the timer has been register by comparing it with the timercount
*   and then changes the startflag to true to start the timer.
*
* @param   hscheduler[in] Pointer to a Scheduler_HandleTypeDef structure 
* @param   Timer[in] Timer to start  
* @retval  Timer_Status returns if the operation was a success.  
*/
uint8_t HIL_SCHEDULER_StartTimer( Scheduler_HandleTypeDef *hscheduler, uint32_t Timer )
{
    assert_error( (hscheduler->timerPtr != NULL), SCHEDULER_ERROR ); /* cppcheck-suppress misra-c2012-11.8 ; function cannot be modify */
    assert_error( (hscheduler->timers != FALSE), SCHEDULER_ERROR );  /* cppcheck-suppress misra-c2012-11.8 ; function cannot be modify */
    assert_error( (hscheduler->tick != FALSE), SCHEDULER_ERROR );   /* cppcheck-suppress misra-c2012-11.8 ; function cannot be modify */
    uint8_t Timer_Status = FALSE;

    if ((Timer < hscheduler->timerCount) && (Timer > ZERO))    /* cppcheck-suppress misra-c2012-18.4 ; operator to pointer is needed */
    {
        ((hscheduler->timerPtr)+(Timer-ONE))->StartFlag = TRUE;     /* cppcheck-suppress misra-c2012-18.4 ; operator to pointer is needed */
        ((hscheduler->timerPtr)+(Timer-ONE))->Count = ((hscheduler->timerPtr)+(Timer-ONE))->Timeout;     /* cppcheck-suppress misra-c2012-18.4 ; operator to pointer is needed */       
        Timer_Status = TRUE;
    }

    return Timer_Status;
}

/**
* @brief   **This function stops a timer**
*
*   the function first checks if the timer has been register by comparing it with the timercount
*   and then changes the startflag to false to stop the timer.
*
* @param   hscheduler[in] Pointer to a Scheduler_HandleTypeDef structure 
* @param   Timer[in] Timer to stop  
* @retval  Timer_Status returns if the operation was a success.  
*/
uint8_t HIL_SCHEDULER_StopTimer( Scheduler_HandleTypeDef *hscheduler, uint32_t Timer )
{
    assert_error( (hscheduler->timerPtr != NULL), SCHEDULER_ERROR ); /* cppcheck-suppress misra-c2012-11.8 ; function cannot be modify */
    assert_error( (hscheduler->timers != FALSE), SCHEDULER_ERROR );  /* cppcheck-suppress misra-c2012-11.8 ; function cannot be modify */
    assert_error( (hscheduler->tick != FALSE), SCHEDULER_ERROR );   /* cppcheck-suppress misra-c2012-11.8 ; function cannot be modify */
    uint8_t Timer_Status = FALSE;

    if ((Timer < hscheduler->timerCount) && (Timer > ZERO))    /* cppcheck-suppress misra-c2012-18.4 ; operator to pointer is needed */
    {
        ((hscheduler->timerPtr)+(Timer-ONE))->StartFlag = FALSE;     /* cppcheck-suppress misra-c2012-18.4 ; operator to pointer is needed */
        Timer_Status = TRUE;
    }

    return Timer_Status;
}