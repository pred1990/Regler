#pragma once
#include "messages.h"

void status_parse(status*, char*){
  //TODO parse
  char delimiter = ' ';
  //int end = index_of();
}

void control_on(control*){
  ctrl_set(true);
}

void control_off(control* ctrl){
  ctrl_set(false);
}

void control_set(control* ctrl, bool is_on){
  ctrl->is_on = is_on;
  str_cpy(ctrl->msg, is_on ? "ON\n" : "OFF\n");
}

void request(request* req){
  str_cpy(req->msg, "request\n");
}