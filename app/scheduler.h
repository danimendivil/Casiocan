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
  * @defgroup Task_TypeDef parameters for tasks
  @{ */
  typedef struct _task
  {
    uint32_t period;          /*!<How often the task shopud run in ms*/
    uint32_t elapsed;         /*!<the cuurent elapsed time*/
    void (*initFunc)(void);   /*!<pointer to init task function*/
    void (*taskFunc)(void);   /*!<pointer to task function*/
    uint8_t stopflag;         /*!<flag to stop task function*/
    uint8_t tick_count;       /*!<tick count for functional safety*/

    //Add more elements if required
  }Task_TypeDef;
  
  /** 
  * @defgroup _Timer_TypeDef parameters for software timers
  @{ */
  typedef struct _Timer_TypeDef
  {
      uint32_t Timeout;           /*!< timer timeout to decrement and reload when the timer is re-started */
      uint32_t Count;             /*!< actual timer decrement count */
      uint32_t StartFlag;         /*!< flag to start timer count */
      void(*callbackPtr)(void);   /*!< pointer to callback function function */
  } Timer_TypeDef;
  
  /** 
  * @defgroup Scheduler_HandleTypeDef parameters for the scheduler
  @{ */
  typedef struct _scheduler
  {
    uint32_t tasks;         /*!<number of task to handle*/
    uint32_t tick;          /*!<the time base in ms*/
    uint32_t tasksCount;    /*!<internal task counter*/
    uint32_t elapsed_time;  /*!<current elapsed time*/
    Task_TypeDef *taskPtr;  /*!<Pointer to buffer for the TCB tasks*/
    uint32_t timers;        /*!<number of software timer to use*/
    Timer_TypeDef *timerPtr /*!<Pointer to buffer timer array*/
    //Add more elements if required
  }Scheduler_HandleTypeDef;

  void HIL_SCHEDULER_Init( Scheduler_HandleTypeDef *hscheduler );
  uint8_t HIL_SCHEDULER_RegisterTask( Scheduler_HandleTypeDef *hscheduler, void (*InitPtr)(void), void (*TaskPtr)(void), uint32_t Period );
  uint8_t HIL_SCHEDULER_StopTask( Scheduler_HandleTypeDef *hscheduler, uint32_t task );
  uint8_t HIL_SCHEDULER_StartTask( Scheduler_HandleTypeDef *hscheduler, uint32_t task );
  uint8_t HIL_SCHEDULER_PeriodTask( Scheduler_HandleTypeDef *hscheduler, uint32_t task, uint32_t period );
  void HIL_SCHEDULER_Start( Scheduler_HandleTypeDef *hscheduler );     

#endif