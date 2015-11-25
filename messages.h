#pragma once
#include <stdio.h>
#include <inttypes.h>
#include "typedefs.h"
#include "string_utils.h"

typedef struct{
  //figure out size/format of sprintf output
  // "status $TEMPERATURE off uint64(20)\n"
  // 6 + 1 + 16 + 1 + 3 + 1 + 20 + 1 + 1 = 50
  char msg[50];
  uint32 msg_size;
  real64 temperature;
  bool is_on;
  uint64 time;
} status_msg;

typedef struct{
  char msg[13]; // "control off\n"
  uint32 msg_size;
  bool set_on;
} control_msg;

typedef struct{
  char msg[9];  // "request\n"
  uint32 msg_size;
} request_msg;

//get message type
//does not check if rest of message is consistent with type
//
//  0 = undefined
//  1 = status
//  2 = control
//  3 = request
uint32 msg_type(char*);

bool status_msg_parse(status_msg*, char*);
void status_msg_temperature(status_msg*, real64);
void status_msg_is_on(status_msg*, bool);
void status_msg_time(status_msg*, uint64);
void status_msg_write(status_msg*);
void status_msg_cpy(status_msg* tar, status_msg* src);

bool control_msg_parse(control_msg*, char*);
void control_msg_set(control_msg*, bool);

void request_msg_init(request_msg*);

#include "messages.c"
