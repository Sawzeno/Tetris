#include  "defines.h"
#include  "core/logger.h"
#include  "containers/darray.h"

int main(void){
  const char* s1  = "HELLO1";
  const char* s2  = "HELLO2";

  const char** strings  = DARRAYCREATE(const char*);
  DARRAYPUSH(strings, s1);

  u64 len = DARRAYLENGTH(strings);
  for(int i = 0 ; i < len ; ++i){
    UDEBUG("%s" , strings[i]);
  }
  return 0;
}
