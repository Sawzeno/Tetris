#include  "defines.h"
#include  "logger.h"

#include  <signal.h>
#include  <execinfo.h>
#include  <dlfcn.h>
#include  <error.h>
#include  <time.h>

#define LOG_TYPES  7
#define PREFIX_LEN 24
#define BUFFER_LEN 256
#define SUFFIX_LEN 6

#define UBUFF_CHECK(result, limit)                    \
if(result >= limit){                                  \
  fprintf(stderr, "result size exceeded limit\n");    \
  return result;                                      \
}else if( result < 0){                                \
  fprintf(stderr, "error in %s\n", __FUNCTION__);     \
  return result;                                      \
}

typedef struct LoggerSystemState{

    bool initialzed;
}LoggerSystemState;

static const char* logLevels[LOG_TYPES]  = 
  { "[FATAL]:",
    "[ERROR]:",
    "[WARN] :",
    "[INFO] :",
    "[DEBUG]:",
    "[TRACE]:",
    "[TEST] :"
  };

static const char* logColors[LOG_TYPES + 1] =
  {
    "\x1b[95m",
    "\x1b[91m",
    "\x1b[93m",
    "\x1b[90m",
    "\x1b[92m",
    "\x1b[97m",
    "\x1b[94m",
    "\x1b[0m\n"
  };

u16   Ulog(log_level level , const char* message , ...){
  char outmsg[PREFIX_LEN + BUFFER_LEN + SUFFIX_LEN];
  u16   prefixlen = snprintf  (outmsg,
                               PREFIX_LEN,"%s%s%2s",logColors[level], logLevels[level],"");
  UBUFF_CHECK(prefixlen, PREFIX_LEN);
  va_list args;
  va_start(args , message);
  u16   bufferlen    = vsnprintf (outmsg + prefixlen,
                                  BUFFER_LEN, message , args);
  va_end(args);
  UBUFF_CHECK(bufferlen, BUFFER_LEN);
  u16   suffixlen    = snprintf  (outmsg + prefixlen + bufferlen,
                                  SUFFIX_LEN, "%s", logColors[LOG_TYPES]);
  UBUFF_CHECK(suffixlen, SUFFIX_LEN);
  u16 totallen = prefixlen + bufferlen + suffixlen;
  if(write(STDOUT_FILENO, outmsg,totallen ) < 0){
    fprintf(stderr, "ULog failed with %s\n", strerror(errno));
  }

  return bufferlen ;
}

u16   Uwrite(u16 limit, const char *message, ...) {
  va_list args;
  va_start(args, message);
  char buffer[limit];
  u64 size  = vsnprintf(buffer,limit, message, args);
  UBUFF_CHECK(size, limit);
  va_end(args);

  if(write(STDOUT_FILENO, buffer, size) < 0){
    fprintf(stderr," Uwrite FAILED WITH %s\n", strerror(errno));
    return 0;
  }; 
  return size;
}

void sendSignal( int sig, const char* str, bool e){
  union sigval sv;

  Uinfo* info = calloc(1, sizeof(Uinfo));
  MEMERR(info);
  info->str  = str;
  info->e    = e;
  sv.sival_ptr  = (void*)info;

  if (sigqueue(getpid(), sig, sv) == -1) {
    UFATAL("SIGQUEUE");
  }
}

void handle_sigill(int sig, siginfo_t* info, void* context){
  (void)context;
  if(sig  ==  SIGILL){
    if(info == NULL){
      UFATAL("SIGILL raised");
      exit(SIGILL);
    }else{
      Uinfo* uinfo  = (Uinfo*)(info->si_value.sival_ptr);

      UFATAL("SIGILL raised : %s", uinfo->str);
      if(uinfo->e){
        UFATAL("EXITING!");
        exit(SIGILL);
      }else{
        return;
      }
    }
  }
}

void handle_sigint(int sig){
  if(sig == SIGINT){
    UFATAL("SIGINT, TERMINATING");
    exit(SIGINT);
  }
}

void handle_sigsegv(int sig) {
  if (sig == SIGSEGV) {
    void* buffer[100];
    size_t size = backtrace(buffer, 100);
    char** symbols = backtrace_symbols(buffer, size);

    if (symbols != NULL) {
      for (size_t i = 0; i < size; i++) {
        UERROR("Backtrace: %s", symbols[i]);
      }
      free(symbols);
    } else {
      UERROR("Could not backtrace symbols");
    }

    UFATAL("SIGSEGV: TERMINATING...");
    exit(SIGSEGV);
  }
}

static LoggerSystemState* loggerStatePtr;

bool  initializeLogging (u64* memoryRequirement, void* state){

  *memoryRequirement = sizeof(LoggerSystemState);
  if(state  == NULL){
    return true;
  }

  loggerStatePtr  = state;
  loggerStatePtr->initialzed  = true;

  stack_t signalStack;
  signalStack.ss_sp = calloc(1,SIGSTKSZ);
  MEMERR(signalStack.ss_sp);
  signalStack.ss_size = SIGSTKSZ;
  signalStack.ss_flags= SS_ONSTACK;

  if(sigaltstack(&signalStack, NULL) == -1){
    UERROR("COULD NOT INITIALIZE SIGNALSTACK !");
    exit(EXIT_FAILURE);
  }

  struct sigaction sigactINT;
  sigemptyset(&sigactINT.sa_mask);
  sigactINT.sa_handler= handle_sigint;
  sigactINT.sa_flags = SA_ONSTACK;

  struct sigaction sigactSEGV;
  sigemptyset(&sigactSEGV.sa_mask);
  sigactSEGV.sa_handler= handle_sigsegv;
  sigactSEGV.sa_flags = SA_ONSTACK;

  struct sigaction sigactILL;
  sigemptyset(&sigactILL.sa_mask);
  sigactILL.sa_sigaction = handle_sigill;
  sigactILL.sa_flags  = SA_SIGINFO ;

  if( 
    sigaction(SIGINT, &sigactINT, NULL) == -1 ||
    sigaction(SIGSEGV,&sigactSEGV,NULL) == -1 ||
    sigaction(SIGILL, &sigactILL, NULL) == -1
  ){
    UERROR("COULD NOT SET UP SIGNAL HANDLERS");
    exit(EXIT_FAILURE);
  }
  return true;
}

void shutdownLogging(void* state){
  UINFO("LOGGING SYSTEM SHUTDOWN");
    state = NULL; 
}


void elapsed(struct timespec* start, const char* func){
  struct timespec end;
  clock_gettime(CLOCK_MONOTONIC, &end);

  f64 elapsed = (end.tv_sec - start->tv_sec) + (end.tv_nsec - start->tv_nsec)/1e9;
  UTRACE("%-30s : %.9f",func,elapsed);
  *start  = end;
}
