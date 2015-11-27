#pragma once
#include "time_utils.h"

uint64 time_to_nsec(struct timespec* time){
  return time->tv_sec * 1000000000 + time->tv_nsec;
}

real64 nsec_to_sec(uint64 nsec){
  return nsec / 1000000000.0;
}

uint64 sec_to_nsec(real64 sec){
  return sec * 1000000000.0;
}

uint64 msec_to_nsec(uint64 msec){
  return msec * 1000000;
}