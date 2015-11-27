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

//parameter interpretation
void interpret_all(config*, int32, char**);
uint64 time_median(time_constrained*);
bool is_send_ok();

int32 main(int32 argL, char** argV){
  config cfg;

  //set default config values
  cfg.port = 4242;
  str_cpy(cfg.ip, "127.0.0.1");
  cfg.target_temp = 90.0;

  //grab available config values from parameters
  interpret_all(&cfg, argL, argV);

  int32 socket_handle = 0;

  int32 error = client_connect(cfg.ip, cfg.port, &socket_handle);
  if(error < 0){
    return -1;
  }

  socket_unblock_io(socket_handle);

  //struct to be used for clock_gettime
  struct timespec time_get = {};
  
  struct timespec time_sleep = {};
  time_sleep.tv_nsec = msec_to_nsec(1);
  
  //prepare request message
  request_msg request = {};
  request_msg_init(&request);

  //prepare control messages
  control_msg control_off = {};
  control_msg control_on = {};
  control_msg_set(&control_off, false);
  control_msg_set(&control_on, true);

  //status message containers
  status_msg status_recv = {};
  status_msg status_latest = {};
  bool has_status_latest = false;
  
  //target temperature range
  temp_range temp_target = {};
  temp_target.temp = cfg.target_temp;
  temp_target.temp_low = temp_target.temp - 1.0;
  temp_target.temp_high = temp_target.temp + 1.0;
  //bool is_within_temp_range = false;
  
  control_state ctrl_state = {};
  
  uint64 t_recv_latest = 0;
  //uint64 t_send_latest = 0; //figure out why this may be useful
  
  real64 t_send_factor = 0.3;
  
  //time for next request
  time_constrained t_send_next = {};
  t_send_next.time = 0;
  t_send_next.time_min = msec_to_nsec(1);
  t_send_next.time_max = msec_to_nsec(50);
  
  //network input message buffer
  int32 bytes_read = 0;
  uint32 buf_size = 1024;
  char message[buf_size];
  memset(message, 0 , buf_size);
  
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

    //receive status message
    while((bytes_read = pending_message_receive(socket_handle, message, buf_size))){
      if(bytes_read == -1){
        // buffer overrun, try again
        continue;
      }

      //status
      if(msg_type(message) == 1){
        //printf("Received message: %s", message);
        bool is_valid = status_msg_parse(&status_recv, message);
        if(!is_valid){
          continue;
        }
        
        clock_gettime(CLOCK_MONOTONIC, &time_get);
        t_recv_latest = time_to_nsec(&time_get);
        
        //TODO figure out estimate time
        
        if(has_status_latest){
          real64 time_diff = nsec_to_sec(status_recv.time - status_latest.time);
          real64 temp_diff = status_recv.temperature - status_latest.temperature;
          
          if(time_diff > 0 && temp_diff != 0.0){
            //linear eq: m * x + b = c
            real64 m = temp_diff / time_diff;
            real64 b = status_recv.temperature;
            real64 c = ctrl_state.is_on ? temp_target.temp_high : temp_target.temp_low;
            
            real64 t_sec = (c - b) / m;
            //printf("(%f - %f) / %f = %f\n", c, b, m, t_sec);
            t_send_next.time = t_sec < 0.0 ? 0 : sec_to_nsec(t_sec) * t_send_factor;
            //printf("wait for %lu (%lu)\n", t_send_next.time, time_median(&t_send_next));
          }else{
            t_send_next.time = 0;
          }
        }
        
        status_msg_cpy(&status_latest, &status_recv);
        has_status_latest = true;
        
      }else{
        printf("not a valid message: %s", message);
      }
    }

    //send control message
    if(has_status_latest){
      if(status_latest.temperature <= temp_target.temp_low){
        if(!status_latest.is_on){
          if(message_send(socket_handle, control_on.msg, control_on.msg_size, 0)){
            ctrl_state.is_on = true;
          }
        }
      } else if(status_latest.temperature >= temp_target.temp_high){
        if(status_latest.is_on){
          if(message_send(socket_handle, control_off.msg, control_off.msg_size, 0)){
            ctrl_state.is_on = false;
          }
        }
      }
    }
    
    clock_gettime(CLOCK_MONOTONIC, &time_get);
    uint64 t_now = time_to_nsec(&time_get);
    
    //send request message
    //printf("now: %lu latest: %lu delay: %lu\n", t_now, t_recv_latest, time_median(&t_send_next));
    if(t_now >= t_recv_latest + time_median(&t_send_next)){
    
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
    }
  }
}

uint64 time_median(time_constrained* time){
  //assume time_low <= time_high
  if(time->time < time->time_min){
    return time->time_min;
  }
  if(time->time > time->time_max){
    return time->time_max;
  }
  return time->time;
}

