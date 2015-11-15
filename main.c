#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include "number_name_defs.h"

void interpret_all(int32, char**);
int32 index_of(char*, char);

int32 main(int32 argL, char** argV){
  uint32 port = 4242;

  int32 socket_handle = socket(AF_INET, SOCK_STREAM, 0);

  //NOTE returns 0 if host cannot be found or is malformed
  struct hostent *server = gethostbyname("127.0.0.1");
  if(server == 0)
    return -1;

  struct sockaddr_in ip_address;
	memset(&ip_address, 0 ,sizeof(ip_address));

  //NOTE Use Inet, despise the fact that we are local
  //NOTE htons translates the number to Network byte order
  ip_address.sin_family = AF_INET;
  ip_address.sin_port = htons(port);

  if(connect(socket_handle, (struct sockaddr*) &ip_address, sizeof(ip_address)) < 0);
    printf("connect error");
  
  char * message = "test\n";
  write(socket_handle, message, sizeof(message));

  close(socket_handle);

  //memcpy(&ip_address.sin_addr.s_addr, server->h_addr_list[0] , server->h_length);


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
