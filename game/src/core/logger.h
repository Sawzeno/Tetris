#pragma once

#include "defines.h"

typedef enum log_level {
  LOG_FATAL = 0,
  LOG_ERROR = 1,
  LOG_WARN  = 2,
  LOG_INFO  = 3,
  LOG_DEBUG = 4,
  LOG_TRACE = 5,
  LOG_TEST  = 6,
} log_level;

#ifndef LOG_WARN_ENABLED
#define LOG_WARN_ENABLED 1
#endif

#ifndef LOG_INFO_ENABLED
#define LOG_INFO_ENABLED  1
#endif

#ifndef LOG_TRACE_ENABLED
#define LOG_TRACE_ENABLED 1
#endif

#ifndef LOG_DEBUG_ENABLED
#define LOG_DEBUG_ENABLED 1
#endif

#ifndef LOG_TEST_ENABLED
#define LOG_TEST_ENABLED 1
#endif


#define UREPORT(X) Ulog(LOG_ERROR, "%s  func : %s   file : %s line : %d", #X, __FUNCTION__, __FILE__, __LINE__);

#define UFATAL(fmt, ...) Ulog(LOG_FATAL, fmt, ##__VA_ARGS__)
#define UERROR(fmt, ...) Ulog(LOG_ERROR, fmt, ##__VA_ARGS__)

#if LOG_WARN_ENABLED == 1
#define UWARN(fmt, ...)  Ulog(LOG_WARN , fmt, ##__VA_ARGS__)
#else
#define UWARN(fmt, ...)
#endif

#if LOG_INFO_ENABLED == 1
#define UINFO(fmt, ...)  Ulog(LOG_INFO , fmt, ##__VA_ARGS__) 
#else
#define UINFO(fmt, ...)
#endif

#if LOG_DEBUG_ENABLED == 1
#define UDEBUG(fmt, ...) Ulog(LOG_DEBUG, fmt, ##__VA_ARGS__)
#else
#define UDEBUG(fmt, ...)
#endif

#if LOG_TRACE_ENABLED == 1
#define UTRACE(fmt, ...) Ulog(LOG_TRACE, fmt, ##__VA_ARGS__)
#else
#define UTRACE(fmt, ...)
#endif

#if LOG_TEST_ENABLED == 1
#define UTEST(fmt, ...) Ulog(LOG_TEST, fmt, ##__VA_ARGS__)
#else
#define UTEST(fmt, ...)
#endif


#define UASSERTIONS_ENABLED

#ifdef UASSERTIONS_ENABLED
#define UASSERT(expr, ret, info) if (!(expr)) { UREPORT(expr); UERROR(info);return ret; }
#else
#define UASSERT(expr)
#endif

#define MEMERR(X) if (X == NULL) { UERROR("%s allocation failed", #X); exit(EXIT_FAILURE); }
#define ISNULL(X, Y) if (X == NULL) { UWARN("%s is NULL", #X); return Y; }
#define UWRITE(fmt, ...) Uwrite(1024, fmt, ##__VA_ARGS__)

#define INITCLOCK   struct timespec start; clock_gettime(CLOCK_MONOTONIC, &start) 
#define WATCH(X) elapsed(&start, X)


typedef struct Uinfo Uinfo;

struct  Uinfo{
  const char* str;
  bool        e;
};

// Initializes logging system , call twice once with state = 0 , to get memory size , and then to pass allocated memory to the state;
//
bool    initializeLogging (u64* memoryRequirement, void* state);
void    shutdownLogging   (void* state);
void    sendSignal        ( int sig, const char* str, bool e);
u16     Ulog              (log_level level , const char* message , ...);
u16     Uwrite            (u16 limit, const char *format , ...); 
void    elapsed           (struct timespec* start, const char* func);
