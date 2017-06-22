/* 
  copyright Misfit Wearables 2012

  typedef.h

  Created by Matt Montgomery on 2/8/2013

  efm32 specific typedefs

*/

#ifndef _typedef_h
#define _typedef_h

#include <stdint.h>
#include <stdbool.h>


// Regular integer data types
typedef uint8_t     u8;
typedef uint16_t    u16;
typedef uint32_t    u32;
typedef uint64_t    u64;

typedef int8_t      s8;
typedef int16_t     s16;
typedef int32_t     s32;
typedef int64_t     s64;

// Fast integer data types
typedef uint_fast8_t        u8_fast;
typedef uint_fast16_t       u16_fast;
typedef uint_fast32_t       u32_fast;

typedef int_fast8_t         s8_fast;
typedef int_fast16_t        s16_fast;
typedef int_fast32_t        s32_fast;

// Least integer data types
typedef uint_least8_t        u8_least;
typedef uint_least16_t       u16_least;
typedef uint_least32_t       u32_least;

typedef int_least8_t         s8_least;
typedef int_least16_t        s16_least;
typedef int_least32_t        s32_least;

/*
 typedef unsigned char       u8;
 typedef unsigned short      u16;
 typedef unsigned long       u32;
 typedef unsigned long long  u64;
 
 typedef signed char         s8;
 typedef short               s16;
 typedef long                s32;
 typedef long long           s64;
 
 
 typedef uint_fast8_t        u8_fast;
 typedef uint_fast16_t       u16_fast;
 typedef uint_fast32_t       u32_fast;
 
 typedef int_fast8_t         s8_fast;
 typedef int_fast16_t        s16_fast;
 typedef int_fast32_t        s32_fast;
 */

#endif //_typedef_h
