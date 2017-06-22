/************************************************************************************************************
 Module:       GoalTracking.c
 
 Revision:     1.0
 
 Description:  ---
 
 Notes:        ---
 
 History:
 Date          Name      Changes
 -----------   ----      -------------------------------------------------------------------------------------
 MM/DD/YYYY    MRM       Began Coding    (MRM = Matt Montgomery)
 
 ************************************************************************************************************/

//###########################################################################################################
//      #INCLUDES
//###########################################################################################################

#include "typedef.h"
#include "GoalTracking.h"
//#include "hw_logger.h"
//#include "config.h"
//#include "bond_storage.h"
//#include "setting_items.h"
//#include "misfit_events.h"
//#include "Goal_Animation_Module.h"

//###########################################################################################################
//      FILE NUMBER
//###########################################################################################################

//FILENUM(FILENUM_XXX);


//###########################################################################################################
//      TESTING #DEFINES
//###########################################################################################################

#if TESTING_GOAL_TRACKING

#define TEST_GET_PERCENT_WITH_ANIM    1

#endif


//###########################################################################################################
//      CONSTANT #DEFINES
//###########################################################################################################

//Multiple by 256 since activity points reported by the algorithm
//are scaled by 256.  Also, the app expects goals and points to be scaled by
//this factor
#define DEFAULT_GOAL_IN_POINTS (2500*256)   // 2500 activity file points, 1000 app display points, equivelent to ~10000 steps

#define DEFAULT_GOAL_IN_STEPS (10000)

//###########################################################################################################
//      MODULE TYPES
//###########################################################################################################

typedef struct
{
  uint32_t goal;
  uint32_t pointsToday;
}
GoalTracker_Points_t;

typedef struct
{
  uint32_t goal;
  uint32_t stepsToday;
}
GoalTracker_Steps_t;

typedef enum
{
  goalTracker_err_ok                   = 0x00,       //The no error case is = zero
  goalTracker_log_set_goal_points_old         = 0x80,
  goalTracker_log_set_goal_points_new         = 0x81,
  goalTracker_log_set_progress_points_old     = 0x82,
  goalTracker_log_set_progress_points_new     = 0x83,
  goalTracker_log_reset_progress_points   = 0x84,
  goalTracker_log_reset_progress_steps    = 0x85,
  
  goalTracker_log_set_goal_steps_old         = 0x86,
  goalTracker_log_set_goal_steps_new         = 0x87,
  goalTracker_log_set_progress_steps_old     = 0x88,
  goalTracker_log_set_progress_steps_new     = 0x89,
  
}
GoalTracker_log_t;


//###########################################################################################################
//      CONSTANTS
//###########################################################################################################



//###########################################################################################################
//      MODULE LEVEL VARIABLES
//###########################################################################################################

static GoalTracker_Points_t GoalTrack_Points;
static GoalTracker_Steps_t GoalTrack_Steps;
static uint32_t Last_Percent_Today = UINT32_MAX;
static uint16_t Goal_Percent_Threshold = UINT16_MAX;
static uint16_t Percent_For_Goal_Animation = UINT16_MAX;

//###########################################################################################################
//      MACROS
//###########################################################################################################



//###########################################################################################################
//      PRIVATE FUNCTION PROTOTYPES
//###########################################################################################################
//static void goaltracking_SaveGoalsToFlash(void);


//###########################################################################################################
//      PUBLIC FUNCTIONS
//###########################################################################################################
/************************************************************************************************************
 Function:
 GoalTracker_RamLost( )
 
 Input Parameters:
 ---
 
 Output Parameters:
 ---
 
 Description:
 ---
 
 Notes:
 ---
 
 Author, Date:
 MRM, MM/DD/YYYY
 */
void GoalTracker_RamLost(void)
{
  // reset all curr steps & points
  GoalTrack_Steps.stepsToday = 0;
  GoalTrack_Points.pointsToday = 0;
}

/************************************************************************************************************
 Function:
 GoalTracker_GoalRecover( )
 
 Input Parameters:
 ---
 
 Output Parameters:
 ---
 
 Description:
 ---
 
 Notes:
 ---
 
 Author, Date:
 Phu Tran, 07/14/2016
 */
