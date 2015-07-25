/*
******************************************************************************
* @file     ac_common.h
* @authors  cxy
* @version  V1.0.0
* @date     10-Sep-2014
* @brief    
******************************************************************************
*/

#ifndef  __AC_COMMON_H__ 
#define  __AC_COMMON_H__

#include <string.h>

typedef signed   char                       s8;                                 
typedef signed   short                      s16;                                
typedef signed   int                        s32;                                
typedef unsigned char                       u8;                                 
typedef unsigned short                      u16;                                
typedef unsigned long                       u32; 
typedef unsigned long long                  u64;   
typedef volatile signed char                vs8;                                
typedef volatile signed short               vs16;                               
typedef volatile signed int                 vs32;                               
typedef volatile unsigned char              vu8;                                
typedef volatile unsigned short             vu16;                               
typedef volatile unsigned int               vu32;      


#define  AC_RET_OK                          (0)
#define  AC_RET_ERROR                       (-1)

#define  AC_MESSAGE_MAX_LEN            (200)

#define RT_DEBUG_OFF        0
#define RT_DEBUG_ERROR      1
#define RT_DEBUG_TRACE      2
#define RT_DEBUG_INFO       3

#ifdef RT_DEBUG_SUPPORT
extern u8 RTDebugLevel;
#define DBGPRINT(Level, Fmt)    	\
{                                   \
    if ((Level) <= RTDebugLevel)    \
    {                               \
        UARTprintf Fmt;			    \
    }                               \
}
#else
#define DBGPRINT(Level, Fmt)
#endif

#ifndef NULL
#define NULL                ((void *)0)
#endif
u64 AC_HtoNlu64(u64 n);
u32 AC_HtoNl(u32 h);
u32 AC_NtoHl(u32 n);
u16 AC_HtoNs(u16 h);
u16 AC_NtoHs(u16 n);
#define AC_HTONS(n) AC_HtoNs(n)
#define AC_HTONL(n) AC_HtoNl(n)
#define AC_HTONLU64(n) AC_HtoNlu64(n)


#endif
/******************************* FILE END ***********************************/

