#pragma once
#include "network.h"

static int32 socket_unblock_io(int32 socket_handle){
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

static int32 client_connect(char* address, uint32 port, int32* socket_handle){

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

static int32 message_recive_pending(int32 socket_handle, char* message, uint32 size){

  int32 available_bytes = recv(socket_handle, message, size, MSG_PEEK);
  printf("%i\n", available_bytes);

  
  return 0;
}


