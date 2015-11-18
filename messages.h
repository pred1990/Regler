#pragma once
#include "typedefs.h"
#include "strings.h"

void status_parse(status*, char*);

void control_on(control*);
void control_off(control*);
void control_set(control*, bool);

void request(request*);

#include "messages.c"
