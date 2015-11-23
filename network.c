#pragma once
#include "network.h"

static int32 socket_unblock_io(int32 socket_handle){
  //get current config of socket
  int32 current_flags = fcntl(socket_handle, F_GETFL, 0);
  if(current_flags < 0){
    return -1;
  }

  //wirteback config with non block
  int32 error = fcntl(socket_handle, F_SETFL, current_flags | O_NONBLOCK);
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
    return -2;
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

static int32 pending_message_receive(int32 socket_handle, char* message, uint32 size){

  errno = 0;
  int32 size_peek = recv(socket_handle, message, size, MSG_PEEK);

  if(errno == EAGAIN || errno == EWOULDBLOCK){
    //there is no data to return
    return 0;
  }


  int32 message_end = index_of_ignore_terminate(message, size, '\n') ;
  if(message_end == -1){
    //has no end of message
    return -1;
  }

  //void buffered data
  if(size_peek == size){
    //remove trash data
    recv(socket_handle, message, size, 0);
    return -2;
  }

  //msg = "M\n\0"
  //message_end == 1;
  //message_end += 1 == 2
  
  message_end += 2;

  //fetch message from stack
  int32 bytes_read = recv(socket_handle, message, message_end, 0);
  
  printf("bytes_read: %i message_end: %i\n", bytes_read, message_end);
  
  //if this condition does fire, we got a problem
  assert(bytes_read == message_end);
  if(bytes_read != message_end){
    printf("warning: network message cannot be read till end\n");
  }

  //change \n to \0
  return message_end; //maybe message_end + 1 ?
}


