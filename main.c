#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "typedefs.h"
#include "network.h"
#include "string_utils.h"

//parameter interpretation
void interpret_all(config*, int32, char**);

int32 main(int32 argL, char** argV){
  config cfg;
  
  //set default config values
  cfg.port = 4242;
  str_cpy(cfg.ip, "127.0.0.1", 16);

  //grab available config values from parameters
  interpret_all(&cfg, argL, argV);

  int32 socket_handle = 0;

  int32 error = client_connect(cfg.ip, cfg.port, &socket_handle);
  if(error < 0){
    return -1;
  }

  //NOTE when the connection is successfully established unblock the i/o
  socket_unblock_io(socket_handle);

  int32 index = 0;
  char* pool[4] = {};
  pool[0] = "mein";
  pool[1] = "name";
  pool[2] = "ist";
  pool[3] = "steffen";
  while(1){
    index = (index + 1) %4;
    int32 len = strlen(pool[index]);
    send(socket_handle, pool[index], len, 0);
  }

  close(socket_handle);
  return 0;

}

void interpret_all(config* cfg, int32 argL, char** argV){
  for(int32 i = 0; i < argL; ++i){

    int32 index = index_of(argV[i], '=');

    if(index == -1){
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
    }
  }
}
