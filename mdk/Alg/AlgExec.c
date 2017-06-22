/************************************************************************************************************
 Module:       AlgExec.c
 
 Revision:     1.0
 
 Description:  This is the main step counting algorithm executive task in shine J.  It controls the flow of data into and out of
 the step counting algorithm.  The algorithm is run from main after each accelerometer watermark
 interrupt (every 24 100Hz samples, or every ~1/4 sec.).
 See https://misfit.jira.com/wiki/display/HAR/Step+Counting+Algorithm+Library for details about the algorithm
 data flows and control.
 
 Notes:        ---
 
 History:
 Date          Name      Changes
 -----------   ----      -------------------------------------------------------------------------------------
 MM/DD/YYYY    MRM       Began Coding    (MRM = Matt Montgomery)
 
 ************************************************************************************************************/

//###########################################################################################################
//      #INCLUDES
//###########################################################################################################
#include <stdbool.h>
#include "StepCount.h"
#include "AccelBuffer.h"
#include "GoalTracking.h"
#include "DownSample.h"
#include "AlgExec.h"
#include "File_Writer.h"
#include "Stream_Data.h"
#include "config.h"
#include "wsf_types.h"
#include "wsf_os.h"
#include "dm_api.h"
#include "att_api.h"
#include "mp_defs.h"
#include "mpsa_main.h"
#include "TapHandler.h"
#include "File_Normal_Entry.h"
#include "Minute_Data_Converter.h"
#include "ActivityTagging.h"
#include "timer.h"
//#include "button_lib.h"   //TODO: Remove after no longer using for temporary activity tagging


//###########################################################################################################
//      FILE NUMBER
//###########################################################################################################

//FILENUM(FILENUM_XXX);


//###########################################################################################################
//      TESTING #DEFINES
//###########################################################################################################



//###########################################################################################################
//      CONSTANT #DEFINES
//###########################################################################################################
//defines how long we wait for a swim lap to occur before we disable swim lap counting
#define MAX_SAMPLES_SINCE_SWIM_LAP (STEP_COUNT_ALG_SAMPLES_PER_MINUTE*15) //30 Minutes


//###########################################################################################################
//      MODULE TYPES
//###########################################################################################################

//control struture for the algorithm
typedef struct
{
  uint32_t processedSampleIdx;  //the current processed AccelBuffer sample relative to the first sample since init
  uint32_t currentMinute;       //the sample number of the current minute
		uint32_t lastProcessedMinute; //the sample number of the last processed minute
		real_t algWindowMemory[N_ACCEL_AXES][SC_ALG_MAX_WINDOW_LENGTH];  //holds pAxis for the algorithm window (currently 4bytes*3*75 = 900 bytes)
  real_t *pAxis[N_ACCEL_AXES]; //points to each row (axis) of the algWindowMemory 2-D array
  
  tag_state_t newTagState;
  tag_state_t currentTagState;
  bool        userTaggedOut;
  uint32_t    samplesSinceLastSwimLap;
  
}Alg_Cntrl_T;

static Alg_Cntrl_T AlgCntrl;



//###########################################################################################################
//      CONSTANTS
//###########################################################################################################



//###########################################################################################################
//      MODULE LEVEL VARIABLES
//###########################################################################################################



//###########################################################################################################
//      MACROS
//###########################################################################################################



//###########################################################################################################
//      PRIVATE FUNCTION PROTOTYPES
//###########################################################################################################
static void Check_Tag_State_Change(void);
static uint32_t Execute_Algorithm(real_t *data[N_ACCEL_AXES], int32_t len, step_count_result_t *result);


//###########################################################################################################
//      PUBLIC FUNCTIONS
//###########################################################################################################


/************************************************************************************************************
 Function:
 Alg_Exec( )
 
 Input Parameters:
 none
 
 Output Parameters:
 none
 
 Description:
 This is the step counting algorithm executive tasks for shine J.  It:
 1.) Reads pAxis from the AccelBuffer
 2.) If pAxis is available, calls the step counting Algorithm (SC_Execute)
 3.) If a new minute has passed it then:
 a.) Gets the minute-by-minute summary from the algorithm (SC_AggregateMinute)
 b.) Converts steps and points to variance for the sleep detection algorithm (when step counts are low)
 c.) Call the file writer to write minute-by-minute pAxis to the file system
 
 
 This function is scheduled to be called from main after each accelerometer interrupt (24 100Hz samples, or approx 250 ms).
 
 
 Notes:
 ---
 
 Author, Date:
 JPO, 7/15/14
 */
