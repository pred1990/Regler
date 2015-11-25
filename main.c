#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "typedefs.h"
#include "network.h"
#include "string_utils.h"
#include "messages.h"

typedef struct{
  uint32 port;
  char ip[16];
  real64 target_temperature;
} config;

/*
typedef struct{
  bool heating_on;
  real64 current_temperature;
} State;
*/

//parameter interpretation
void interpret_all(config*, int32, char**);

int32 main(int32 argL, char** argV){
  config cfg;

  //set default config values
  cfg.port = 4242;
  str_cpy(cfg.ip, "127.0.0.1");
  cfg.target_temperature = 90.0;

  //grab available config values from parameters
  interpret_all(&cfg, argL, argV);

  int32 socket_handle = 0;

  int32 error = client_connect(cfg.ip, cfg.port, &socket_handle);
  if(error < 0){
    return -1;
  }

  socket_unblock_io(socket_handle);

  struct timespec time_sleep = {};
  time_sleep.tv_nsec = 5 * 1000 * 1000; // 5 milliseconds
  uint32 time_request = 35 * 1000 * 1000; // 35 milliseconds
  uint32 time_passed = 0;
  
  //prepare request message
  request_msg request = {};
  request_msg_init(&request);

  //prepare control messages
  control_msg control_off = {};
  control_msg control_on = {};
  control_msg_set(&control_off, false);
  control_msg_set(&control_on, true);

  //status message container (receive)
  status_msg status = {};
  
  //send initial status message
  send(socket_handle, request.msg, request.msg_size, 0);
  printf("first request sent!\n");
  
  //buffer
  int32 bytes_read = 0;
  //uint32 buf_size = 1024;
  char message[1024];
  memset(message, 0 , 1024);
  
  while(true){

    //printf("bar\n");
    //check for messages
    while((bytes_read = pending_message_receive(socket_handle, message, 1024))){
      if(bytes_read == -1){
        // buffer overrun, try again
        continue;
      }

      //status
      if(msg_type(message) == 1){
        printf("Received message: %s", message);
        bool is_valid = status_msg_parse(&status, message);
        if(!is_valid){
          continue;
        }

        //TODO do somthing meaningfull here

        if(status.temperature <= (cfg.target_temperature - 1)){
          if(!status.is_on){
            send(socket_handle, control_on.msg, control_on.msg_size, 0);
          }
        } else if(status.temperature >= (cfg.target_temperature + 1)){
          if(!status.is_on){
            send(socket_handle, control_off.msg, control_off.msg_size, 0);
          }
        }

      }else{
        printf("not a valid message: %s", message);
      }
    }

    //send a reaquest for status
    if(time_passed >= time_request){
      //printf("Sending periodic request\n");
      errno = 0;
      send(socket_handle, request.msg, request.msg_size, 0);
      if(errno != 0){
        if(errno != EWOULDBLOCK || errno != EAGAIN){
          printf("Sending failed: %s \n", strerror(errno));
        }
      }
      time_passed = 0;
    }

    //wait till next round
    nanosleep(&time_sleep, 0);
    time_passed += time_sleep.tv_nsec;
  }

  printf("exited loop!\n");
  
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
      cfg->target_temperature = (atof(dst_chars));
      printf("target temperature set to %f\n", cfg->target_temperature);
    }
  }
}
