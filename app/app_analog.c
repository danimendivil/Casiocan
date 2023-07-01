#include "app_analog.h"

/** 
  * @defgroup DMA values configurations
  @{ */
#define ARRAY_LENGHT      3u    /*!< Lenght of the array to stored with dma*/
/**
  @} */

/** 
  * @defgroup PWM configuration values
  @{ */
#define PREESCALER      320    /*!< Preescaler to get 1000hz*/
#define PERIOD          100    /*!< Period to get 1000 hz*/
/**
  @} */

/** 
  * @defgroup POT configuration values
  @{ */ 
#define MAX_CONTRAST          15u     /*!< Max value accepted by the lcd contrast command*/
#define MAX_INTENSITY         100u    /*!< Max value of the backlight pwm*/
#define MAX_POT_VALUE         255u    /*!< Max value of the pot*/
/**
  @} */

/** 
  * @defgroup TIM_ADC configuration values
  @{ */
#define PREESCALER_ADC_TIM      10000      /*!< Preescaler to get 50ms*/
#define PERIOD_ADC_TIM          64         /*!< Period to get 50ms*/
/**
  @} */

/**
  * @defgroup Numbers defines
  * @{
  */
#define CERO        0u    /*!< Value for counter: 0 */
#define ONE         1u    /*!< Value for counter: 1 */
#define TWO         2u    /*!< Value for counter: 2 */
#define THREE       3u    /*!< Value for counter: 3 */
#define FOUR        4u    /*!< Value for counter: 4 */
/**
  * @}
  */

/**
 * @brief  Variable for tim used by the pwm
 */
static TIM_HandleTypeDef TimHandle2;            /*TIM initial structure*/

/**
 * @brief  Array were data of the DMA will be stored
 */
static uint32_t AdcData[ARRAY_LENGHT];

/**
 * @brief  Variable DMA configuration
 */
DMA_HandleTypeDef DmaHandler;           /*dma handler estructure*/

/**
 * @brief  Variable for ADD configuration
 */
ADC_HandleTypeDef  AdcHandler;          /*adc handler estructure*/


static uint8_t Analogs_GetContrast( void );
static uint8_t Analogs_GetIntensity( void );