void GoalTracker_GoalRecover(void)
{
  uint32_t goals_steps_points[2];
  uint32_t default_goals[2] = {DEFAULT_GOAL_IN_POINTS, DEFAULT_GOAL_IN_STEPS};
  
  
  // Get goal in steps and goal in points from flash memory, format: first word is goal in points, second word is goal in stepss
  //BondStorage_GetGoal((uint8_t *)goals_steps_points, sizeof(goals_steps_points));
  //get_setting(&device_settings.goal_in_point, (uint8_t*)&goals_steps_points[0], sizeof(uint32_t));
  //get_setting(&device_settings.goal_in_step, (uint8_t*)&goals_steps_points[1], sizeof(uint32_t));
  
  /* Sanity check for each goal */
  uint8_t len = sizeof(goals_steps_points) / sizeof(uint32_t);
  for(int i = 0; i < len; i++)
  {
    if(goals_steps_points[i] == 0xFFFFFFFF)
    {
      // If empty goal in flash, use default value
      goals_steps_points[i] = default_goals[i];
    }
  }
  
  // Update module variables
  GoalTrack_Points.goal = goals_steps_points[0];
  GoalTrack_Steps.goal = goals_steps_points[1];
}

/************************************************************************************************************
 Function:
 GoalTracker_NotifyNewDay( )
 
 Input Parameters:
 ---
 
 Output Parameters:
 ---
 
 Description:
 ---
 
 Notes:
 ---
 
 Author, Date:
 ---
 */
void GoalTracker_NotifyNewDay(void)
{
  //Add HW Log Entries
#define POINTS_PROGRESS_RESET_LOG_BYTES  7
  u8 bytes[POINTS_PROGRESS_RESET_LOG_BYTES];
  u8 *p = bytes;
  //UINT8_TO_BSTREAM(p, goalTracker_log_reset_progress_points);
  //UINT32_TO_BSTREAM(p, GoalTrack_Points.pointsToday);
  //UINT16_TO_BSTREAM(p, (u16)GoalTracker_GetPercentToday() );
  //hwLog_WriteLogEntry( log_algorithm, bytes, POINTS_PROGRESS_RESET_LOG_BYTES);
  
  //Add HW Log Entries
#define STEPS_RESET_LOG_BYTES  5
  u8 bytes_2[STEPS_RESET_LOG_BYTES];
  p = bytes_2;
  //UINT8_TO_BSTREAM(p, goalTracker_log_reset_progress_steps);
  //UINT32_TO_BSTREAM(p, GoalTrack_Steps.stepsToday);
  //hwLog_WriteLogEntry( log_algorithm, bytes_2, STEPS_RESET_LOG_BYTES);
  
  GoalTrack_Points.pointsToday = 0;
  GoalTrack_Steps.stepsToday = 0;
}

/************************************************************************************************************
 Function:
 GoalTracker_SetGoalInPoints( )
 
 Input Parameters:
 ---
 
 Output Parameters:
 ---
 
 Description:
 ---
 
 Notes:
 ---
 
 Author, Date:
 ---
 */
void GoalTracker_SetGoalInPoints(uint32_t goalToSet)
{
  
  //Add HW Log Entries
#define GOAL_SET_LOG_BYTES  5
  u8 bytes[GOAL_SET_LOG_BYTES];
  u8 *p = bytes;
  //UINT8_TO_BSTREAM(p, goalTracker_log_set_goal_points_old);
  //UINT32_TO_BSTREAM(p, GoalTrack_Points.goal);
  //hwLog_WriteLogEntry( log_algorithm, bytes, GOAL_SET_LOG_BYTES);
  p = bytes;
  //UINT8_TO_BSTREAM(p, goalTracker_log_set_goal_points_new);
  //UINT32_TO_BSTREAM(p, goalToSet);
  //hwLog_WriteLogEntry( log_algorithm, bytes, GOAL_SET_LOG_BYTES);
  
  // Set goal
  if( GoalTrack_Points.goal != goalToSet )
  {
    GoalTrack_Points.goal = goalToSet;
    
    // Write new goal to flash memory
    //goaltracking_SaveGoalsToFlash();
    //if (set_setting(&device_settings.goal_in_point, (uint8_t*)&goalToSet, sizeof(uint32_t)))
    //{
      // fail to update, need to do somthing here
    //}
  }
  
}

/************************************************************************************************************
 Function:
 GoalTracker_GetGoalInPoints( )
 
 Input Parameters:
 ---
 
 Output Parameters:
 ---
 
 Description:
 ---
 
 Notes:
 ---
 
 Author, Date:
 ---
 */
uint32_t GoalTracker_GetGoalInPoints(void)
{
  return (GoalTrack_Points.goal);
}

/************************************************************************************************************
 Function:
 GoalTracker_SetPointsToday( )
 
 Input Parameters:
 ---
 
 Output Parameters:
 ---
 
 Description:
 ---
 
 Notes:
 ---
 
 Author, Date:
 ---
 */
