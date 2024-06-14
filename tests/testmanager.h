#include "defines.h"
#include  "core/logger.h"
#define BYPASS 2

typedef u8 (*PFN_TEST)();

#define EXPECTED_AS(expected, actual)                                                              \
if(actual != expected){                                                                            \
  UERROR("expected %lld , got %lld \nFile : %s, Line : %d", expected, actual, __FILE__, __LINE__); \
  return false;                                                                                    \
}

#define NOT_EXPECTED_AS(expected, actual)                                                         \
if(actual == expected){                                                                           \
  UERROR("%lld != %lld but equal \nFile : %s, Line : %d", expected, actual, __FILE__, __LINE__);  \
  return false;                                                                                   \
}

#define EXPECTED_FLOAT_AS(expected, actual)                                                       \
if(abs(expected - actual) > 0.001f){                                                              \
  UERROR("expected %f as %f but not, File : %s, Line : %d", expected, actual, __FILE__, __LINE__);\
  return false;                                                                                   \
}

#define IS_TRUE(actual)                                                                           \
if(actual != true){                                                                               \
  UERROR("expected as true but got false at %s : %d",__FUNCTION__, __LINE__);                     \
  return false;                                                                                   \
}

#define IS_FALSE(actual)                                                                          \
if(actual != false){                                                                              \
  UERROR("expected as true but got false at %s : %d",__FUNCTION__, __LINE__);                     \
  return false;                                                                                   \
}

void testManagerInit();
void testManagerRegisterTest(PFN_TEST, char* desc);
void testManagerRunTests();