/**
* @brief   **This function intiates the functions used by the analog file**
*
*  In this function first we initialize the TIM3 wich will be the tim in control of the pwm
*  for the lcd backlight pin to adjust the intensity level, this pwm is set to 1khz: 
*  pwm_frequency=32mhz/320/100=1000hz.
*  then we initialized the DMA to be linked with the ADC1, then we initialized a tim 
*  so that the timer will automatically trigger the ADC conversion at each interval,
*  without any CPU intervention, this timer is every 20ms
*  tim_frequency=32mhz/64/10000=50ms.
*  then we confidgure the adc parameters, the resolution will be of 8 bits, it is 
*  configure to be triggered by the tim and to call the dma to store the data.
*  lastly we configure the channels to be read by the ADC wich are 2 pots
*  and the internal temperature sensor.
* 
*/
void Analogs_Init( void )
{
  
  static ADC_ChannelConfTypeDef  sChanConfig;    /*adc channel configuration structure*/
  static TIM_HandleTypeDef TimHandler;            /*TIM initial structure*/
  static TIM_MasterConfigTypeDef sMasterConfig;  /*trigger config structure*/
  static TIM_OC_InitTypeDef sConfig;       /*estructura para controlar canal de PWM*/
  
  
  HAL_Init();    

  __HAL_RCC_DMA1_CLK_ENABLE();    
  __TIM4_CLK_ENABLE(); 

  TimHandle2.Instance = TIM3;
  TimHandle2.Init.Prescaler     = PREESCALER;
  TimHandle2.Init.Period        = PERIOD;
  TimHandle2.Init.CounterMode   = TIM_COUNTERMODE_UP;
  HAL_TIM_PWM_Init(&TimHandle2);
  
  sConfig.OCMode     = TIM_OCMODE_PWM1;
  sConfig.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfig.OCFastMode = TIM_OCFAST_DISABLE;

  HAL_TIM_PWM_ConfigChannel( &TimHandle2, &sConfig, TIM_CHANNEL_1 );
  HAL_TIM_PWM_Start( &TimHandle2, TIM_CHANNEL_1 );

  DmaHandler.Instance                  = DMA1_Channel1;       /*DMA1 channel 1 only*/
  DmaHandler.Init.Request              = DMA_REQUEST_ADC1;    /*request from the ADC*/
  DmaHandler.Init.Direction            = DMA_PERIPH_TO_MEMORY;    /*trnasfer data from peripheral to memory*/
  DmaHandler.Init.PeriphInc            = DMA_PINC_DISABLE;        /*do not increment peripheral address*/
  DmaHandler.Init.MemInc               = DMA_MINC_ENABLE;         /*increment memory address*/
  DmaHandler.Init.PeriphDataAlignment  = DMA_PDATAALIGN_WORD; /*4 bytes transactions*/
  DmaHandler.Init.MemDataAlignment     = DMA_MDATAALIGN_WORD; /*4 bytes transactions*/
  DmaHandler.Init.Mode                 = DMA_CIRCULAR;        /*overwrite data*/
  DmaHandler.Init.Priority             = DMA_PRIORITY_HIGH;   /*high priority channel*/
  HAL_DMA_Init( &DmaHandler );

  /*set a link between ADC and DMA handlers*/
  __HAL_LINKDMA( &AdcHandler, DMA_Handle, DmaHandler );
  
  /*configure timer 4 to generate a 1 seg (16MHz) timebase*/
  TimHandler.Instance               = TIM4;
  TimHandler.Init.Period            = PERIOD_ADC_TIM;        
  TimHandler.Init.Prescaler         = PREESCALER_ADC_TIM;
  TimHandler.Init.ClockDivision     = FALSE;
  TimHandler.Init.CounterMode       = TIM_COUNTERMODE_UP;
  TimHandler.Init.RepetitionCounter = FALSE;
  HAL_TIM_Base_Init( &TimHandler );
  
  /* Config timer to generate an internal trigger signal */
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
  sMasterConfig.MasterSlaveMode     = TIM_MASTERSLAVEMODE_DISABLE;
  HAL_TIMEx_MasterConfigSynchronization( &TimHandler, &sMasterConfig );

  /*enable DMA interrupts*/
  HAL_NVIC_SetPriority( DMA1_Channel1_IRQn, TWO, CERO );
  HAL_NVIC_EnableIRQ( DMA1_Channel1_IRQn );

  AdcHandler.Instance                   = ADC1;
  AdcHandler.Init.ClockPrescaler        = ADC_CLOCK_SYNC_PCLK_DIV4;   /*APB clock divided by two*/
  AdcHandler.Init.Resolution            = ADC_RESOLUTION8b;           /*8 bit resolution with a Tconv of 8.5*/
  AdcHandler.Init.ScanConvMode          = ADC_SCAN_SEQ_FIXED;         /*scan adc channels from 0 to 16 in that order*/
  AdcHandler.Init.DataAlign             = ADC_DATAALIGN_RIGHT;        /*data converter is right alightned*/
  AdcHandler.Init.SamplingTimeCommon1   = ADC_SAMPLETIME_39CYCLES_5;    /*sampling time of 1.5*/  
  AdcHandler.Init.ExternalTrigConv      = ADC_EXTERNALTRIG_T4_TRGO;   /*set the timer TIM3 to trigger the ADC*/
  AdcHandler.Init.ExternalTrigConvEdge  = ADC_EXTERNALTRIGCONVEDGE_RISING;  /*only on rising edges*/
  AdcHandler.Init.DMAContinuousRequests = ENABLE;
  AdcHandler.Init.EOCSelection          = ADC_EOC_SEQ_CONV;        /*ISR at the end of one channel conversion*/
  AdcHandler.Init.Overrun               = ADC_OVR_DATA_OVERWRITTEN;   /*data will be overwriten in case is not read it*/
  AdcHandler.Init.OversamplingMode      = FOUR;   
  HAL_ADC_Init( &AdcHandler );
  
  sChanConfig.Channel = ADC_CHANNEL_0;
  sChanConfig.Rank = ADC_RANK_CHANNEL_NUMBER;
  sChanConfig.SamplingTime = ADC_SAMPLINGTIME_COMMON_1;
  HAL_ADC_ConfigChannel( &AdcHandler, &sChanConfig );


  sChanConfig.Channel = ADC_CHANNEL_1;
  sChanConfig.Rank = ADC_RANK_CHANNEL_NUMBER;
  sChanConfig.SamplingTime = ADC_SAMPLINGTIME_COMMON_1;
  HAL_ADC_ConfigChannel( &AdcHandler, &sChanConfig );
  
  sChanConfig.Channel = ADC_CHANNEL_TEMPSENSOR;
  sChanConfig.Rank = ADC_RANK_CHANNEL_NUMBER;
  sChanConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
  HAL_ADC_ConfigChannel( &AdcHandler, &sChanConfig );

  
  HAL_ADCEx_Calibration_Start( &AdcHandler );

  HAL_ADC_Start_DMA( &AdcHandler, &AdcData[CERO], THREE );

  HAL_TIM_Base_Start( &TimHandler );
}

