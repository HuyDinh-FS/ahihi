//
//  TapConfig.h
//  tapdetect
//
//  Created by james ochs on 5/4/15.
//  Copyright (c) 2015 Misfit. All rights reserved.
//

#ifndef tapdetect_TapConfig_h
#define tapdetect_TapConfig_h

//These need to be device in the build configuration settings for a given project
#define _TD_ACCEL_ADXL 1   //For Pluto we only plan to use the ADXL.  This will need to be changed 
                          //based on product rev if we ever use other accelerometers with this repo.





#ifdef _TD_ACCEL_LIS2DH
#define IS_MOTION_THRESH 300
#define UP_SLOPE_THRESH 1400
#define IS_MOTION_2_THRESH 1600

#endif

#ifdef _TD_ACCEL_ADXL
#define IS_MOTION_THRESH 200
#define UP_SLOPE_THRESH 900
#define IS_MOTION_2_THRESH 1100
#endif


#endif