void GoalTracker_SetPointsToday(uint32_t points)
{
  
  // Add HW Log Entries
#define PROGRESS_SET_LOG_BYTES  5
  u8 bytes[PROGRESS_SET_LOG_BYTES];
  u8 *p = bytes;
  //UINT8_TO_BSTREAM(p, goalTracker_log_set_progress_points_old);
  //UINT32_TO_BSTREAM(p, GoalTrack_Points.pointsToday);
  //hwLog_WriteLogEntry( log_algorithm, bytes, PROGRESS_SET_LOG_BYTES);
  p = bytes;
  //UINT8_TO_BSTREAM(p, goalTracker_log_set_progress_points_new);
  //UINT32_TO_BSTREAM(p, points);
  //hwLog_WriteLogEntry( log_algorithm, bytes, PROGRESS_SET_LOG_BYTES);
  
  // Set the points
  GoalTrack_Points.pointsToday = points;
  
}

/************************************************************************************************************
 Function:
 GoalTracker_GetPointsToday( )
 
 Input Parameters:
 ---
 
 Output Parameters:
 ---
 
 Description:
 ---
 
 Notes:
 ---
 
 Author, Date:
 ---
 */
uint32_t GoalTracker_GetPointsToday(void)
{
  return (GoalTrack_Points.pointsToday);
}

/************************************************************************************************************
 Function:
 GoalTracker_SetGoalInSteps( )
 
 Input Parameters:
 ---
 
 Output Parameters:
 ---
 
 Description:
 ---
 
 Notes:
 ---
 
 Author, Date:
 ---
 */
void GoalTracker_SetGoalInSteps(uint32_t goalToSet)
{
  
  //Add HW Log Entries
#define GOAL_SET_LOG_BYTES  5
  u8 bytes[GOAL_SET_LOG_BYTES];
  u8 *p = bytes;
  //UINT8_TO_BSTREAM(p, goalTracker_log_set_goal_steps_old);
  //UINT32_TO_BSTREAM(p, GoalTrack_Steps.goal);
  //hwLog_WriteLogEntry( log_algorithm, bytes, GOAL_SET_LOG_BYTES);
  p = bytes;
  //UINT8_TO_BSTREAM(p, goalTracker_log_set_goal_steps_new);
  //UINT32_TO_BSTREAM(p, goalToSet);
  //hwLog_WriteLogEntry( log_algorithm, bytes, GOAL_SET_LOG_BYTES);
  
  // Set goal
  if( GoalTrack_Steps.goal != goalToSet )
  {
    GoalTrack_Steps.goal = goalToSet;
    
    // Write to flash memory if allow to use step concept
    //goaltracking_SaveGoalsToFlash();
    //if (set_setting(&device_settings.goal_in_step, (uint8_t*)&goalToSet, sizeof(uint32_t)))
    //{
      // fail to update, need to do somthing here
    //}
  }
}

/************************************************************************************************************
 Function:
 GoalTracker_GetGoalInSteps( )
 
 Input Parameters:
 ---
 
 Output Parameters:
 ---
 
 Description:
 ---
 
 Notes:
 ---
 
 Author, Date:
 ---
 */
uint32_t GoalTracker_GetGoalInSteps(void)
{
  return (GoalTrack_Steps.goal);
}

/************************************************************************************************************
 Function:
 GoalTracker_SetStepsToday( )
 
 Input Parameters:
 ---
 
 Output Parameters:
 ---
 
 Description:
 ---
 
 Notes:
 ---
 
 Author, Date:
 ---
 */
void GoalTracker_SetStepsToday(uint32_t steps)
{
  // Add HW Log Entries
#define PROGRESS_SET_LOG_BYTES  5
  u8 bytes[PROGRESS_SET_LOG_BYTES];
  u8 *p = bytes;
  //UINT8_TO_BSTREAM(p, goalTracker_log_set_progress_steps_old);
  //UINT32_TO_BSTREAM(p, GoalTrack_Steps.stepsToday);
  //hwLog_WriteLogEntry( log_algorithm, bytes, PROGRESS_SET_LOG_BYTES);
  p = bytes;
  //UINT8_TO_BSTREAM(p, goalTracker_log_set_progress_steps_new);
  //UINT32_TO_BSTREAM(p, steps);
  //hwLog_WriteLogEntry( log_algorithm, bytes, PROGRESS_SET_LOG_BYTES);
  
  // Set the steps
  GoalTrack_Steps.stepsToday = steps;
}

/************************************************************************************************************
 Function:
 GoalTracker_GetStepsToday( )
 
 Input Parameters:
 ---
 
 Output Parameters:
 ---
 
 Description:
 ---
 
 Notes:
 ---
 
 Author, Date:
 ---
 */
uint32_t GoalTracker_GetStepsToday(void)
{
  return (GoalTrack_Steps.stepsToday);
}

/************************************************************************************************************
 Function:
 GoalTracker_GetPercentToday( )
 
 Input Parameters:
 ---
 
 Output Parameters:
 ---
 
 Description:
 ---
 
 Notes:
 ---
 
 Author, Date:
 ---
 */
