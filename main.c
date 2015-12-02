#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "typedefs.h"
#include "network.h"
#include "string_utils.h"
#include "messages.h"
#include "time_utils.h"

typedef struct{
  uint32 port;
  char ip[16];
  real64 target_temp;
  real64 target_range;
} config;

typedef struct{
  real64 temp_low;
  real64 temp;
  real64 temp_high;
} temp_range;

typedef struct{
  uint64 time_min;
  uint64 time;
  uint64 time_max;
} time_constrained;

typedef struct{
  bool is_on;
} control_state;

//interprets key-value pairs from console parameters
//pairs should be passed as "key=value" without quotes
//expected keys are: port, ip, target, range
void interpret_all(config*, int32, char**);

//applies time constraints
//returns the struct's time if it is within the defined bounds
//else returns the appropriate bound
uint64 get_constrained_time(time_constrained*);

int32 main(int32 argL, char** argV){
  config cfg;

  //set default config values
  cfg.port = 4242;
  str_cpy(cfg.ip, "127.0.0.1");
  cfg.target_temp = 90.0;
  cfg.target_range = 1.0;

  //grab available config values from parameters
  interpret_all(&cfg, argL, argV);

  //target temperature range
  real64 temp_range_factor = 0.65;
  temp_range temp_target = {};
  temp_target.temp = cfg.target_temp;
  temp_target.temp_low = temp_target.temp - cfg.target_range * temp_range_factor;
  temp_target.temp_high = temp_target.temp + cfg.target_range * temp_range_factor;

  //timers structs
  struct timespec time_get = {};
  struct timespec time_sleep = {};
  time_sleep.tv_nsec = msec_to_nsec(10);  //TODO use timeout-blocking poll on socket instead

  //timer variables for next request
  uint64 t_recv_latest = 0;
  time_constrained t_send_next = {};
  t_send_next.time_min = msec_to_nsec(10);
  t_send_next.time_max = msec_to_nsec(500);
  real64 t_send_factor = 0.3;

  //prepare request message
  request_msg request = {};
  request_msg_init(&request);

  //prepare control messages
  control_msg control_off = {};
  control_msg control_on = {};
  control_msg_set(&control_off, false);
  control_msg_set(&control_on, true);
  //bool set_on_latest = false;
  control_state ctrl_state = {};

  //status message containers
  status_msg status_recv = {};
  status_msg status_latest = {};
  bool has_status_latest = false;

  //network input message buffer
  int32 bytes_read = 0;
  uint32 buf_size = 1024;
  char message[buf_size];
  memset(message, 0 , buf_size);


  //connect to simulation
  int32 socket_handle = 0;
  int32 error = client_connect(cfg.ip, cfg.port, &socket_handle);
  if(error < 0){
    return -1;
  }
  socket_unblock_io(socket_handle);


  //send off control msg
  if(message_send(socket_handle, control_off.msg, control_off.msg_size, 0)){
    ctrl_state.is_on = false;
  }

  //send initial request
  if(message_send(socket_handle, request.msg, request.msg_size, 0)){
    //clock_gettime(CLOCK_MONOTONIC, &time_get);
    //t_send_latest = time_to_nsec(&time_get);
  }


  while(true){

    //receive status messages
    while((bytes_read = message_receive(socket_handle, message, buf_size))){
      if(bytes_read == -1){
        // buffer overrun, try again
        continue;
      }
      if(msg_type(message) != 1 || !status_msg_parse(&status_recv, message)){
        printf("not a valid status message: %s\n", message);
        continue;
      }

      clock_gettime(CLOCK_MONOTONIC, &time_get);
      t_recv_latest = time_to_nsec(&time_get);
      
      printf("Received message: %s\n", message);
      //calculate est. time at which target temperature is reached
      if(has_status_latest){
        //linear eq: m * x + b = c  ->  x = (c - b) / m
        //where c: target_temp, b: recv_temp, m: delta_temp / delta_time
        real64 time_diff = nsec_to_sec(status_recv.time - status_latest.time);
        real64 temp_diff = status_recv.temperature - status_latest.temperature;

        if(time_diff > 0 && temp_diff != 0.0){
          real64 t_temp = ctrl_state.is_on ? temp_target.temp_high : temp_target.temp_low;
          real64 t_sec = (t_temp - status_recv.temperature) * time_diff / temp_diff;

          t_send_next.time = t_sec < 0.0 ? 0 : sec_to_nsec(t_sec * t_send_factor);
          //printf("wait for %lu (%lu)\n", t_send_next.time, get_constrained_time(&t_send_next));
        }else{
          t_send_next.time = 0;
        }
      }
      
      status_msg_cpy(&status_latest, &status_recv);
      has_status_latest = true;
    }

    //send control message
    if(has_status_latest){
      if(status_latest.temperature <= temp_target.temp_low){
        if(!status_latest.is_on){
          if(message_send(socket_handle, control_on.msg, control_on.msg_size, 0)){
            printf("sending: ON\n");
            ctrl_state.is_on = true;
          }
        }
      } else if(status_latest.temperature >= temp_target.temp_high){
        if(status_latest.is_on){
          if(message_send(socket_handle, control_off.msg, control_off.msg_size, 0)){
            printf("sending: OFF\n");
            ctrl_state.is_on = false;
          }
        }
      }
    }

    clock_gettime(CLOCK_MONOTONIC, &time_get);
    uint64 t_now = time_to_nsec(&time_get);

    //send request message
    if(t_now >= t_recv_latest + get_constrained_time(&t_send_next)){

      //printf("Sending periodic request\n");
      if(message_send(socket_handle, request.msg, request.msg_size, 0)){
        //clock_gettime(CLOCK_MONOTONIC, &time_get);
        //t_send_latest = time_to_nsec(&time_get);
      }
    }

    //wait till next round
    nanosleep(&time_sleep, 0);
  }

  close(socket_handle);
  return 0;
}

void interpret_all(config* cfg, int32 argL, char** argV){
  for(int32 i = 0; i < argL; ++i){

    int32 index = index_of(argV[i], '=');

    if(index < 0){
      continue;

    }else if(str_begins_with(argV[i], "port")){
      int size = 6;     //44444 + '\0' = 6
      char dst_chars[size];
      str_cpy_substr(dst_chars, argV[i], index + 1, index + 1 + size);
      cfg->port = atoi(dst_chars);
      printf("port set to %i\n", cfg->port);

    }else if(str_begins_with(argV[i], "ip")){
      int size = 16;    //255.255.255.255 + '\0' = 16
      str_cpy_substr(cfg->ip, argV[i], index + 1, index + 1 + size);
      printf("ip set to %s\n", cfg->ip);

    }else if(str_begins_with(argV[i], "target")){
      int size = 24;    //est. max length for double value
      char dst_chars[size];
      str_cpy_substr(dst_chars, argV[i], index + 1, index + 1 + size);
      cfg->target_temp = (atof(dst_chars));
      printf("target temperature set to %f\n", cfg->target_temp);

    }else if(str_begins_with(argV[i], "range")){
      int size = 24;    //est. max length for double value
      char dst_chars[size];
      str_cpy_substr(dst_chars, argV[i], index + 1, index + 1 + size);
      cfg->target_temp = (atof(dst_chars));
      printf("target temperature range set to %f\n", cfg->target_range);
    }
  }
}

uint64 get_constrained_time(time_constrained* time){
  //assume time_low <= time_high
  if(time->time < time->time_min){
    return time->time_min;
  }
  if(time->time > time->time_max){
    return time->time_max;
  }
  return time->time;
}