void Alg_Exec(void)
{
  step_count_result_t result;
  uint32_t offset;
  
  Check_Tag_State_Change();
  //execute algorithm if there is enough pAxis
  while(AccelBuffer_Read(AlgCntrl.pAxis, SC_GetWindowSize(), AlgCntrl.processedSampleIdx))
  {
    offset=Execute_Algorithm(AlgCntrl.pAxis, SC_GetWindowSize(), &result);
    
    //Stream_ActivityOutputData_Write(&result,AlgCntrl.processedSampleIdx);
    
    /*********************************************************************************
     //Handle window-by-window processing here. e.g, deal with result.motionPassed flag
     **********************************************************************************/
    
    //update curent pointer
    AlgCntrl.processedSampleIdx += offset;
    
    // Phu Tran, April 8, 2016: Steps needed to be updated more often than points
    GoalTracker_AddSteps(result.numSteps);
    
    AlgCntrl.currentMinute = AccelBuffer_GetCurrentMinuteSample();
    if((AlgCntrl.currentMinute != AlgCntrl.lastProcessedMinute) && (AlgCntrl.processedSampleIdx >= AlgCntrl.currentMinute))
    {
      AlgCntrl.lastProcessedMinute = AlgCntrl.currentMinute;
      
      minute_summary_t summary;
      
      SC_AggregateMinute(&summary);
      
      //Handle minute-by-minute processing here!!
      fe_normal_t entryWritten;
      
#if CONFIG_USE_BUTTON
      FileWriter_WriteStepsAndPoints(&summary, 0, 0, &entryWritten);
      // Phu Tran, June 8, 2016: doubleTap & tripleTaps params for FileWriter_WriteStepsAndPoints() function are set to 0
      // for now
#endif
      
#if CONFIG_USE_TAP_DETECTION
      tap_minute_stats_t tapStats;
      TapHandler_GetMinuteStats(&tapStats);
      FileWriter_WriteStepsAndPoints(&summary, tapStats.doubleTaps, tapStats.tripleTaps, &entryWritten);
#endif
      
      // Update Non-activity Goal Tracking count to File System
      //NonActivity_GoalTracking_FlushToFileSystem(NONACTIVITY_GOALTRACKING_NONIMMEDIATE_WRITE);
      
      ///JOA - TODO: Re-add these functions, possibly somewhere else so that they
      // will be updated more than once per minute.
      //TODO MRM PLUTO
#if 1
      //mpsaAdv_UpdateStepsPointsVariance(entryWritten.steps,entryWritten.points);
      //mpsaAdv_UpdateTime();   //XXX: change how we call this.  for now will only get updated once per minute
#else
#warning: "mpsaAdv_UpdateStepsPointsVariance and mpsaAdv_UpdateTimer is not called in Alg_Exec"
#endif
      
      //InactivityTracker_NotifyNewMinute(summary.steps, summary.accumVariance/summary.numWindows);
      
      // Phu Tran, April 8, 2016: add points after every new minute
      GoalTracker_AddPoints(summary.activityPoints);
      
    }
  }
}
/************************************************************************************************************
 Function:
 Alg_Reset( )
 
 Input Parameters:
 none
 
 Output Parameters:
 none
 
 Description:
 Reset the algorithm execution control structure.
 
 Notes:
 ---
 
 Author, Date:
 Phu Tran, 8/22/16
 */
void Alg_Reset(void)
{
  SC_Init();
  
  AlgCntrl.processedSampleIdx = 1;
  AlgCntrl.lastProcessedMinute = AccelBuffer_GetCurrentMinuteSample();
  AlgCntrl.samplesSinceLastSwimLap = 0;
}
/************************************************************************************************************
 Function:
 Alg_Init( )
 
 Input Parameters:
 none
 
 Output Parameters:
 none
 
 Description:
 Initialize the algorithm executive tack.  It is the callers responsibility to ensure that Alg_Init is called
 before Alg_Exec.
 
 Notes:
 ---
 
 Author, Date:
 JPO, 7/15/14
 */