uint32_t GoalTracker_GetPercentToday(void)
{
  uint32_t percent_today;
  
  // For SAM, steps is used for calculating progress per day
  percent_today = ( (100 * GoalTrack_Steps.stepsToday) / GoalTrack_Steps.goal);
  
  // Check whether the goal threshold has been crossed
  if ( (Goal_Percent_Threshold <= percent_today) && (Goal_Percent_Threshold > Last_Percent_Today)
      && (UINT32_MAX != Last_Percent_Today) && Goal_Percent_Threshold <= 100)
  {
    //msf_events_set_event(MSF_EVENT_PROGRESS_GOAL_MET);
    Percent_For_Goal_Animation = Goal_Percent_Threshold;
  }
  
  // Update history variables
  Last_Percent_Today = percent_today;
  
  // Calculate the threshold percentage for goal celebration animations
  Goal_Percent_Threshold = ((GoalTrack_Steps.stepsToday/GoalTrack_Steps.goal)+1)*100;
  
  return percent_today;
}

/************************************************************************************************************
 Function:
 GoalTracker_NotifyNewMinute( )
 
 Input Parameters:
 ---
 
 Output Parameters:
 ---
 
 Description:
 ---
 
 Notes:
 ---
 
 Author, Date:
 ---
 */
void GoalTracker_NotifyNewMinute(uint32_t pointsThisMinute, uint32_t stepsThisMinute)
{
  GoalTrack_Points.pointsToday += pointsThisMinute;
  GoalTrack_Steps.stepsToday += stepsThisMinute;
}

/************************************************************************************************************
 Function:
 GoalTracker_AddPoints( )
 
 Input Parameters:
 ---
 
 Output Parameters:
 ---
 
 Description:
 ---
 
 Notes:
 ---
 
 Author, Date:
 ---
 */
void GoalTracker_AddPoints(uint32_t points)
{
  GoalTrack_Points.pointsToday += points;
}

/************************************************************************************************************
 Function:
 GoalTracker_AddSteps( )
 
 Input Parameters:
 ---
 
 Output Parameters:
 ---
 
 Description:
 ---
 
 Notes:
 ---
 
 Author, Date:
 ---
 */
void GoalTracker_AddSteps(uint32_t steps)
{
  GoalTrack_Steps.stepsToday += steps;
}

/************************************************************************************************************
 Function:
 GoalTracker_GoalMet_Event( )
 
 Input Parameters:
 ---
 
 Output Parameters:
 ---
 
 Description:
 ---
 
 Notes:
 ---
 
 Author, Date:
 ---
 */
void GoalTracker_GoalMet_Event(void *p_event_data, uint16_t event_size)
{
  // Call goal animation function
  //Start_Goal_Animation(Percent_For_Goal_Animation);
}

//###########################################################################################################
//      PRIVATE FUNCTIONS
//###########################################################################################################
#if 0 // Removed during migration to PDS
static void goaltracking_SaveGoalsToFlash(void)
{
  uint32_t goals_to_write[2] = {GoalTrack_Points.goal, GoalTrack_Steps.goal};
  
  // write in bond storage
  BondStorage_UpdateGoal((uint8_t *)goals_to_write, sizeof(goals_to_write));
}
#endif

//###########################################################################################################
//      INTERRUPTS
//###########################################################################################################



//###########################################################################################################
//      TEST HARNESSES
//###########################################################################################################

#if TEST_GET_PERCENT_WITH_ANIM
#include "sd_interface.h"
#include "Animations_SM.h"
#include "leds.h"
#include "rtc.h"
volatile uint32_t goal;
volatile uint32_t points;
int main(void)
{
  
#ifndef _NO_SOFT_DEVICE
  SDIntfc_Init();
#endif
  
  rtc1_init();
  leds_init();
  
#if 1
  GoalTracker_SetGoal(640000);        //Gets divided by 256 when stored internally
  //GoalTracker_SetPointsToday(0);
  GoalTracker_SetPointsToday(640000 - 1);
  //GoalTracker_SetPointsToday(640000);
  //GoalTracker_SetPointsToday(640000/2);
#else
  GoalTrack.goal = 2500;
  //GoalTrack.pointsToday = 0;
  GoalTrack.pointsToday = GoalTrack.goal - 1;
  //GoalTrack.pointsToday = GoalTrack.goal;
  //GoalTrack.pointsToday = GoalTrack.goal/2;
#endif
  
  goal = GoalTracker_GetGoal();
  points = GoalTracker_GetPointsToday();
  
  AnimationsSM_Start(ANIM_EV_ENTRY);
  AnimationsSM_Run(ANIM_EV_BUTTON_CLICK);
  
  while(1);
  
}
#endif


//###########################################################################################################
//      END OF GoalTracking.c
//###########################################################################################################

