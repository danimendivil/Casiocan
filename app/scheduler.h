/**
 * @file    <scheduler.h>
 * @brief   **Header file for scheduler**
 *
 * This fila contains global variables, structures or defines for
 * the scheduler function.
 */
#ifndef SCHEDULER_H__
#define SCHEDULER_H__

  #include "app_bsp.h"

  /** 
  * @defgroup <Task_TypeDef> parameters for tasks
  @{ */
  typedef struct _task
  {
    uint32_t period;          /*How often the task shopud run in ms*/
    uint32_t elapsed;         /*the cuurent elapsed time*/
    void (*initFunc)(void);   /*pointer to init task function*/
    void (*taskFunc)(void);   /*pointer to task function*/
    uint8_t stopflag;
    //Add more elements if required
  }Task_TypeDef;
  /**
    @} */

  /** 
  * @defgroup <Scheduler_HandleTypeDef> parameters for the scheduler
  @{ */
  typedef struct _scheduler
  {
    uint32_t tasks;         /*number of task to handle*/
    uint32_t tick;          /*the time base in ms*/
    uint32_t tasksCount;    /*internal task counter*/
    uint32_t elapsed_time;    /*current elapsed time*/
    Task_TypeDef *taskPtr;  /*Pointer to buffer for the TCB tasks*/
    //Add more elements if required
  }Scheduler_HandleTypeDef;
  
#endif