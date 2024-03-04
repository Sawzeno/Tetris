#include<stdio.h>
#include<inttypes.h>
#include"config.h"

int main(){

  initConfig();
  printf("RESX : %"PRIu64"\n", config.RESX);
  return 0;
}
