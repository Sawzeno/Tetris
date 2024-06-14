#include "testmanager.h"

#include  "defines.h"
#include  "time.h"
#include  "core/logger.h"
#include  "containers/darray.h"

typedef struct TestEntry{
  PFN_TEST func;
  char* desc;
}TestEntry;

static TestEntry* tests;

void testManagerInit(){
  tests  = DARRAYCREATE(TestEntry);
}

void testManagerRegisterTest(u8(*PFN_TEST)(), char* desc){
  TestEntry e;
  e.func  = PFN_TEST;
  e.desc  = desc;
  DARRAYPUSH(tests, e);
}

void testManagerRunTests(){
  u32 passed  = 0;
  u32 failed  = 0;
  u32 skipped = 0;

  u32 count = DARRAYLENGTH(tests);
  INITCLOCK;

  for(u32 i = 0; i < count ; ++i){
    UTEST("RUNNING TEST %d of %d", i, count);
    u8 result = tests[i].func();
    if(result == true){
      ++passed;
    }else if(result == BYPASS){
      UWARN("[SKIPPED] : %s", tests[i].desc);
      ++skipped;
    }else{
      UERROR("[FAILED] : %s", tests[i].desc);
      ++failed;
    }
    WATCH(tests[i].desc);
  }
  UTEST("BYPASSED : %d, PASSED : %d, FAILED : %d", skipped, passed, failed);
}
