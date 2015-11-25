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
void status_calculate_next(status_msg* next, status_msg* last, struct timespec* time, environment* env);

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

  //internal status of last on/off switch
  status_msg status_last;
  
  //status message container (response)
  status_msg status_public;

  //set initial status
  struct timespec status_time;
  clock_gettime(CLOCK_MONOTONIC, &status_time);
  status_msg_temperature(&status_last, env.temperature + 30.0);
  status_msg_is_on(&status_last, false);
  status_msg_time(&status_last, time_as_nanos(&status_time));

  //control messages containers (parse incoming messages)
  control_msg control;
  
  //sleep timer
  struct timespec time_sleep;
  time_sleep.tv_sec = 0;
  time_sleep.tv_nsec = 1000 * 1000 * 50;  //50ms
  
  //buffer
  int32 bytes_read = 0;
  uint32 buf_size = 1024;
  char message[buf_size];
  printf("msg_size: %lu", sizeof(message));
  
  while(true){
    nanosleep(&time_sleep, 0);
    
    //TODO put read thingy in some loop
    //as it is, only one message gets read per iteration
    while((bytes_read = pending_message_receive(client, message, buf_size))){
      if(bytes_read == -1){
        //no message, but has more data -> retry
        continue;
      }
      
      uint32 type = msg_type(message);
      
      //control
      if(type == 2){
        bool is_valid = control_msg_parse(&control, message);
        if(!is_valid){
          continue;
        }
        if(control.set_on == status_last.is_on){
          continue;     //no change
        }
        //update reference status
        status_calculate_next(&status_last, &status_last, &status_time, &env);
        status_last.is_on = control.set_on;
        
      //status
      }else if(type == 3){
        status_calculate_next(&status_public, &status_last, &status_time, &env);
        status_msg_write(&status_public);
        printf("sending message: %s", status_public.msg);
        message_send(client, status_public.msg, 50, 0);
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

void status_calculate_next(status_msg* next, status_msg* last, struct timespec* time, environment* env){
  //get time
  clock_gettime(CLOCK_MONOTONIC, time);
  uint64 time_next = time_as_nanos(time);   //next and last could point to the same struct
  real64 d_time = (time_next - last->time) / 1000000000.0;
  next->time = time_next;
  
  //copy on|off state
  next->is_on = last->is_on;
  
  //calculate temperature according to on|off state
  next->temperature = last->temperature + (last->is_on ? env->t_sec_on : env->t_sec_off) * d_time;
  
  //if cooled below env. temperature, set to env temperature
  if(next->temperature < env->temperature){
    next->temperature = env->temperature;
  }
}