void Alg_Init(void)
{
  int32_t i;
  
  SC_Init();
  
  for(i=0; i<N_ACCEL_AXES; i++)
  {
    AlgCntrl.pAxis[i] = AlgCntrl.algWindowMemory[i];
  }
  AlgCntrl.processedSampleIdx = 1;
  AlgCntrl.lastProcessedMinute = AccelBuffer_GetCurrentMinuteSample();
  AlgCntrl.currentTagState = TAG_STATE_OUT;
  AlgCntrl.newTagState = TAG_STATE_OUT;
  AlgCntrl.userTaggedOut = false;
  AlgCntrl.samplesSinceLastSwimLap = 0;
}

void Alg_NotifyTagIn(void)
{
  AlgCntrl.newTagState = TAG_STATE_IN;
  AlgCntrl.userTaggedOut = false;
  AlgCntrl.samplesSinceLastSwimLap = 0;
}

void Alg_NotifyTagOut(bool user_tag_out)
{
  AlgCntrl.newTagState = TAG_STATE_OUT;
  AlgCntrl.userTaggedOut = user_tag_out;
}




//###########################################################################################################
//      PRIVATE FUNCTIONS
//###########################################################################################################
void Check_Tag_State_Change(void)
{
#if CONFIG_LAP_COUNTING_ENABLED
  
  //check our own internal timeout, we only allow swim lap counting to be
  //enabled for at most 30 minutes after the last lap is detected.  If timeout
  //occured, force the tag state to tag out.
  //
  //Note: the Activity tagging module has a timeout of 4 hours and I did not want
  //to change that.  This means that we will have a "swim session" end marker in the file
  //3.5 hours before the "activity tag out marker" if the user forgets to tag out after swimming
  if(AlgCntrl.samplesSinceLastSwimLap >= MAX_SAMPLES_SINCE_SWIM_LAP)
  {
    AlgCntrl.newTagState = TAG_STATE_OUT;
    AlgCntrl.userTaggedOut = false;
  }
  
  if(AlgCntrl.currentTagState != AlgCntrl.newTagState)
  {
    AlgCntrl.currentTagState = AlgCntrl.newTagState;
    uint32_t seconds = clock_get_second();
    if(AlgCntrl.currentTagState == TAG_STATE_IN)
    {
      FileWriter_Write_Swim_Session_Marker(FE_SWIM_MARK_START,seconds);
      SWM_Init();  //switch to lap swimming mode
    }
    else
    {
      fe_swim_session_marker_t marker;
      if(AlgCntrl.userTaggedOut)
      {
        marker = FE_SWIM_MARK_USER_END;
      }
      else
      {
        marker = FE_SWIM_MARK_AUTO_END;
      }
      FileWriter_Write_Swim_Session_Marker(marker,seconds);
      SC_Init(); //switch to step counting mode
    }
  }
  
#endif
}

uint32_t Execute_Algorithm(real_t *data[N_ACCEL_AXES], int32_t len, step_count_result_t *result)
{
  uint32_t offset;
#if CONFIG_LAP_COUNTING_ENABLED
  
  if(AlgCntrl.currentTagState == TAG_STATE_IN)
  {
    //always run swim algorithm if we are tagged in
    //the app will figure out if it wants to show the laps to the user
    //if the user is tagged in as swimming or not
    lap_stats_t lap;
    offset = SWM_Exec(data,len,result,&lap);
    AlgCntrl.samplesSinceLastSwimLap += offset;
    //if a lap was detected, write it to the file
    if(lap.duration_in_10th_seconds > 0)
    {
      AlgCntrl.samplesSinceLastSwimLap = 0;
      fe_lap_info_data_t lap_for_file;
      lap_for_file.duration_in_10th_seconds = lap.duration_in_10th_seconds;
      lap_for_file.lap_end_in_10th_seconds = lap.lap_end_in_10th_seconds;
      lap_for_file.num_strokes = lap.nStrokes;
      lap_for_file.rfu = lap.svm;
      FileWriter_Write_Lap_Info(&lap_for_file);
    }
    
  }
  else
  {
    //run step counting when tagged out
    offset = SC_Execute(data,len,result);
  }
  
#else
  //if lap counting is not enabled, just call step counting algorithm
  offset = SC_Execute(data,len,result);
#endif
  
  return offset;
}


//###########################################################################################################
//      INTERRUPTS
//###########################################################################################################



//###########################################################################################################
//      TEST HARNESSES
//###########################################################################################################




//###########################################################################################################
//      END OF XXX.c
//###########################################################################################################

