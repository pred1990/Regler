#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>
#include <errno.h>
#include "number_name_defs.h"

void interpret_all(int32, char**);
int32 index_of(char*, char);

//network
int32 client_connect(char* address, uint32 port, int32* socket_handle);
int32 socket_unblock_io(int32 socket_handle);

int32 main(int32 argL, char** argV){
  uint32 port = 4242;
  int32 socket_handle = 0;

  int32 error = client_connect("127.0.0.1", port, &socket_handle);
  if(error < 0){
    return -1;
  }

  //NOTE when the connection is successfully established unblock the i/o
  socket_unblock_io(socket_handle);

  char message[12] = {};
  //for(uint32 i = 0; i < 20000; ++i){
    //itoa(i, message, 10);
    send(socket_handle, message, sizeof(message), 0);
  //}


  close(socket_handle);

	//interpret_all(argL, argV);
	return 0;
}

void interpret_all(int32 argL, char** argV){
	for(int32 i = 0; i < argL; ++i){
		int32 index = index_of(argV[i], '=');
		if(index == -1){
			continue;
		}
		//TODO parse kv pair
	}
}

int32 index_of(char* search_str, char find_char){
	int32 i = 0;
	while(search_str[i] && search_str[i] != find_char){
		++i;
  	}
	return search_str[i] ? -1 : i;
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

int32 client_connect(char * address, uint32 port, int32* socket_handle){

  //get socket
  *socket_handle = socket(AF_INET, SOCK_STREAM, 0);
  //NOTE -1 is only valid for Linux!
  if(*socket_handle == -1){
    return -1;
  }


  //NOTE returns 0 if host cannot be found or is malformed
  struct hostent *server = gethostbyname(address);
  if(server == 0){
    return -1;
  }

  struct sockaddr_in ip_address;
  memset(&ip_address, 0 ,sizeof(ip_address));


  //NOTE htons translates the number to Network byte order
  ip_address.sin_family = AF_INET;
  ip_address.sin_port = htons(port);


  printf("connecting to: %s\n", address);
  int32 state = connect(  *socket_handle,
                          (struct sockaddr*) &ip_address,
                          sizeof(ip_address));
  if(state < 0){
    printf("%s\n",strerror(errno));
    close(*socket_handle);
    return -2;
  }

  return 0;
}
