#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <time.h>
#include <fcntl.h>
#include <fcntl.h>
#include "typedefs.h"
#include "messages.h"
#include "network.h"

typedef struct {
  real64 temperature;
  real64 t_sec_on;
  real64 t_sec_off;
} environment;

uint64 time_as_nanos(struct timespec* time);
void status_calculate_next(status_msg*, struct timespec*, environment* env);

int32 main(int32 argL, char** argV){

  struct sockaddr_in server_address;
  struct sockaddr_in client_address;

  //clear server address
  memset(&server_address, 0 ,sizeof(server_address));

  //define nessesaray data
  server_address.sin_family = AF_INET;
  server_address.sin_addr.s_addr = INADDR_ANY;
  server_address.sin_port = htons(4242);

  //get server handle
  int32 server = socket(AF_INET, SOCK_STREAM, 0);

  //bind server to port etc
  int32 bind_error = bind(  server,
                            (struct sockaddr *) &server_address,
                            sizeof(server_address));
  if (bind_error < 0){
    return 1;
  }

  //begin listing on socket
  listen(server, 5);

  uint32 client_address_size = sizeof(client_address);
  int32 client = accept( server,
                         (struct sockaddr *) &client_address,
                         &client_address_size);
  if(client == -1){
    printf("accept failed");
  }

  //create environment
  environment env;
  env.temperature = 23.5;
  env.t_sec_on = 0.8;
  env.t_sec_off = -0.1;

  //status message container (response)
  status_msg status;

  //set initial status
  struct timespec status_time;
  clock_gettime(CLOCK_MONOTONIC, &status_time);
  status_msg_temperature(&status, env.temperature + 90.0);
  status_msg_is_on(&status, false);
  status_msg_time(&status, time_as_nanos(&status_time));

  //control messages containers (parse incoming messages)
  control_msg control;
  
  //sleep timer
  struct timespec time_sleep;
  time_sleep.tv_sec = 0;
  time_sleep.tv_nsec = 1000 * 1000 * 200;  //50ms
  
  //buffer
  int32 bytes_read = 0;
  uint32 buf_size = 1024;
  char message[buf_size];
  printf("msg_size: %lu", sizeof(message));
  
  while(true){
    nanosleep(&time_sleep, 0);
    
    bytes_read = pending_message_receive(client, message, buf_size);
    if(bytes_read == 0){
      //TODO handle error cases
      continue;
    }else if(bytes_read == -1){
      continue;
    }else if(bytes_read == -2){
      continue;
    }else{
      if(msg_type(message) == 2){         //control
        bool is_valid = control_msg_parse(&control, message);
        if(is_valid){
          status_calculate_next(&status, &status_time, &env);
          status.is_on = control.set_on;
        }
      }else if(msg_type(message) == 3){   //status
        status_calculate_next(&status, &status_time, &env);
        status_msg_write(&status);
        printf("sending message: %s", status.msg);
        message_send(client, status.msg, 50, 0);
      }else{
        printf("not a valid message: %s\n", message);
      }
    }
    
  }

  close(client);
  close(server);

}

uint64 time_as_nanos(struct timespec* time){
  return time->tv_sec * 1000000000 + time->tv_nsec;
}

void status_calculate_next(status_msg* status, struct timespec* status_time, environment* env){
  //get time
  clock_gettime(CLOCK_MONOTONIC, status_time);
  uint64 time_new = time_as_nanos(status_time);
  real64 time_diff_sec = (time_new - status->time) / 1000000000.0;
  status->time = time_new;
  
  //change temperature according to on/off state
  status->temperature += (status->is_on ? env->t_sec_on : env->t_sec_off) * time_diff_sec;
  
  //if cooled below env. temperature, set to env temperature
  if(status->temperature < env->temperature){
    status->temperature = env->temperature;
  }
}
