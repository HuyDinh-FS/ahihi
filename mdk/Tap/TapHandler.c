/*

*/
#include "TapHandler.h"
#include <limits.h>
#include <string.h>

typedef struct
{
  uint32_t singleTaps;
  uint32_t doubleTaps;
  uint32_t tripleTaps;
  uint32_t quadTaps;
}tap_cumulative_stats_t;


typedef enum{
  tap_err_ok                       = 0x00,       //The no error case is = zero 
  tap_log_num_double_triple_taps   = 0x80,
  tap_log_num_multi_taps           = 0x81,
  tap_log_num_single_quad_taps     = 0x82,
  tap_log_num_double_triple_taps2  = 0x83,
  tap_log_num_single_quad_taps2  = 0x84
  
}tap_log_t;

static tap_cumulative_stats_t cumulativeStats;
static tap_minute_stats_t minuteStats;

void TapHandler_LogCumulativeTaps(void)
{

}

void TapHandler_Init(void)
{
  TapDetection_Init();
}

/**
 * @brief   This function is called in case of power on reset, which causes RAM lost,
 *          in order to reset these stats number to 0. In case of soft reset (e.g. watchdog reset),
 *          these values are maintained since they are declared as "no_init" variables
 *
 * Input parameters: None
 *
 * Output parameters: None
 */
void TapHandler_NotifyRamLost(void)
{
  memset(&cumulativeStats,0,sizeof(tap_cumulative_stats_t));
  memset(&minuteStats,0,sizeof(tap_minute_stats_t));
}

void TapHandler_Exec(accel_value_t currentSample[N_ACCEL_AXES])
{
  uint32_t taps =  TapDetection_Exec(currentSample);
  
  if(taps == 0)
  {
    return;
  }
  else if(taps == 1)
  {
    cumulativeStats.singleTaps++;
  }
  else if(taps == 2)
  {
    cumulativeStats.doubleTaps++;
    minuteStats.doubleTaps++;
  }
  else if(taps == 3)
  {
    cumulativeStats.tripleTaps++;
    minuteStats.tripleTaps++;
  }
  else if(taps == 4)
  {
    cumulativeStats.quadTaps++;
  }
  
}



void TapHandler_GetMinuteStats(tap_minute_stats_t *minuteStatsOut)
{
  *minuteStatsOut = minuteStats;
  memset(&minuteStats,0,sizeof(tap_minute_stats_t));
}



