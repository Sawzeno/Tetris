#pragma once

#include  <stdio.h>
#include  <string.h>
#include  <stdint.h>
#include  <stdlib.h>
#include  <signal.h>
#include  <unistd.h>
#include  <stdbool.h>
#include  <time.h>
#include  <inttypes.h>
#include  <execinfo.h>
#include  <dlfcn.h>
#include  <error.h>

typedef enum log_level {
  LOG_FATAL = 0,
  LOG_ERROR = 1,
  LOG_WARN = 2,
  LOG_INFO = 3,
  LOG_DEBUG = 4,
  LOG_TRACE = 5,
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
#define WATCH(X) elapsed(&start, #X)

typedef uint64_t  u64;
typedef uint32_t  u32;
typedef uint16_t  u16;
typedef uint8_t   u8;

typedef int64_t   i64;
typedef int32_t   i32;
typedef int16_t   i16;
typedef int8_t    i8;

typedef float     f32;
typedef double    f64;

typedef struct Unode Unode;
typedef struct Ulist Ulist;
typedef struct Uinfo Uinfo;

struct  Uinfo{
  const char* str;
  bool        e;
};

struct  Unode{
  void*   item;
  Unode*  next;
};

struct  Ulist{
  Unode* head;
  Unode* tail;
};

void    initLogging (void);
void    sendSignal  ( int sig, const char* str, bool e);
u16     Ulog        (log_level level , const char* message , ...);
u16     Uwrite      (u16 limit, const char *format , ...); 
void    elapsed     (struct timespec* start, const char* func);

char*   Ushift_args   (int* argc, char*** argv);
bool    UdirExists  (const char* path);
bool    Umkdir      (const char* path);
u8*     UreadFile   (const char* path);
bool    UwriteFile  (const char* path, const void* data, u64 size);
void    writeint    (i64 integer);

Unode*  createUnode (void* item,  Unode* next);
Unode*  addUnode    (Unode* prev, Unode* newUnode);
Unode*  reachUnode  (Unode* head, Unode* dest);
Unode*  freeUnode   (Unode* node);

Ulist*  createUlist (void);
u64     freeUlist   (Ulist* ulist);


#define PUSHUNODE(ulist, newUnode)\
ulist->tail = addUnode(ulist->tail, newUnode) 

#define POPUNODE(ulist)\
ulist->tail = reachUnode(ulist->head, ulist->tail)

#define INSERTUNODE(prev, newUnode, next)\
addUnode(addUnode(prev, newUnode), next)


void initLogging(void) __attribute__((constructor));
