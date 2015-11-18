#pragma once
#include <stdint.h>

typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef float real32;
typedef double real64;

#define false 0
#define true  1

typedef int32 bool;

typedef struct{
  uint32 port;
  char ip[16];
} config;

typedef struct{
  char* msg;
  real32 temperature; //or 64?
  bool is_on;
  uint64 time;
} status;

typedef struct{
  char* msg;
  bool is_on;
} control;

typedef struct{
  char* msg;
} request;

