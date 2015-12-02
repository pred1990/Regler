#pragma once
#include <time.h>
#include "typedefs.h"

//converts time from a timespec into 64bit uint nanoseconds
uint64 time_to_nsec(struct timespec*);

//converts 64bit uint nanoseconds to 64bit floating point seconds
real64 nsec_to_sec(uint64);

//converts 64bit floating point seconds to 64bit uint nanoseconds
uint64 sec_to_nsec(real64);

//converts 64bit uint milliseconds to 64bit uint nanoseconds
uint64 msec_to_nsec(uint64);

#include "time_utils.c"