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


  //wait 100 miliseconds as default
  uint32 milisec_stop = 100;

  char message[kilobyte];
  State state = {};

  while(1){
    //first check for messages
    int32 error = 0; 
    error = pending_message_recive(socket_handle, message, sizeof(message));
    if(error == -2){
        // buffer overrun, try again
        continue;
      }

    //prepare status message
    status_msg status = {};

    //check message
    uint32 message_type = msg_type(message);
    if(message_type == 1){

      //convert the message into internal type
      bool error = status_msg_parse(&status, message);
      if(error == 1){
        printf("Warning: parsing of message %s failed\n",message);
        continue;
      }

      //TODO do somthing meaningfull here
        //create the control_off message
      control_msg control_off;
      if(status.temperature < cfg.target_temperature){
        if(state.heating_on == false){
          //too low temperature and heating is off -> turn heating on
          control_msg_set(&control_off, true);
          send(socket_handle, control_off.msg, sizeof(control_off.msg), 0);
        }
      } else {
        if(state.heating_on == true){
          //temperature is reached and heating is still on -> turn heating off
          control_msg_set(&control_off, false);
          send(socket_handle, control_off.msg, sizeof(control_off.msg), 0);
        }
      }

    } else {
      //message type is unknown
      printf("Warning: message %s is not of type status\n", message);
    }


    //create requests

    //wait till next round
    time_wait.tv_nsec = 1000 * milisec_stop;
    nanosleep(&time_wait, 0);
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
