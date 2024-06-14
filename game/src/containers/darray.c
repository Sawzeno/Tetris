#include  "darray.h"
#include  "core/kmemory.h"
#include  "defines.h"
#include  "core/logger.h"
#include  <stdlib.h>

void* _darrayCreate(u64 cap , u64 stride){
  u64 headersize  = DARRAY_FIELD_LENGTH * sizeof(u64);
  u64 arraysize   = cap * stride;

  UDEBUG("DARRAY_FIELD_LENGTH  : %"PRIu64"",headersize + arraysize); 
  u64* darray   = kallocate(arraysize + headersize , MEMORY_TAG_DARRAY);
  MEMERR(darray);

  darray[DARRAY_CAPACITY] = cap;
  darray[DARRAY_LENGTH]   = 0;
  darray[DARRAY_STRIDE]   = stride;

  u64* arr  = darray + DARRAY_FIELD_LENGTH;
  UDEBUG("CREATED: %p --> %p",darray,arr);
  UDEBUG("CREATED: len  :%"PRIu64" cap : %"PRIu64" stride : %"PRIu64"",DARRAYLENGTH(arr),DARRAYCAPACITY(arr),DARRAYSTRIDE(arr));
  return (void*)(arr);
}

void* _darrayResize(void* array){
  ISNULL(array ,NULL);

  u64 oldlen    = DARRAYLENGTH  (array);
  u64 oldcap    = DARRAYCAPACITY(array);
  u64 oldstride = DARRAYSTRIDE  (array);
  u64 newcap    = oldcap * DARRAY_RESIZE_FACTOR;
  u64* oldhead  = (u64*)array - DARRAY_FIELD_LENGTH;
  UDEBUG("OLD: %p",oldhead);
  //ALLOCATE
  u64 headersize    = DARRAY_FIELD_LENGTH * sizeof(u64);
  u64 newarraysize  = newcap * oldstride;
  UDEBUG("NEW DARRAY_FIELD_LENGTH  : %"PRIu64"",headersize + newarraysize); 
  u64* newhead    = kallocate(headersize + newarraysize, MEMORY_TAG_DARRAY);
  MEMERR(newhead);
  UDEBUG("NEW: %p",newhead);
  //COPY HEADER
  UDEBUG("oldhead contains [0]: %"PRIu64" [1] :%"PRIu64" [2] :%"PRIu64"", *((u64*)oldhead),*((u64*)oldhead+1),*((u64*)oldhead+2));
  kcopyMemory(newhead, oldhead, headersize, __FUNCTION__);
  newhead[DARRAY_CAPACITY]= newcap;
  newhead[DARRAY_LENGTH]  = oldlen;
  newhead[DARRAY_STRIDE]  = oldstride;
  UDEBUG("newhead contains [0]: %"PRIu64" [1] :%"PRIu64" [2] :%"PRIu64"", *((u64*)newhead),*((u64*)newhead+1),*((u64*)newhead+2));
  //COPY BODY
  u64 newbody = (u64)(newhead);
  newbody+= (headersize);
  u64 oldbody = (u64)(array);
  kcopyMemory((void*)newbody, (void*)oldbody, oldlen * oldstride, __FUNCTION__);

  UDEBUG("RESIZE RETURNED : %p", (u64*)oldhead +DARRAY_FIELD_LENGTH);
  return (void*)newbody;
}

void* _darrayPush(void* array , const void* valueptr){
  void* darray = array;
  ISNULL(array , NULL);
  u64 length    =   DARRAYLENGTH  (darray);
  u64 stride    =   DARRAYSTRIDE  (darray);
  u64 cap       =   DARRAYCAPACITY(darray);
  UDEBUG("PUSING IN DARRAY: %p",(u64*)array);
  if (length >= cap) {
    UDEBUG("--------RESIZE CALLED------------");
    darray = _darrayResize(darray);
    UDEBUG("RESIZED DARRAY: %p",darray);
  }
  u64 addr = (u64)darray;
  addr += (length * stride );
  kcopyMemory((void*)addr, valueptr, stride, __FUNCTION__);
  _darrayFieldSet(darray, DARRAY_LENGTH, ++length);

  length    = DARRAYLENGTH  (darray);
  stride    = DARRAYSTRIDE  (darray);
  cap       = DARRAYCAPACITY(darray);

  UDEBUG("len  :%"PRIu64" cap : %"PRIu64" stride : %"PRIu64"",length,cap,stride);
  return darray;
}

