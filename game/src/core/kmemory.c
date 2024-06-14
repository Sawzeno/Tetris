#include "kmemory.h"
#include "defines.h"

#include  "logger.h"
#include <stdio.h>
#include  <stdlib.h>
#include  <unistd.h>
#include  <string.h>

typedef struct MemoryStats MemoryStats;

struct MemoryStats {
  u64 totalAllocated;
  u64 taggedAllocations[MEMORY_TAG_MAX_TAGS];
};

static const char* memory_tag_strings[MEMORY_TAG_MAX_TAGS] = {
  "UNKNOWN",
  "ARRAY",
  "DARRAY",
  "DICT",
  "RING_QUEUE",
  "BST",
  "STRING",
  "APPLICATION",
  "JOB",
  "TEXTURE",
  "MATERIAL_INSTANCE",
  "RENDERER",
  "GAME",
  "TRANSFORM",
  "ENTITY",
  "ENTITY_NODE",
  "ENTITY_SCENE",
  "LINEAR_ALLOCATOR"
};


#define MEM_STATS_LEN 1024


typedef struct MemorySystemStats{
  MemoryStats stats;
  u64         allocCount;
}MemorySystemStats;

static MemorySystemStats* memorySystemStatePtr;

void initializeMemory(u64*  memoryRequirement, void* state){
  *memoryRequirement = sizeof(MemorySystemStats);
  if(state == NULL){
    return ;
  }
  memorySystemStatePtr  = state;
  kzeroMemory(&memorySystemStatePtr->stats,sizeof(memorySystemStatePtr->stats));
  memorySystemStatePtr->allocCount  = 0;
  UINFO("MEMORY SUSBSYSTEM INITIALIZED");
}

void shutdownMemory(void* state){
  UINFO("MEMORY SYSTEM SHUTDOWN");
  memorySystemStatePtr  = NULL;
}

u64 getMemoryAllocCount(){
  if(memorySystemStatePtr){
    return memorySystemStatePtr->allocCount;
  }else{
    return 0;
  }
}

char* getMemoryUsage(){
  const u64 gib   = 1024 * 1024 * 1024;
  const u64 mib   = 1024 * 1024;
  const u64 kib   = 1024;

  char buffer[MEM_STATS_LEN]   = "System Memory Use(TAGGED):\n";
  u64 offset  = strlen(buffer);

  for(u32 i = 0 ; i < MEMORY_TAG_MAX_TAGS  ; ++i){
    char unit[4]  = "Xb";
    float amount  = 1.0f;

    if(memorySystemStatePtr->stats.taggedAllocations[i] >= gib){
      unit[0] = 'G';
      amount  = memorySystemStatePtr->stats.taggedAllocations[i]/(float)gib;
    }else if(memorySystemStatePtr->stats.taggedAllocations[i] >= mib){
      unit[0] = 'M';
      amount  = memorySystemStatePtr->stats.taggedAllocations[i]/(float)mib;
    }else if(memorySystemStatePtr->stats.taggedAllocations[i] >= kib){
      unit[0] = 'K';
      amount  = memorySystemStatePtr->stats.taggedAllocations[i]/(float)kib;
    }else{
      unit[0] = 'b';
      unit[1] = 0;
      amount  = memorySystemStatePtr->stats.taggedAllocations[i];
    } 
    i32 length = snprintf(buffer + offset, MEM_STATS_LEN - offset, "%-20s: %.2f%s\n", memory_tag_strings[i], amount, unit);
    offset+= length;
  }

  char* outString = strdup(buffer);
  return outString;
}

void* kallocate(u64 size , Memory_tag tag){
  if(tag ==  MEMORY_TAG_UNKNOWN){
    UWARN("kallocate called on MEMORY_TAG_UNKNOWN , reclass this allocation !");
  }
  void* block = calloc(1, size);
  if(block == NULL ){
    exit(1);
  }
  if(memorySystemStatePtr){
    memorySystemStatePtr->stats.totalAllocated  +=  size;
    memorySystemStatePtr->stats.taggedAllocations[tag]  += size;
    ++memorySystemStatePtr->allocCount;
  }

  return block;
}

void  kfree(void* block , u64 size , Memory_tag tag){
  if(tag  == MEMORY_TAG_UNKNOWN){
    UWARN("kfree called on MEMORY_TAG_UNKOWN");
  }
  memorySystemStatePtr->stats.totalAllocated  -=  size;
  memorySystemStatePtr->stats.taggedAllocations[tag]  -=  size;
  free(block);
}

void* kzeroMemory(void* block , u64 size){
  return memset(block , 0 , size);
}

void* kcopyMemory(void* dest , const void* source , u64 size, const char* func){
  //UDEBUG("copying %p to %p in function %s",source,dest,func);
  void* copied = memcpy(dest , source , size);

  return copied ;
}

void* ksetMemory(void* dest , i32 value , u64 size){
  return memset(dest , value , size);
}

