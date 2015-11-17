#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <fcntl.h>
#include "number_name_defs.h"

#define false 0
#define true  1

typedef int32 bool;

typedef struct config{
  uint32 port;
  char* ip;
} config;

//parameter interpretation
void interpret_all(config*, int32, char**);
int32 index_of(char*, char);

//str utils
bool str_equals(char*, int32, char*, int32);
void str_cpy_substr(char* dst_chars, char* src_chars, int32 src_offset, int32 src_end_index);

//network
int32 client_connect(char* address, uint32 port, int32* socket_handle);
int32 socket_unblock_io(int32 socket_handle);


int32 main(int32 argL, char** argV){
  config cfg;
  
  //set default values
  cfg.port = 4242;
  cfg.ip = "127.0.0.1";
  
  //grab available parameters
	interpret_all(&cfg, argL, argV);
  
  int32 socket_handle = 0;
  int32 error = client_connect(cfg.ip, cfg.port, &socket_handle);
  if(error < 0)
    printf("connection failed");

  //NOTE when the connection is successfully established unblock the i/o
  //socket_unblock_io(socket_handle);

  char* message = "test\n";
  write(socket_handle, message, sizeof(message));

  close(socket_handle);
	return 0;
}

void interpret_all(struct config* cfg, int32 argL, char** argV){
	for(int32 i = 0; i < argL; ++i){
    
		int32 index = index_of(argV[i], '=');
    
		if(index == -1){
			continue;
      
		}else if(str_equals(argV[i], index, "port", 4)){
      int size = 6;     //44444 + '\0' = 6
      char dst_chars[size];
      str_cpy_substr(dst_chars, argV[i], index + 1, index + 1 + size);
      (*cfg).port = atoi(dst_chars);
      printf("port set to %i\n", (*cfg).port);
      
    }else if(str_equals(argV[i], index, "ip", 2)){
      int size = 16;    //255.255.255.255 + '\0' = 16
      char dst_chars[size];
      str_cpy_substr(dst_chars, argV[i], index + 1, index + 1 + size);
      (*cfg).ip = dst_chars;  //<<=== is this ok ?
      printf("ip set to %s\n", (*cfg).ip);
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

bool str_equals(char* a_chars, int32 a_len, char* b_chars, int32 b_len){
  if(a_len != b_len){
    return false;
  }
  for(int i = 0; i < a_len; ++i){
    if(a_chars[i] != b_chars[i]){
      return false;
    }
  }
  return true;
}

//copies a substring from src_chars to dst_chars 
//as defined by src_offset and src_end_index
//or until it encounters '\0'
//
//appends '\0' if none is present
//
//dst_chars needs to be at least (src_end_index - src_offset) chars long if source is terminated by '\0'
//dst_chars needs to be at least (src_end_index + 1 - src_offset) chars long if source if not terminated by '\0'
void str_cpy_substr(char* dst_chars, char* src_chars, int32 src_offset, int32 src_end_index){
  int dst_i = 0;
  int src_i = src_offset;
  while(src_i < src_end_index){
    dst_chars[dst_i] = src_chars[src_i];
    if(!src_chars[src_i]){
      return;
    }
    ++dst_i;
    ++src_i;
  }
  dst_chars[dst_i] = '\0';
}

int32 socket_unblock_io(int32 socket_handle){
  //get current config of socket
  int32 current_flags = fcntl(socket_handle, F_GETFL);
  if(current_flags < 0){
    return -1;
  }

  //wirteback config with non block
  int32 error = fcntl(socket_handle, F_GETFL, current_flags | O_NONBLOCK);
  if(error < 0){
    return -1;
  }
  return 0;
}

int32 client_connect(char* address, uint32 port, int32* socket_handle){

  //get socket
  *socket_handle = socket(AF_INET, SOCK_STREAM, 0);


  //NOTE returns 0 if host cannot be found or is malformed
  struct hostent *server = gethostbyname(address);
  if(server == 0)
    return -1;


  struct sockaddr_in ip_address;
  memset(&ip_address, 0 ,sizeof(ip_address));


  //NOTE htons translates the number to Network byte order
  ip_address.sin_family = AF_INET;
  ip_address.sin_port = htons(port);


  printf("connecting to: %s\n", address);
  int32 state = connect(  *socket_handle,
                          (struct sockaddr*) &ip_address,
                          sizeof(ip_address));
  if(state < 0)
    return -1;

  return 0;
}
