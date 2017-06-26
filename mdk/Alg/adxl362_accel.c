#include "scalgtypes.h"
#include "stdint.h"
#include "ADXL362.h"
#include "adxl362_spi.h"
#include "string.h"
#include "fsl_spi.h"
#include "accel_spi.h"
#include "AccelBuffer.h"
#include "TapHandler.h"
#include "spi_for_accel.h"


#define CONFIG_USE_TAP_DETECTION	1
#define RUN_ALGORITHM               1

#if USE_DMA_SPI_ACCEL
static uint32_t accel_tmp32_buf[ACCEL_NUM_SAMPLES_PER_WTM*3*2 + 1];
static uint32_t cmd[ACCEL_NUM_SAMPLES_PER_WTM*3*2 + 1];
#else
static uint8_t cmd[ACCEL_NUM_SAMPLES_PER_WTM*3*2 + 1];
#endif

volatile static uint16_t sample_count = 0;
volatile static uint8_t current_minute = 0;
volatile static uint8_t last_minute = 0;

//*****************************************************************************
//
//  Downsample filter
//
//  The filter that was explicitly designed was for a 100Hz accel data rate.
//  This is downsampled to 12.5 Hz before passing to the step counter.
//
//  CAVEAT:  All other filter rates (200, 50, 25) have approximate coefficients
//           made with linear interpolation.  They should be re-designed!
//
//  Sample rate is 100 Hz
//  Downsample by 8
//  Original values were created with
//      H = design(fdesign.lowpass('N,Fp,Fst',14,.05,.0625), 'ALLFIR');
//      b = H(2).Numerator;
//  Our values are multiplied by 2^19
//  Huy's note:
//      b's range    15 bits
//      data's range 12 bits
//      filter length 4 bits -> values fit in 31 bit
//
//  Note that in ProcessFifo the filter algorithm requires the number of
//  taps to be even.  This is easy to accomplish by appending a zero term
//  if the design has an odd number.
//
//*****************************************************************************

#define ACCEL_200Hz   0x01
#define ACCEL_100Hz   0x02
#define ACCEL_50Hz    0x03
#define ACCEL_25Hz    0x04
#define ACCEL_TAP     (CONFIG_USE_TAP_DETECTION ? 0x10 : 0)

#define ACCEL_200Hz_Tap   ( ACCEL_200Hz | ACCEL_TAP )
#define ACCEL_100Hz_Tap   ( ACCEL_100Hz | ACCEL_TAP )
#define ACCEL_100Hz_NoTap ( ACCEL_100Hz )
#define ACCEL_50Hz_NoTap  ( ACCEL_50Hz  )
#define ACCEL_25Hz_NoTap  ( ACCEL_25Hz  )

#if CONFIG_USE_TAP_DETECTION
#define ACCEL_CONFIG  ACCEL_100Hz_Tap
#else
#define ACCEL_CONFIG  ACCEL_100Hz_NoTap
#endif

#define ACCEL_FILTER  (ACCEL_CONFIG & 0x0F)
#if ACCEL_FILTER == ACCEL_200Hz

#define ACCEL_200Hz_FilterLength 32
#define ACCEL_200Hz_Stage1Offset 16
#define ACCEL_200Hz_ScaleFactor  19

static const int32_t ACCEL_200Hz_Coeff [ACCEL_200Hz_FilterLength] =
{
  10784, 11252, 11721, 12189, 12603, 13017, 13367, 13718,
  13999, 14280, 14484, 14689, 14814, 14939, 14980, 15022,
  14980, 14939, 14814, 14689, 14484, 14280, 13999, 13718,
  13367, 13017, 12603, 12189, 11721, 11252, 10784,     0
};

#elif (ACCEL_FILTER == ACCEL_100Hz) || (ACCEL_FILTER == ACCEL_100Hz_Tap)

#define ACCEL_100Hz_FilterLength 16
#define ACCEL_100Hz_Stage1Offset  8
#define ACCEL_100Hz_ScaleFactor  19

static const int32_t ACCEL_100Hz_Coeff [ACCEL_100Hz_FilterLength] =
{
  22504, 24378, 26033, 27436,
  28559, 29378, 29877, 30044,
  29877, 29378, 28559, 27436,
  26033, 24378, 22504,     0
};

#elif ACCEL_FILTER == ACCEL_50Hz

