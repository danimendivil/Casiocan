#include "app_analog.h"

/** 
  * @defgroup DMA values configurations
  @{ */
#define ARRAY_LENGHT      3u    /*!< Lenght of the array to stored with dma*/
/**
  @} */

/**
 * @brief  Variable for tim used by the pwm
 */
static TIM_HandleTypeDef TimHandle2;            /*TIM initial structure*/

/**
 * @brief  Array were data of the DMA will be stored
 */
static uint32_t AdcData[3u];

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
 *  In this function first we initialize the TIM3
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
  TimHandle2.Init.Prescaler     = 3200;
  TimHandle2.Init.Period        = 1000;
  TimHandle2.Init.CounterMode   = TIM_COUNTERMODE_UP;
  HAL_TIM_PWM_Init(&TimHandle2);
  
  sConfig.OCMode     = TIM_OCMODE_PWM1;
  sConfig.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfig.OCFastMode = TIM_OCFAST_DISABLE;
  sConfig.Pulse = 100;
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
  TimHandler.Init.Period            = 64;        /*value for 50ms*/
  TimHandler.Init.Prescaler         = 10000;
  TimHandler.Init.ClockDivision     = 0;
  TimHandler.Init.CounterMode       = TIM_COUNTERMODE_UP;
  TimHandler.Init.RepetitionCounter = 0x0;
  HAL_TIM_Base_Init( &TimHandler );
  
  /* Config timer to generate an internal trigger signal */
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
  sMasterConfig.MasterSlaveMode     = TIM_MASTERSLAVEMODE_DISABLE;
  HAL_TIMEx_MasterConfigSynchronization( &TimHandler, &sMasterConfig );

  /*enable DMA interrupts*/
  HAL_NVIC_SetPriority( DMA1_Channel1_IRQn, 2, 0 );
  HAL_NVIC_EnableIRQ( DMA1_Channel1_IRQn );

  /* Conversion time is given by::
  Tcon = ( Tsampling + Tconv ) / ADC clock 
  Tcon = ( AdcHandler.Init.SamplingTimeCommon1 + AdcHandler.Init.Resolution ) / ( APB Clock / AdcHandler.Init.ClockPrescaler )
  Tcon = ( 160.5 + 8.5 ) / 2MHz = 84.5us */
  AdcHandler.Instance                   = ADC1;
  AdcHandler.Init.ClockPrescaler        = ADC_CLOCK_SYNC_PCLK_DIV4;   /*APB clock divided by two*/
  AdcHandler.Init.Resolution            = ADC_RESOLUTION8b;           /*8 bit resolution with a Tconv of 8.5*/
  AdcHandler.Init.ScanConvMode          = ADC_SCAN_SEQ_FIXED;         /*scan adc channels from 0 to 16 in that order*/
  AdcHandler.Init.DataAlign             = ADC_DATAALIGN_RIGHT;        /*data converter is right alightned*/
  AdcHandler.Init.SamplingTimeCommon1   = ADC_SAMPLETIME_79CYCLES_5;    /*sampling time of 1.5*/  
  AdcHandler.Init.ExternalTrigConv      = ADC_EXTERNALTRIG_T4_TRGO;   /*set the timer TIM3 to trigger the ADC*/
  AdcHandler.Init.ExternalTrigConvEdge  = ADC_EXTERNALTRIGCONVEDGE_RISING;  /*only on rising edges*/
  AdcHandler.Init.DMAContinuousRequests = ENABLE;
  AdcHandler.Init.EOCSelection          = ADC_EOC_SEQ_CONV;        /*ISR at the end of one channel conversion*/
  AdcHandler.Init.Overrun               = ADC_OVR_DATA_OVERWRITTEN;   /*data will be overwriten in case is not read it*/
  AdcHandler.Init.OversamplingMode      = 4;   
  HAL_ADC_Init( &AdcHandler );
  /*apply ADC configuration*/
  
      /*config adc channel number 0*/
  sChanConfig.Channel = ADC_CHANNEL_0;
  sChanConfig.Rank = ADC_RANK_CHANNEL_NUMBER;
  sChanConfig.SamplingTime = ADC_SAMPLINGTIME_COMMON_1;
  /*apply channel configuration*/
  HAL_ADC_ConfigChannel( &AdcHandler, &sChanConfig );

  /*config adc channel number 0*/
  sChanConfig.Channel = ADC_CHANNEL_1;
  sChanConfig.Rank = ADC_RANK_CHANNEL_NUMBER;
  sChanConfig.SamplingTime = ADC_SAMPLINGTIME_COMMON_1;
  /*apply channel configuration*/
  HAL_ADC_ConfigChannel( &AdcHandler, &sChanConfig );
  

  /*config adc channel number 1*/
  sChanConfig.Channel = ADC_CHANNEL_TEMPSENSOR;
  sChanConfig.Rank = ADC_RANK_CHANNEL_NUMBER;
  sChanConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
  /*apply channel configuration*/
  HAL_ADC_ConfigChannel( &AdcHandler, &sChanConfig );

  /*Apply internal calibration*/
  HAL_ADCEx_Calibration_Start( &AdcHandler );

  HAL_ADC_Start_DMA( &AdcHandler, &AdcData[0], 3u );

  HAL_TIM_Base_Start( &TimHandler );
}

int8_t Analogs_GetTemperature( void )
{
  int8_t temperature;

  temperature = __HAL_ADC_CALC_TEMPERATURE( VDD_VALUE, AdcData[2], ADC_RESOLUTION8b);

  return temperature;
}

uint8_t Analogs_GetContrast( void )
{
  uint8_t contrast;

  contrast = (AdcData[1]*15u)/255u;

  return contrast;
}

uint8_t Analogs_GetIntensity( void )
{
  uint8_t intensity;

  intensity = (AdcData[0]*100u)/255u;

  return intensity;
}

void Display_LcdTask( void )
{
  static uint8_t intensity_level   = 0;
  static uint8_t contrast_level    = 0;
  
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