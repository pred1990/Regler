#pragma once
#include <time.h>
#include "typedefs.h"

uint64 time_to_nsec(struct timespec*);
real64 nsec_to_sec(uint64);
uint64 sec_to_nsec(real64);
uint64 msec_to_nsec(uint64);

#include "time_utils.c"