#define ACCEL_50Hz_FilterLength  8
#define ACCEL_50Hz_Stage1Offset  4
#define ACCEL_50Hz_ScaleFactor  19

static const int32_t ACCEL_50Hz_Coeff [ACCEL_50Hz_FilterLength] =
{
  48756, 54872,
  58756, 60088,
  58756, 54872,
  48756,     0
};

#elif ACCEL_FILTER == ACCEL_25Hz

#define ACCEL_25Hz_FilterLength  4
#define ACCEL_25Hz_Stage1Offset  2
#define ACCEL_25Hz_ScaleFactor  19

static const int32_t ACCEL_25Hz_Coeff [ACCEL_25Hz_FilterLength] =
{
  109744,
  120176,
  109744,
  0
};

#else
#error  No Accel Filter selected
#endif


//*****************************************************************************
// LOCAL STRUCT DEFS
//*****************************************************************************
typedef struct
{
  uint32_t  Length;
  uint32_t  Stage1Offset;
  uint32_t  ScaleFactor;
  const int32_t  *pCoeff;
  
}   AccelFilter_t;

static struct
{
  int32_t Index;
  int32_t xAxis;
  int32_t yAxis;
  int32_t zAxis;
  
}   FilterStage[2];

#if ACCEL_CONFIG == ACCEL_200Hz_Tap

static const AccelFilter_t AccelFilter =
{
  .Length       = ACCEL_200Hz_FilterLength,
  .Stage1Offset = ACCEL_200Hz_Stage1Offset,
  .ScaleFactor  = ACCEL_200Hz_ScaleFactor,
  .pCoeff       = ACCEL_200Hz_Coeff,
};

#elif ACCEL_CONFIG == ACCEL_100Hz_Tap

static const AccelFilter_t AccelFilter =
{
  .Length       = ACCEL_100Hz_FilterLength,
  .Stage1Offset = ACCEL_100Hz_Stage1Offset,
  .ScaleFactor  = ACCEL_100Hz_ScaleFactor,
  .pCoeff       = ACCEL_100Hz_Coeff,
};

#elif ACCEL_CONFIG == ACCEL_100Hz_NoTap

static const AccelFilter_t AccelFilter =
{
  .Length       = ACCEL_100Hz_FilterLength,
  .Stage1Offset = ACCEL_100Hz_Stage1Offset,
  .ScaleFactor  = ACCEL_100Hz_ScaleFactor,
  .pCoeff       = ACCEL_100Hz_Coeff,
};

#elif ACCEL_CONFIG == ACCEL_50Hz_NoTap

static const AccelFilter_t AccelFilter =
{
  .Length       = ACCEL_50Hz_FilterLength,
  .Stage1Offset = ACCEL_50Hz_Stage1Offset,
  .ScaleFactor  = ACCEL_50Hz_ScaleFactor,
  .pCoeff       = ACCEL_50Hz_Coeff,
};

#elif ACCEL_CONFIG == ACCEL_25Hz_NoTap

static const AccelFilter_t AccelFilter =
{
  .Length       = ACCEL_25Hz_FilterLength,
  .Stage1Offset = ACCEL_25Hz_Stage1Offset,
  .ScaleFactor  = ACCEL_25Hz_ScaleFactor,
  .pCoeff       = ACCEL_25Hz_Coeff,
};
#else
#error  No Accel configuration specified
#endif


//
// Buffer for holding ADXL samples.
//
static accel_value_t SampleBuffer[ ACCEL_NUM_SAMPLES_PER_WTM * 3 ];
static uint8_t TempBuffer[ACCEL_NUM_SAMPLES_PER_WTM*3*2 + 1];
//
// CAVEAT:  The SampleFiltered size assumes a downsample by 8.  This is not true for TAP products!
//
accel_value_t SampleFiltered[(ACCEL_NUM_SAMPLES_PER_WTM/4+1)*3];

static uint8_t accel_int_counter = 0;

//***************************************************************************************
// FUNCTIONS PROTOTYPES + EXTERNS
//***************************************************************************************
void adxl362_ProcessFifo( accel_value_t  *pRawData,                 // Where to store raw data
                         uint32_t        NumRawSampleSets,         // How many sample sets are in the Adxl FIFO
                         accel_value_t  *pFilteredData,            // Where to store filtered data
                         uint32_t       *pNumFilteredSampleSets ); // How many filtered sets were emitted
