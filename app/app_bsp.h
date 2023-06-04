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
  * @defgroup Display Display task values.
  @{ */
  #define    DISPLAY_MESSAGE        1u   /*!< State for changing the time of the clock*/
  #define    NO_DISPLAY_MESSAGE     0u   /*!< State for changing the date of the clock*/
  /**
  @} */

  /*macro to detect erros, wehere if expr is evaluated to false is an error*/
  #define assert_error(expr, error)         ((expr) ? (void)0U : safe_state((uint8_t *)__FILE__, __LINE__, (error)))

  /*external link to fucntion that will act as the safe state*/
  extern void safe_state(uint8_t *file, uint32_t line, uint8_t error);

  extern HAL_StatusTypeDef Status;

  typedef enum _App_ErrorsCode
  {
    
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
    HARDFAULT_ERROR
          
  } App_ErrorsCode;

  /**
  * @brief   Structure for message that are recived by CAN to store time and date in stdlib tm format 
  */
  typedef struct _APP_TmTypeDef 
  {
    uint32_t tm_sec;         /*!< seconds,  range 0 to 59          */
    uint32_t tm_min;         /*!< minutes, range 0 to 59           */
    uint32_t tm_hour;        /*!< hours, range 0 to 23             */
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
  }APP_MsgTypeDef;

  /**
  * @brief  Variable for state machien messages.
  */
  extern APP_MsgTypeDef CAN_td_message;

  /**
  * @brief  Variable for clockMsg to diplay.
  */
  extern APP_MsgTypeDef ClockMsg;

  /**
  * @brief  Variable for CAN configuration
  */
  extern FDCAN_HandleTypeDef CANHandler; /* cppcheck-suppress misra-c2012-8.4 ; this function can`t be modify */
     
#endif