/**
* @brief   **This returns the temperature**
*
*  this function returns the temperature given by the internal sensor
*  this uses the hal function __HAL_ADC_CALC_TEMPERATURE this takes as parameter
*  the vref of the sensor in wich we will use the define VDD_VALUE that is equal to
*  3300 note that this value refers to milivolts, the value of the temperature
*  that is stored by the dma in the position 2 of Adcdata array
*  
* @retval  temperature temperature of the sensor on C
*/
int8_t Analogs_GetTemperature( void )
{
  int8_t temperature;

  temperature = __HAL_ADC_CALC_TEMPERATURE( VDD_VALUE, AdcData[TWO], ADC_RESOLUTION8b);

  return temperature;
}

/**
* @brief   **This returns the contrast of the lcd**
*
*  this function returns the contrast of the lcd, this is given
*  by the pot value stored on AdcData[1], and we do a convertion
*  so that the value will be betwen 0 and 15;
*  
* @retval  contrast contrast of the sensor
*/
uint8_t Analogs_GetContrast( void )
{
  uint8_t contrast;

  contrast = (AdcData[1]*MAX_CONTRAST)/MAX_POT_VALUE;

  return contrast;
}

/**
* @brief   **This returns the intensity of the lcd**
*
*  this function returns the intensity of the lcd, this is given
*  by the pot value stored on AdcData[0], and we do a convertion
*  so that the value will be betwen 0 and 100;
*  
* @retval  intensity intensity of the sensor
*/
uint8_t Analogs_GetIntensity( void )
{
  uint8_t intensity;

  intensity = (AdcData[CERO]*MAX_INTENSITY)/MAX_POT_VALUE;

  return intensity;
}

/**
* @brief   **This function applied the of intensity and contrast to the lcd**
*
*  the function first checks if the value read by the pot is different to the one
*  that was previously applied, in both cases if its different then it applies the value
*  in the case of the intensity level this is applied by changing the pwm period of the 
*  lcd backlight.
*  in the case of the contrast the value changes by calling the function HEL_LCD_Contrast
*  wich sends a command to the lcd to change the contrast, the second parameter is the contrast value
*  we use the previous fucntion to get this values.
*  
*/
void Display_LcdTask( void )
{
  static uint8_t intensity_level   = FALSE;
  static uint8_t contrast_level    = FALSE;
  
  if(intensity_level !=Analogs_GetIntensity())
  {
    intensity_level =Analogs_GetIntensity();
    __HAL_TIM_SET_COMPARE( &TimHandle2, TIM_CHANNEL_1, intensity_level );
  }
  if(contrast_level !=Analogs_GetContrast())
  {
    contrast_level =Analogs_GetContrast();
    (void)HEL_LCD_Contrast( &LCDHandle, contrast_level );
  }
  
}