static void adxl362_ClearSampleBuffer( void );
void adxl362_ResetFilter( void );
void adxl362_PrepAlgorithm( accel_value_t *pRawData,
                           uint32_t       NumRawSampleSets,
                           accel_value_t *pFilteredData,
                           uint32_t       NumFilteredSampleSets );
void adxl362_accel_alg_data_handler (void * p_event_data, uint16_t event_size);
//***************************************************************************************
//* FUNCTION DEFINITIONS
//***************************************************************************************

uint8_t clock_get_minute(void)
{
  if (accel_int_counter >= 4)
  {
    accel_int_counter = 0;
    current_minute++;
  }
  return current_minute;
}

//*****************************************************************************
//
//  adxl362_accel_config_and_init
//  also enables SPI1 and DMA
//
//*****************************************************************************

void adxl362_accel_config_and_init (void)
{
  adxl362_ClearSampleBuffer();
  adxl362_ResetFilter();
  //spi1_enable_disable(true);
  // these will be fed data from the accel interrupt
  //AccelBuffer_Init();
  adxl362_ResetFilter();
}

//*****************************************************************************
//
// Handler for DMA tranmission complete
//
//*****************************************************************************

void adxl362_accel_fifo_receive_handler(void)
{
  uint32_t NumFilteredSampleSets = 0;
  //*************************************************************************
  //
  //  Read the samples from the FIFO and filter them.
  //
  //*************************************************************************
  
  adxl362_ProcessFifo( SampleBuffer,
                      ACCEL_NUM_SAMPLES_PER_WTM,
                      SampleFiltered,
                      &NumFilteredSampleSets );
  
  //*************************************************************************
  //
  //  Don't process the first junk from the Adxl
  //
  //  CAVEAT: Previous code checked for wrap, but this only rolls over every
  //          1024*1024*1024 seconds for tap products, roughly 34 years...
  //
  //*************************************************************************
  
  {
    static uint32_t Watermark_Isr_Count = 0;
    
    if( 0 == Watermark_Isr_Count++ )
    return;
  }
  
  //*************************************************************************
  //
  //  Prepare data for the algorithm.
  //
  //*************************************************************************
  
#ifdef RUN_ALGORITHM
  adxl362_PrepAlgorithm( SampleBuffer,
                        ACCEL_NUM_SAMPLES_PER_WTM,
                        SampleFiltered,
                        NumFilteredSampleSets );
  adxl362_accel_alg_data_handler(NULL,0);
#endif
}
//*****************************************************************************
//
//  Adxl362_WtmIsr       Interrupts are disabled!
//
//  Called from main::am_gpio_isr when the adxl raises INT1.
//  This occurs when the Accel FIFO watermark is reached at 50 sample sets.
//  Here we initiate a chained read of the samples into the SPI FIFO.
//
//  NJH 10/6/2016
//
//*****************************************************************************

void adxl362_WtmIsr( void )
{
    volatile uint16_t fifo_sample;
    volatile uint8_t i = 0;
    spi_transfer_t transferConfig = {0};
#if USE_DMA_SPI_ACCEL
    //memset(cmd,0xAA, SIZE_OF_ACCEL_BUFFER*sizeof(uint32_t));
    //for (i = 0; i < SIZE_OF_ACCEL_BUFFER; i++)
    //{
    //    cmd[i] = 0x000000AA;
    //}
#else
    //memset(cmd,0xAA, SIZE_OF_ACCEL_BUFFER);
#endif
    cmd[0] = ADXL362_WRITE_FIFO;
    
    sample_count++;
    if (sample_count >= 480)
    {
        current_minute++;
        sample_count = 0;
    }
    i++;
    
    transferConfig.txData = (uint8_t *) cmd;
    transferConfig.configFlags |= kSPI_FrameAssert;
#if USE_DMA_SPI_ACCEL
    transferConfig.rxData = (uint8_t *) TempBuffer;
    transferConfig.dataSize = (ACCEL_NUM_SAMPLES_PER_WTM*2*3 + 1);
    accel_dma_transfer(&transferConfig);
#else
    transferConfig.rxData = (uint8_t *) TempBuffer;
    transferConfig.dataSize = (ACCEL_NUM_SAMPLES_PER_WTM*2*3 + 1);
	accel_nondma_transfer(&transferConfig);
    accel_spi_deinit();
    adxl362_accel_fifo_receive_handler();
#endif
}


