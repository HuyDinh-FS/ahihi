#ifndef _TAP_HANDLER_H
#define _TAP_HANDLER_H

//#######################################################################################
// INTERRUPT SERVICE ROUTINE PROTOTYPES
//#######################################################################################

#include "typedef.h"
#include "TapDetection.h"


typedef struct
{
  uint8_t doubleTaps;
  uint8_t tripleTaps;
}tap_minute_stats_t;




extern void TapHandler_Init(void);
extern void TapHandler_NotifyRamLost(void);
extern void TapHandler_GetMinuteStats(tap_minute_stats_t *minuteStatsOut);
extern void TapHandler_LogCumulativeTaps(void);
extern void TapHandler_Exec(accel_value_t currentSample[N_ACCEL_AXES]);



#endif
