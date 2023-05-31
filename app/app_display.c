#include "app_display.h"
#include "hel_lcd.h"

/** 
  * @defgroup <Lcd state machine> this are defines for the states
  @{ */
#define    STATE_IDLE               0
#define    STATE_PRINTH_MONTH       1
#define    STATE_PRINTH_DAY         2
#define    STATE_PRINTH_YEAR        3
#define    STATE_PRINTH_WDAY        4
#define    STATE_PRINTH_HOUR        5
#define    STATE_PRINTH_MINUTES     6
#define    STATE_PRINTH_SECONDS     7
/**
  @} */

LCD_HandleTypeDef LCDHandle;
SPI_HandleTypeDef SpiHandle;
static void month(char *mon,char pos);
static void week(char *week,char pos);
void Display_Init( void )
{
    LCDHandle.SpiHandler  =   &SpiHandle;
    /*Reset pin configuration*/
    LCDHandle.RstPort     =   GPIOD;
    LCDHandle.RstPin      =   GPIO_PIN_2;
    /*LCD rs pin configuration*/
    LCDHandle.RsPort      =   GPIOD;
    LCDHandle.RsPin       =   GPIO_PIN_4;
    /*Chip select pint configuration*/
    LCDHandle.CsPort      =   GPIOD;
    LCDHandle.CsPin       =   GPIO_PIN_3;
    /*Backliht pin configuration*/
    LCDHandle.BklPort     =   GPIOB;
    LCDHandle.BklPin      =   GPIO_PIN_4;

    HEL_LCD_MspInit(&LCDHandle);

    LCDHandle.SpiHandler->Instance            = SPI1;
    LCDHandle.SpiHandler->Init.Mode           = SPI_MODE_MASTER;
    LCDHandle.SpiHandler->Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;
    LCDHandle.SpiHandler->Init.Direction      = SPI_DIRECTION_2LINES;
    LCDHandle.SpiHandler->Init.CLKPhase       = SPI_PHASE_2EDGE;
    LCDHandle.SpiHandler->Init.CLKPolarity    = SPI_POLARITY_HIGH;
    LCDHandle.SpiHandler->Init.DataSize       = SPI_DATASIZE_8BIT;
    LCDHandle.SpiHandler->Init.FirstBit       = SPI_FIRSTBIT_MSB;
    LCDHandle.SpiHandler->Init.NSS            = SPI_NSS_SOFT;
    LCDHandle.SpiHandler->Init.CRCCalculation = SPI_CRCCALCULATION_DISABLED;
    LCDHandle.SpiHandler->Init.TIMode         = SPI_TIMODE_DISABLED;
    
    HAL_GPIO_WritePin( LCDHandle.CsPort, LCDHandle.CsPin, SET );
    HAL_SPI_Init( LCDHandle.SpiHandler );

    HEL_LCD_Init(&LCDHandle );
}
char fila_1[]=" XXX,XX XXXX XX ";
char fila_2[] = "00:00:00";
uint8_t LCD_State  = STATE_IDLE;
void Display_Task( void )
{
    
    switch(LCD_State)
    {

        case STATE_IDLE:
            if( ClockMsg.msg == DISPLAY_MESSAGE)
            {
                LCD_State = STATE_PRINTH_MONTH;
                ClockMsg.msg = NO_DISPLAY_MESSAGE;
            }
        break;

        case STATE_PRINTH_MONTH:
            month(&fila_1[1],ClockMsg.tm.tm_mon);
            LCD_State = STATE_PRINTH_DAY;
        break;

        case STATE_PRINTH_DAY:
            fila_1[5] = ((ClockMsg.tm.tm_mday / 10u) + 48u);
            fila_1[6] = ((ClockMsg.tm.tm_mday % 10u) + 48u);
            LCD_State =  STATE_PRINTH_YEAR;
        break;

        case STATE_PRINTH_YEAR:
            fila_1[8]   = ( (ClockMsg.tm.tm_year_msb / 10u) + 48u);
            fila_1[9]   = ( (ClockMsg.tm.tm_year_msb % 10u) + 48u);
            fila_1[10]  = ( (ClockMsg.tm.tm_year_lsb / 10u) + 48u);
            fila_1[11]  = ( (ClockMsg.tm.tm_year_lsb % 10u) + 48u);
            LCD_State = STATE_PRINTH_WDAY;
        break;

        case STATE_PRINTH_WDAY:
            HEL_LCD_SetCursor(&LCDHandle,FIRST_ROW,0);
            week(&fila_1[13],1);
            HEL_LCD_String(&LCDHandle, fila_1);
            LCD_State = STATE_PRINTH_HOUR;
        break;
        
        case STATE_PRINTH_HOUR:
            HEL_LCD_SetCursor(&LCDHandle,SECOND_ROW,3 );
            fila_2[0] = ((ClockMsg.tm.tm_hour / 10u) + 48u);
            fila_2[1] = ((ClockMsg.tm.tm_hour % 10u) + 48u);
            LCD_State = STATE_PRINTH_MINUTES;
        break;

        case STATE_PRINTH_MINUTES:
            fila_2[3] = ((ClockMsg.tm.tm_min / 10u) + 48u);
            fila_2[4] = ((ClockMsg.tm.tm_min % 10u) + 48u);
            LCD_State = STATE_PRINTH_SECONDS;
        break;

        case STATE_PRINTH_SECONDS:
            fila_2[6] = ((ClockMsg.tm.tm_sec / 10u) + 48u);
            fila_2[7] = ((ClockMsg.tm.tm_sec % 10u) + 48u);
            HEL_LCD_String(&LCDHandle, fila_2);
            LCD_State = STATE_IDLE;
        break;

        default:
            LCD_State = STATE_IDLE;
        break;
    }
}

void month(char *mon,char pos)
{
   pos=(pos-1)*4;
   char *mes = "ENE,FEB,MAR,ABR,MAY,JUN,JUL,AGO,SEP,OCT,NOV,DIC," ;
   for(int i=0;i<4;i++)
   {
        *(mon+i)=*(mes+i+pos);
   }
}

void week(char *week,char pos)
{
   pos=(pos-1)*3;
   char *sem = "LU MA MI JU VI SA DO" ;
   for(int i=0;i<3;i++)
   {
        *(week+i)=*(sem+i+pos);
   }
}