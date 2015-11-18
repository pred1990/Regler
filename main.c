#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "typedefs.h"
#include "network.h"

//parameter interpretation
void interpret_all(config*, int32, char**);

//string utils
int32 index_of(char*, char);
bool str_begins_with(char*, char*);
bool substr_begins_with(char*, int32, char*);
void str_cpy(char* dst_chars, char* src_chars, int32 src_end_index);
void str_cpy_substr(char* dst_chars, char* src_chars, int32 src_offset, int32 src_end_index);


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

int32 index_of(char* search_str, char find_char){
  int32 i = 0;
  while(search_str[i] && search_str[i] != find_char){
    ++i;
  }
  return search_str[i] ? i : -1;
}

bool str_begins_with(char* a_chars, char* b_chars){
  return substr_begins_with(a_chars, 0, b_chars);
}

//compares a substring with a proper (i.e. zero-terminated) string
bool substr_begins_with(char* a_chars, int32 a_offset, char* b_chars){
  a_chars += a_offset;
  while(*b_chars){
    if(*a_chars != *b_chars){
      return false;   // different chars or a_chars ended
    }
    ++a_chars;
    ++b_chars;
  }
  return true;
}

void str_cpy(char* dst_chars, char* src_chars, int32 src_len){
  str_cpy_substr(dst_chars, src_chars, 0, src_len);
}

//copies a substring from src_chars to dst_chars
//the copied string may zero-terminate before the specified src_end_index is reached
//the copy string will be zero-terminated
//dst_chars needs to be sufficiently big
void str_cpy_substr(char* dst_chars, char* src_chars, int32 src_offset, int32 src_end_index){
  int src_i = src_offset;
  while(src_i < src_end_index){
    *dst_chars = src_chars[src_i];
    if(!src_chars[src_i]){
      return;
    }
    ++dst_chars;
    ++src_i;
  }
  *dst_chars = '\0';
}

