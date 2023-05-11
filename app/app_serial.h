#ifndef APP_SERIAL_H__
#define APP_SERIAL_H__

#include "app_bsp.h"

void Serial_Init( void );
void Serial_Task( void );


#define    SERIAL_MSG_TIME     1u
#define    SERIAL_MSG_DATE     2u
#define    SERIAL_MSG_ALARM    3u

#define    GETMSG              4u
#define    FAILED              5u
#define    OK                  6u

typedef struct _APP_TmTypeDef 
{
    uint32_t tm_sec;         /* seconds,  range 0 to 59          */
    uint32_t tm_min;         /* minutes, range 0 to 59           */
    uint32_t tm_hour;        /* hours, range 0 to 23             */
    uint32_t tm_mday;        /* day of the month, range 1 to 31  */
    uint32_t tm_mon;         /* month, range 0 to 11             */
    uint32_t tm_year_msb;        /* years in rage 1900 2100          */
    uint32_t tm_year_lsb;
    uint32_t tm_wday;        /* day of the week, range 0 to 6    */
    uint32_t tm_yday;        /* day in the year, range 0 to 365  */
    uint32_t tm_isdst;       /* daylight saving time             */
}APP_TmTypeDef;

typedef struct _APP_MsgTypeDef
{
    uint8_t msg;          /*!< Store the message type to send */
    APP_TmTypeDef tm;     /*!< time and date in stdlib tm format */
}APP_MsgTypeDef;

extern FDCAN_HandleTypeDef CANHandler;
extern FDCAN_TxHeaderTypeDef CANTxHeader;
extern FDCAN_RxHeaderTypeDef CANRxHeader;
extern FDCAN_FilterTypeDef CANFilter;
extern uint8_t flag; 
extern uint8_t sizer;
extern uint8_t datar[8];  
extern uint8_t cases;
extern APP_MsgTypeDef mtm;





#endif