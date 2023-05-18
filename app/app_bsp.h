#ifndef BSP_H_
#define BSP_H_

    #include "stm32g0xx.h"
    #include <stdint.h>

    /** 
    * @defgroup <BOOL> Boolean values.
    @{ */
    #define    TRUE     1u   /*!< State for changing the time of the clock*/
    #define    FALSE    0u   /*!< State for changing the date of the clock*/
    /**
    @} */

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
    * @brief  Variable for Cases of the state machine
    */
    extern APP_MsgTypeDef td_message;

    extern FDCAN_HandleTypeDef CANHandler; /* cppcheck-suppress misra-c2012-8.4 ; this function can`t be modify */



#endif

