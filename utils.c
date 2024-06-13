#include  <signal.h>
#include  <stdio.h>
#include  <stdlib.h>
#include  <stdarg.h>
#include  <stdbool.h>
#include  <unistd.h>
#include  <time.h>
#include  <sys/types.h>
#include  <sys/stat.h>
#include  <errno.h>

#include  "utils.h"

#define LOG_TYPES  6
#define PREFIX_LEN 24
#define BUFFER_LEN 256
#define SUFFIX_LEN 6
static const char* logLevels[LOG_TYPES]  = 
  { "[FATAL]:",
    "[ERROR]:",
    "[WARN] :",
    "[INFO] :",
    "[DEBUG]:",
    "[TRACE]:"
  };

static const char* logColors[LOG_TYPES + 1] =
  {
    "\x1b[95m",
    "\x1b[91m",
    "\x1b[93m",
    "\x1b[90m",
    "\x1b[92m",
    "\x1b[97m",
    "\x1b[0m\n"
  };


#define UBUFF_CHECK(result, limit)                    \
if(result >= limit){                                  \
  fprintf(stderr, "result size exceeded limit\n");    \
  return result;                                      \
}else if( result < 0){                                \
  fprintf(stderr, "error in %s\n", __FUNCTION__);     \
  return result;                                      \
}

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

void  initLogging (void){
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
}

Unode*  createUnode(void* item, Unode* next) {
  Unode* unode = calloc(1 , sizeof(Unode)); 
  MEMERR(unode);
  unode->item = item; 
  unode->next = next;
  return unode;
}

Unode*  addUnode(Unode* prev, Unode* newUnode){
  ISNULL(prev , NULL);
  prev->next  = newUnode;
  return newUnode != NULL ? newUnode : prev;
}

Unode*  reachUnode(Unode* head, Unode* dest){
  ISNULL(head, NULL);
  Unode* temp = head;
  while(temp->next != dest && temp->next != NULL){
    temp  = temp->next;
  }
  return temp;
}

Unode*  freeUnode(Unode* node){
  ISNULL(node, NULL);
  Unode* temp = node->next;
  free(node);
  return temp;
}

Ulist*  createUlist(void){
  Ulist* ulist  = calloc(1 , sizeof(Ulist));
  MEMERR(ulist);
  Unode* dummy  = createUnode(NULL, NULL);
  ulist->head = ulist->tail = dummy;
  return ulist;
}

u64 freeUlist(Ulist* ulist) {
  u64 items = 0;
  ISNULL(ulist , 0);
  Unode* curr = ulist->head;
  while (curr != NULL) {
    curr = freeUnode(curr);
    ++items;
  }
  free(ulist);
  return items;
}

void elapsed(struct timespec* start, const char* func){
  struct timespec end;
  clock_gettime(CLOCK_MONOTONIC, &end);

  f64 elapsed = (end.tv_sec - start->tv_sec) + (end.tv_nsec - start->tv_nsec)/1e9;
  UDEBUG("%-10s : %.9f",func,elapsed);
  *start  = end;
}

#define RETURN_DEFER(value) do { result = (value); goto defer; } while(0)

char* Ushift_args(int* argc, char*** argv){

  UASSERT(*argc > 0,NULL, "no arguments");
  char* arg = **argv;
  ++*argv;
  --*argc;
  return arg;
}

bool UdirExists(const char* path){
  struct stat stats;
  stat(path, &stats);

  return S_ISDIR(stats.st_mode) ? true : false;
}

bool Umkdir(const char* path){

  if(UdirExists(path)){
    UINFO("'%s' exists", path);
  }else{
    UWARN("'%s' does not exists, making one ...",path);
    if(mkdir(path, S_IFDIR) < 0){
      UERROR("'%s' could not be made %s",path, strerror(errno));
    }else{
      UINFO("'%s' has been made!", path);
    }
  } 

  return UdirExists(path) ? true : false; 
}

u8* UreadFile(const char* path) {

  bool result = true;
  FILE* file = fopen(path, "rb");

  if(file == NULL)              RETURN_DEFER(false);
  if(fseek(file, 0, SEEK_END))  RETURN_DEFER(false);
  u64 fileSize = ftell(file);
  if(fileSize < 0)              RETURN_DEFER(false);
  if(fseek(file, 0, SEEK_SET))  RETURN_DEFER(false);
  u8* fileBuffer = calloc(fileSize + 1, sizeof(u8)); 
  MEMERR(fileBuffer);

  u64 bytesRead = fread(fileBuffer, sizeof(u8), fileSize, file);
  if (bytesRead != fileSize) {
    free(fileBuffer);
    UERROR("error reading file");
    exit(EXIT_FAILURE);
  }

  fileBuffer[fileSize] = '\0'; 

defer:
  if(!result) UFATAL("COULD NOT READ FILE %s: %s",path, strerror(errno));
  if(file)  fclose(file);
  return fileBuffer;
}

bool UwriteFile(const char* path, const void* data, u64 size){

  bool result = true;
  FILE* file = fopen(path, "wb");
  if(file == NULL){
    UFATAL("COULD NOT OPEN FILE %s: %s",path, strerror(errno));
    RETURN_DEFER(false);
  }
  const char* buf = data;
  while(size > 0){
    u64 n = fwrite(buf, 1, size, file);
    if(ferror(file)){
      UFATAL("COULD NOT WRITE TO FILE %s: %s",path, strerror(errno));
      RETURN_DEFER(false);     
    }
    size -= n;
    buf  += n;
  }
defer:
  if(file) fclose(file);
  return result;
}


void writeint(i64 integer) {
  bool isNegative = false;
  if (integer < 0) {
    isNegative = true;
    integer = -integer;
  }

  u64 _int = (u64)integer;

  u8 string[24];
  u8 count = 0;
  u8 rem = 0;

  do {
    rem = (u8)(_int % 10);
    string[count] = rem + '0';
    _int /= 10;
    ++count;
  } while (_int != 0);

  if (isNegative) {
    putchar('-');
  }

  for (size_t i = count; i > 0; --i) {
    putchar(string[i - 1]);
  }
  putchar('\n');
}


