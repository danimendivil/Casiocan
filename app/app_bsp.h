#ifndef BSP_H_
#define BSP_H_

  #include "stm32g0xx.h"
  #include <stdint.h>
  
  /** 
  * @defgroup BOOL Boolean values.
  @{ */
  #define    TRUE     1u   /*!< State for changing the time of the clock*/
  #define    FALSE    0u   /*!< State for changing the date of the clock*/
  /**
  @} */

  /** 
  * @defgroup Alarm Values.
  @{ */
  #define    ALARM_ON        0u   /*!< An alarm is going to happen*/
  #define    ALARM_OFF       1u   /*!< Alarm is off*/
  #define    ALARM_ACTIVE    2u   /*!< An alarm is happening*/
  /**
  @} */

  /** 
  * @defgroup Display Display task values.
  @{ */
  #define    DISPLAY_MESSAGE        1u   /*!< State for changing the time of the clock*/
  /**
  @} */

  /*macro to detect erros, wehere if expr is evaluated to false is an error*/
  #define assert_error(expr, error)         ((expr) ? (void)0U : safe_state((uint8_t *)__FILE__, __LINE__, (error)))

  /*external link to fucntion that will act as the safe state*/
  extern void safe_state(uint8_t *file, uint32_t line, uint8_t error);

  /**
 * @brief  Variable for functional safety
 */
  extern HAL_StatusTypeDef Status;

  /**
  * @brief Enum for Errors.
  *
  * This enumeration represents the various types of errors that can be trigger
  * during execution
  */
  typedef enum _App_ErrorsCode
  { /* cppcheck-suppress misra-c2012-2.4 ; enum is used on functional safety */
    FDCAN_CONFIG_ERROR = 1u,
    FDCAN_CONFIG_FILTER_ERROR,
    FDCAN_CALLBACK_ERROR,
    FDCAN_CONFIG_GLOBAL_FILTER_ERROR,
    FDCAN_START_ERROR,
    FDCAN_ACTIVATE_NOTIFICATION_ERROR,
    FDCAN_ADDMESSAGE_ERROR,
    FDCAN_GETMESSAGE_ERROR,
    RTC_INIT_ERROR,
    RTC_SETTIME_ERROR,
    RTC_SETDATE_ERROR,
    RTC_SET_ALARM_ERROR,
    RTC_SDESACTIVATE_ALARM_ERROR,
    RTC_GET_TIME_ERROR,
    RTC_GET_DATE_ERROR,
    SPI_COMMAND_ERROR,
    SPI_INIT_ERROR,
    SPI_SET_CURSOR_ERROR,
    SPI_STRING_ERROR,
    SPI_CALLBACK_ERROR,
    WWDG_INIT_ERROR,
    WWDG_REFRESH_ERROR,
    WWDG_EARLY_WAKEUP_ERROR,
    PWREX_CONTROL_VOLTAGE_ERROR,
    RCC_OSC_CONF_ERROR,
    RCC_CLOCK_CONF_ERROR,
    RCCEX_PRIPH_CLK_CONF_ERROR,
    HARDFAULT_ERROR,
    ECC_ONE_ERROR,
    ECC_TWO_ERROR,
    QUEUE_PAR_ERROR,
    SCHEDULER_ERROR,
    SCHEDULER_REGISTER_ERROR,
    SHCEDULER_WATCHDOG_ERROR,
    SHCEDULER_SERIAL_ERROR,
    SHCEDULER_CLOCK_ERROR,
    SHCEDULER_DISPLAY_ERROR,
    SHCEDULER_HEARTH_ERROR
  } App_ErrorsCode;   /* cppcheck-suppress misra-c2012-2.3 ; enum is used on functional safety */

  /**
  * @brief   Structure for message that are recived by CAN to store time and date in stdlib tm format 
  */
  typedef struct _APP_TmTypeDef 
  {
    uint32_t tm_sec;         /*!< seconds,  range 0 to 59          */
    uint32_t tm_min;         /*!< minutes, range 0 to 59           */
    uint32_t tm_min_alarm;   /*!< minutes, range 0 to 59           */
    uint32_t tm_hour;        /*!< hours, range 0 to 23             */
    uint32_t tm_hour_alarm;        /*!< hours, range 0 to 23             */
    uint32_t tm_mday;        /*!< day of the month, range 1 to 31  */
    uint32_t tm_mon;         /*!< month, range 0 to 11             */
    uint32_t tm_year_msb;    /*!< year most significant bits in range 1900 2100       */
    uint32_t tm_year_lsb;    /*!< year least significant bits in range 1900 2100      */
    uint32_t tm_wday;        /*!< day of the week, range 0 to 6    */
    uint32_t tm_yday;        /*!< day in the year, range 0 to 365  */
    uint32_t tm_isdst;       /*!< daylight saving time             */
  }APP_TmTypeDef;

  /**
  * @brief   structure with APP_TmTypeDef and a message 
  */
  typedef struct _APP_MsgTypeDef
  {
    uint8_t msg;          /*!< Store the message type to send */
    APP_TmTypeDef tm;     /*!< time and date in stdlib tm format */
    uint8_t S_alarm;
    uint8_t F_alarm;
  }APP_MsgTypeDef;

  /**
  * @brief  Variable for CAN configuration
  */
  extern FDCAN_HandleTypeDef CANHandler; /* cppcheck-suppress misra-c2012-8.4 ; this function can`t be modify */

  /**
  * @brief  Variable for rtc configuration
  */
  extern RTC_HandleTypeDef hrtc;

  /**
  * @brief  Variable for button state
  */
  extern uint8_t button;
     
#endif