// Process FIFO ported from SAM
// This is potentially inefficient

void adxl362_ProcessFifo( accel_value_t  *pRawData,                 // Where to store raw data
                         uint32_t        NumRawSampleSets,         // How many samples are in the Adxl FIFO
                         accel_value_t  *pFilteredData,            // Where to store filtered data
                         uint32_t       *pNumFilteredSampleSets)   // How many filtered sample sets were emitted
{
  // HDinh: Used in place of reading SPI data from IOM from Ambiq
  // Should start at second byte since first byte is
  // the read command response
  uint32_t * accel_spi_data = (uint32_t *) (TempBuffer + 1);
  
  //*************************************************************************
  //
  //  Union to break apart the word read from the SPI FIFO.
  //  The SPI reads 4 bytes at a time which are two axes from the adxl362
  //
  //*************************************************************************
  
  union
  {
    uint32_t Raw;
    
    struct
    {
      int32_t SampLo:12;
      int32_t AxisLo: 4;
      int32_t SampHi:12;
      int32_t AxisHi: 4;
      
    }   Field;
  }   TwoSample;
  
  int32_t xAxis[2];
  int32_t yAxis[2];
  int32_t zAxis[2];
  int32_t Coeff[2];
  
  //*************************************************************************
  //
  //  Local copy of the sign-extended samples from 2 sample sets.
  //
  //*************************************************************************
  
  uint32_t NumDoubleSetsInFifo   = 0;     // As read from IOM
  uint32_t NumDoubleSetsToGo     = 0;     // Our loop does 2 at a time
  uint32_t NumFilteredSampleSets = 0;     // Count of how many filtered sample sets were emitted
  static uint32_t uSecsHoldOff   = 1;     // How many loops to delay by  CAVEAT:  should be at least 1 for hal_delay_us
  uint32_t TuneCounter           = 0;     // How many unneeded io polls were made.  For tuning uSecsHoldOff
  bool     FirstSetFlag          = true;  // True if this is the first set.  For a different holdoff
  const uint32_t NumDoubleSetsPerWait =  2; // How many double sets to accumulate in FIFO
  const uint32_t BytesPerDoubleSet    = 12;
  
  //*************************************************************************
  //
  //  We're reading two sample sets at time to properly unroll the loop
  //
  //*************************************************************************
  
  for( NumDoubleSetsToGo = NumRawSampleSets/2; NumDoubleSetsToGo; --NumDoubleSetsToGo )
  {
    //*********************************************************************
    //
    //  Fetch two sample sets
    //
    //*********************************************************************
    
    TwoSample.Raw = (*((uint32_t*)(accel_spi_data)++));
    *pRawData++ = xAxis[0] = TwoSample.Field.SampLo;
    *pRawData++ = yAxis[0] = TwoSample.Field.SampHi;
    
    TwoSample.Raw = (*((uint32_t*)(accel_spi_data)++));
    *pRawData++ = zAxis[0] = TwoSample.Field.SampLo;
    *pRawData++ = xAxis[1] = TwoSample.Field.SampHi;
    
    TwoSample.Raw = (*((uint32_t*)(accel_spi_data)++));
    *pRawData++ = yAxis[1] = TwoSample.Field.SampLo;
    *pRawData++ = zAxis[1] = TwoSample.Field.SampHi;
    
    //*********************************************************************
    //
    //  Pass them through the filter
    //
    //  Note that this implementation requires the number of filter taps to
    //  be even.  This is easy to implement by appending a zero term if the
    //  length is odd.
    //
    //*********************************************************************
    
    Coeff[0] = AccelFilter.pCoeff[FilterStage[0].Index    ];
    Coeff[1] = AccelFilter.pCoeff[FilterStage[0].Index + 1];
    
    FilterStage[0].xAxis += Coeff[0] * xAxis[0] + Coeff[1] * xAxis[1];
    FilterStage[0].yAxis += Coeff[0] * yAxis[0] + Coeff[1] * yAxis[1];
    FilterStage[0].zAxis += Coeff[0] * zAxis[0] + Coeff[1] * zAxis[1];
    
    Coeff[0] = AccelFilter.pCoeff[FilterStage[1].Index    ];
    Coeff[1] = AccelFilter.pCoeff[FilterStage[1].Index + 1];
    
    FilterStage[1].xAxis += Coeff[0] * xAxis[0] + Coeff[1] * xAxis[1];
    FilterStage[1].yAxis += Coeff[0] * yAxis[0] + Coeff[1] * yAxis[1];
    FilterStage[1].zAxis += Coeff[0] * zAxis[0] + Coeff[1] * zAxis[1];
    
    //*********************************************************************
    //
    //  If a filter stage wraps,
    //   - Emit downsampled output
    //   - Reset the filter
    //
    //*********************************************************************
    
    FilterStage[0].Index += 2;
    
    if( AccelFilter.Length <= FilterStage[0].Index )
    {
      *pFilteredData++ = FilterStage[0].xAxis >> AccelFilter.ScaleFactor;
      *pFilteredData++ = FilterStage[0].yAxis >> AccelFilter.ScaleFactor;
      *pFilteredData++ = FilterStage[0].zAxis >> AccelFilter.ScaleFactor;
      
      memset( &FilterStage[0], 0, sizeof( FilterStage[0] ));
      ++NumFilteredSampleSets;
    }
    
    FilterStage[1].Index += 2;
    
    if( AccelFilter.Length <= FilterStage[1].Index )
    {
      *pFilteredData++ = FilterStage[1].xAxis >> AccelFilter.ScaleFactor;
      *pFilteredData++ = FilterStage[1].yAxis >> AccelFilter.ScaleFactor;
      *pFilteredData++ = FilterStage[1].zAxis >> AccelFilter.ScaleFactor;
      
      memset( &FilterStage[1], 0,sizeof( FilterStage[1] ));
      ++NumFilteredSampleSets;
    }
  }
  
  *pNumFilteredSampleSets = NumFilteredSampleSets;
  
  //*************************************************************************
  //
  //  Tune the wait holdoff.  A little hysteresis keeps the unnecessary polls
  //  in a tight range.
  //
  //*************************************************************************
  
  if( TuneCounter > 10 )
  {
    if( uSecsHoldOff < NumDoubleSetsPerWait * BytesPerDoubleSet - 4 )
    ++uSecsHoldOff;
  }
  else if( TuneCounter == 0 )
  {
    if( uSecsHoldOff > 1 )
    --uSecsHoldOff;
  }
}