u8 _darrayPop(void* array, void* dest) {
  ISNULL(array, false);

  u64 length = DARRAYLENGTH(array);
  u64 stride = DARRAYSTRIDE(array);
  if (length == 0) {
    UERROR("Pop called on empty array!");
    return false;
  }

  u64 addr = (u64)array + ((length - 1) * stride);
  kcopyMemory(dest, (void*)addr, stride, __FUNCTION__);
  _darrayFieldSet(array, DARRAY_LENGTH, length - 1);

  UDEBUG("POPPED: len: %" PRIu64 " cap: %" PRIu64 " stride: %" PRIu64 "", DARRAYLENGTH(array), DARRAYCAPACITY(array), stride);
  return true;
}

void* _darrayPopAt(void* array, u64 index, void* dest) {
  ISNULL(array, NULL);

  u64 length = DARRAYLENGTH(array);
  u64 stride = DARRAYSTRIDE(array);
  if (index >= length) {
    UERROR("Index outside the bounds of this array! Length: %" PRIu64 ", index: %" PRIu64 "", length, index);
    return array;
  }

  u64 addr = (u64)array + (index * stride);
  kcopyMemory(dest, (void*)addr, stride, __FUNCTION__);

  if (index != length - 1) {
    kcopyMemory(
      (void*)(addr),
      (void*)(addr + stride),
      stride * (length - index - 1),
      __FUNCTION__
    );
  }

  _darrayFieldSet(array, DARRAY_LENGTH, length - 1);
  UDEBUG("POPPED AT: len: %" PRIu64 " cap: %" PRIu64 " stride: %" PRIu64 "", DARRAYLENGTH(array), DARRAYCAPACITY(array), stride);
  return array;
}


void* _darrayInsertAt(void* array, u64 index, void* valueptr) {
  ISNULL(array , NULL);
  u64 length    = DARRAYLENGTH  (array);
  u64 stride    = DARRAYSTRIDE  (array);
  u64 capacity  = DARRAYCAPACITY(array);
  if (index >= length) {
    UERROR("Index outside the bounds of this array! Length: %i, index: %index", length, index);
    return array;
  }
  if (length >= capacity) {
    array = _darrayResize(array);
  }
  u64 addr = (u64)array;

  // If not on the last element, copy the rest outward.
  if (index != length - 1) {
    kcopyMemory(
      (void*)(addr + ((index + 1) * stride)),
      (void*)(addr + (index * stride)),
      stride * (length - index), __FUNCTION__);
  }

  // Set the value at the index
  kcopyMemory((void*)(addr + (index * stride)), valueptr, stride, __FUNCTION__);

  _darrayFieldSet(array, DARRAY_LENGTH, length + 1);
UDEBUG("INSERTED AT: len: %" PRIu64 " cap: %" PRIu64 " stride: %" PRIu64 "", DARRAYLENGTH(array), DARRAYCAPACITY(array), stride);
  return array;
}

u8    _darrayDestroy(void* array){
  ISNULL(array , false);
  u64*  header  = (u64*)array - DARRAY_FIELD_LENGTH; 
  free(header);
  UDEBUG("DESTROYED : %p --> %p",header,array);
  return true;
}

u64   _darrayFieldGet(void* array, u64 field) {
  ISNULL(array , 0);
  u64* header = (u64*)array - DARRAY_FIELD_LENGTH;
  return header[field];
}

u8    _darrayFieldSet(void* array, u64 field, u64 value) {
  ISNULL(array , false);
  u64* header = (u64*)array - DARRAY_FIELD_LENGTH;
  header[field] = value;
  return true;
}



