#pragma once
#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include "typedefs.h"
#include "string_utils.h"

//status message struct
typedef struct{
  char msg[50];
  uint32 msg_size;
  real64 temperature;
  bool is_on;
  uint64 time;
} status_msg;

//control message struct
typedef struct{
  char msg[13]; // "control off\n"
  uint32 msg_size;
  bool set_on;
} control_msg;

//request message struct
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


//parses a status message
//fills a status_msg struct with data taken from a string
//returns 0 if parsing fails, returns 1 otherwise
bool status_msg_parse(status_msg*, char*);

//generates a status message string
//reads from the status_msg struct's fields
//writes the string into the struct's msg field
//writes the generated string's size into the struct's msg_size field
void status_msg_write(status_msg*);

//copies content of one status_msg struct to another
//does not copy the msg or msg_size fields
void status_msg_cpy(status_msg* tar, status_msg* src);


//parses a control message
//writes to a control_msg struct's set_on field 
//returns 0 if parsing fails, returns 1 otherwise
bool control_msg_parse(control_msg*, char*);

//sets a control_msg struct's is_on field
//and generates a matching message string in the struct's msg field
//writes the generated string's size into the struct's msg_size field
void control_msg_set(control_msg*, bool);


//initializes a request_msg struct
//writes the only apropriate message string to the struct's msg field
//writes the string's size into the struct's msg_size field
void request_msg_init(request_msg*);

#include "messages.c"
