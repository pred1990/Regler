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

typedef struct{
  bool heating_on;
  real64 current_temperature;
} State;

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

  struct timespec time_wait;


  char message[kilobyte];
  int32 time_round = 100 * 1000; // 100 milliseconds
  int32 time_request = 200 * 1000; // 1000 milliseconds
  int32 time_passed = 0;
  State state = {};

  //prepare request message
  request_msg request = {};
  request_msg_init(&request);

  //prepare control messages
  control_msg control_off = {};
  control_msg control_on = {};
  control_msg_set(&control_off, false);
  control_msg_set(&control_on, true);

  //send initial status message
  message_send(socket_handle, request.msg, sizeof(request.msg), 0);

  while(true){
    memset(message, 0 , kilobyte);

    //first check for messages
    int32 ret_val = 0;
    ret_val = pending_message_receive(socket_handle, message, sizeof(message));
    if(ret_val == -1){
        // buffer overrun, try again
        continue;
    }else if(ret_val > 0){
      //message received process it...
      printf("Received message: %s", message);
      status_msg status = {};

      //check message
      uint32 message_type = msg_type(message);
      if(message_type == 1){

        //convert the message into internal type
        bool parsing_complete = status_msg_parse(&status, message);
        if(parsing_complete == false){
          printf("Warning: parsing of message %s failed\n",message);
          continue;
        }
      }

      //TODO do somthing meaningfull here

      //TODO change status.temperature to status_msg_temperature ?
      if(status.temperature < cfg.target_temperature){
        if(state.heating_on == false){
          message_send(socket_handle, control_on.msg, sizeof(control_on.msg), 0);
        }
      //TODO change status.temperature to status_msg_temperature ?
      } else if(status.temperature > cfg.target_temperature){
        if(state.heating_on == true){
          message_send(socket_handle, control_off.msg, sizeof(control_off.msg), 0);
        }

      }
    }

    //send a reaquest for status
    if(time_passed >= time_request){
      //printf("Sending periodic request\n");
      errno = 0;
      message_send(socket_handle, request.msg, sizeof(request.msg), 0);
      if(errno != 0){
        if(errno != EWOULDBLOCK || errno != EAGAIN){
          printf("Sending failed: %s \n", strerror(errno));
        }
      }
      time_passed = 0;
    }

    //wait till next round
    time_wait.tv_nsec = time_round;
    nanosleep(&time_wait, 0);
    time_passed += time_round;
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
      cfg->target_temperature = (atof(dst_chars));
      printf("target temperature set to %f\n", cfg->target_temperature);
    }
  }
}
