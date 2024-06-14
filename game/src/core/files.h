#include  "defines.h"
#include  "logger.h"

#include  <sys/types.h>
#include  <sys/stat.h>

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


