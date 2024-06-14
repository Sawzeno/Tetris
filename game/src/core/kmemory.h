#pragma once

#include  "defines.h"

#define MEMORY_TAG_MAX_TAGS 18

typedef enum Memory_tag{
  MEMORY_TAG_UNKNOWN,
  MEMORY_TAG_ARRAY,
  MEMORY_TAG_DARRAY,
  MEMORY_TAG_DICT,
  MEMORY_TAG_RING_QUEUE,
  MEMORY_TAG_BST,
  MEMORY_TAG_STRING,
  MEMORY_TAG_APPLICATION,
  MEMORY_TAG_JOB,
  MEMORY_TAG_TEXTURE,
  MEMORY_TAG_MATERIAL_INSTANCE,
  MEMORY_TAG_RENDERER,
  MEMORY_TAG_GAME,
  MEMORY_TAG_TRANSFORM,
  MEMORY_TAG_ENTITY,
  MEMORY_TAG_ENTITY_NODE,
  MEMORY_TAG_ENTITY_SCENE,
  MEMORY_TAG_LINEAR_ALLOCATOR,
}Memory_tag;


void  initializeMemory(u64*  memoryRequirement, void* state);
void  shutdownMemory(void* state);

void* kallocate(u64 size , Memory_tag tag);

void  kfree(void* block , u64 size , Memory_tag tag);

void* kzeroMemory(void* block , u64 size);

void* kcopyMemory(void* dest , const void* source , u64 size, const char* func);

void* ksetMemory(void* dest , i32 value , u64 size);

char* getMemoryUsage();

u64   getMemoryAllocCount();
