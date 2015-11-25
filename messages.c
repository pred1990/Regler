#pragma once
#include "messages.h"

uint32 msg_type(char* msg){
  if(str_begins_with(msg, "status ")){
    return 1;
  }else if(str_begins_with(msg, "control ")){
    return 2;
  }else if(str_begins_with(msg, "request\n")){
    return 3;
  }else{
    return 0; //undefined
  }
}

bool status_msg_parse(status_msg* status, char* msg){
  //skip type
  int32 index = index_of(msg, ' ');
  if(index < 0){
    return false;
  }
  
  //move to temperature
  msg += index + 1;
  index = index_of(msg, ' ');
  if(index < 0){
    return false;
  }
  
  //read temperature
  char temperature_str[20];   //TODO figure out proper length
  str_cpy_substr(temperature_str, msg, 0, index);
  real64 temperature = atof(temperature_str);
  
  //move to is_on
  msg += index + 1;
  index = index_of(msg, ' ');
  if(index < 0){
    return false;
  }
  
  //read is_on
  char is_on_str[4];
  str_cpy_substr(is_on_str, msg, 0, index);
  bool is_on;
  if(str_begins_with(is_on_str, "ON")){
    is_on = true;
  }else if(str_begins_with(is_on_str, "OFF")){
    is_on = false;
  }else{
    return false;
  }
  
  //move to time
  msg += index + 1;
  index = index_of(msg, '\n');
  if(index < 0){
    return false;
  }
  
  //read time
  char time_str[20];
  str_cpy_substr(time_str, msg, 0, index);
  uint64 time = atoi(time_str);
  //TODO what happens if atoi/atof receives an invalid number? -> use strtol or something
  
  //set values
  status->temperature = temperature;
  status->is_on = is_on;
  status->time = time;
  return true;
}

void status_msg_temperature(status_msg* status, real64 temperature){
  status->temperature = temperature;
}

void status_msg_is_on(status_msg* status, bool is_on){
  status->is_on = is_on;
}

void status_msg_time(status_msg* status, uint64 time){
  status->time = time;
}

void status_msg_write(status_msg* status){
  status->msg_size = sprintf(status->msg, 
                      "status %e %s %" PRIu64 "\n", 
                      status->temperature, 
                      status->is_on ? "ON" : "OFF", 
                      status->time) + 1;
}

void status_msg_cpy(status_msg* tar, status_msg* src){
  str_cpy(tar->msg, src->msg);
  tar->temperature = src->temperature;
  tar->is_on = src->is_on;
  tar->time = src->time;
}

bool control_msg_parse(control_msg* ctrl, char* msg){
  //skip type
  int32 index = index_of(msg, ' ');
  if(index < 0){
    return false;
  }
  
  //move to status
  msg += index + 1;
  index = index_of(msg, '\n');
  if(index < 0){
    return false;
  }
  
  //read status
  char status_str[4];
  str_cpy_substr(status_str, msg, 0, index);
  if(str_begins_with(status_str, "ON")){
    ctrl->set_on = true;
  }else if(str_begins_with(status_str, "OFF")){
    ctrl->set_on = false;
  }else{
    return false;
  }
  
  return true;
}

void control_msg_set(control_msg* ctrl, bool set_on){
  ctrl->set_on = set_on;
  if(set_on){
    str_cpy(ctrl->msg, "control ON\n");
    ctrl->msg_size = 12;
  }else{
    str_cpy(ctrl->msg, "control OFF\n");
    ctrl->msg_size = 13;
  }
}

void request_msg_init(request_msg* req){
  str_cpy(req->msg, "request\n");
  req->msg_size = 9;
}