//*****************************************************************************
//
//  adxl362_accel_alg_data_handler
//
//  Event handler for new accelerometer data available.
//  Finishes any Stream data and calls the step counting algorithm.
//
//*****************************************************************************

void adxl362_accel_alg_data_handler (void * p_event_data, uint16_t event_size)
{
    Alg_Exec();
}

//*****************************************************************************
//
//  adxl362_PrepAlgorithm      Interrupts are disabled!
//
//  CAVEAT:   This entire routine is fairly useless.
//            Better to not Stream in customer mode.
//            Also better to directly write the AccelBuffer in ProcessFifo
//
//*****************************************************************************

void adxl362_PrepAlgorithm( accel_value_t *pRawData,
                           uint32_t       NumRawSampleSets,
                           accel_value_t *pFilteredData,
                           uint32_t       NumFilteredSampleSets )
{
  uint8_t CurrentMinute     = 0;
  static uint8_t LastMinute = 0;
  
  CurrentMinute = (uint8_t) clock_get_minute();
  
  //*************************************************************************
  //
  //  Push raw data to Tap detector
  //
  //*************************************************************************
  
#if ACCEL_TAP
  {
    accel_value_t *pData = pRawData;
    
    for( int i=0; i< NumRawSampleSets; ++i )
    {
      TapHandler_Exec( pData );
      pData += 3;
    }
  }
#endif
  
  //*************************************************************************
  //
  //  Push downsampled data to the AccelBuffer
  //
  //*************************************************************************
  {
    AccelBuffer_WriteSamples( pFilteredData,
                             NumFilteredSampleSets,
                             LastMinute != CurrentMinute );
    LastMinute = CurrentMinute;
  }
}

//*****************************************************************************
//
//  adxl362_ClearSampleBuffer
//
//*****************************************************************************

static void adxl362_ClearSampleBuffer( void )
{
  memset( (void *)&SampleBuffer, 0, sizeof(SampleBuffer) );
}

//*****************************************************************************
//
//  adxl362_ResetFilter
//
//*****************************************************************************

void adxl362_ResetFilter( void )
{
  memset( FilterStage, 0, sizeof( FilterStage ));
  
  FilterStage[1].Index = AccelFilter.Stage1Offset;
}
