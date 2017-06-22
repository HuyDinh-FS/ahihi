/************************************************************************************************************
Module:		    GoalTracker.c

Revision:	    1.0

Description:	---

Notes:			  ---

History:
Date		      Name	    Changes
-----------	  ----	    -------------------------------------------------------------------------------------
MM/DD/YYYY	  MRM		    Began Coding    (MRM = Matt Montgomery)

************************************************************************************************************/

#ifndef _GOAL_TRACKING_H
#define _GOAL_TRACKING_H

//###########################################################################################################
// #INCLUDES
//###########################################################################################################

#include "typedef.h"


//###########################################################################################################
// DEFINED CONSTANTS
//###########################################################################################################



//###########################################################################################################
// DEFINED TYPES
//###########################################################################################################



//###########################################################################################################
// DEFINED MACROS
//###########################################################################################################



//###########################################################################################################
// FUNCTION PROTOTYPES
//###########################################################################################################

void GoalTracker_RamLost(void);
void GoalTracker_GoalRecover(void);

/* For Points */
void GoalTracker_SetGoalInPoints(uint32_t goalToSet);
uint32_t GoalTracker_GetGoalInPoints(void);
void GoalTracker_SetPointsToday(uint32_t points);
uint32_t GoalTracker_GetPointsToday(void);

/* For Steps */
void GoalTracker_SetGoalInSteps(uint32_t goalToSet);
uint32_t GoalTracker_GetGoalInSteps(void);
void GoalTracker_SetStepsToday(uint32_t steps);
uint32_t GoalTracker_GetStepsToday(void);

/* For Progress */
uint32_t GoalTracker_GetPercentToday(void);

/* To update step point per minute */
void GoalTracker_NotifyNewMinute(uint32_t pointsThisMinute, uint32_t stepsThisMinute);
void GoalTracker_NotifyNewDay(void);


void GoalTracker_AddPoints(uint32_t points);
void GoalTracker_AddSteps(uint32_t steps);

void GoalTracker_GoalMet_Event(void *p_event_data, uint16_t event_size);

//###########################################################################################################
// INTERRUPT SERVICE ROUTINE PROTOTYPES
//###########################################################################################################



//###########################################################################################################
// END OF GoalTracker.h
//###########################################################################################################
#endif
