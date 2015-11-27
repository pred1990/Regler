#pragma once
#include "network.h"

int32 socket_unblock_io(int32 socket_handle){
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

int32 client_connect(char* address, uint32 port, int32* socket_handle){

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
  //ip_address.sin_addr = 
    inet_aton(address, &ip_address.sin_addr);


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

bool message_send(int32 socket_handle, char* message, int32 message_size, int32 flags){
  send(socket_handle, message, message_size, flags);
  if(errno != 0){
    if(errno != EWOULDBLOCK || errno != EAGAIN){
      printf("Sending failed: %s \n", strerror(errno));
      errno = 0;
      return false;
    }
  }
  return true;
}

int32 pending_message_receive(int32 socket_handle, char* message, uint32 size){
  //note: return -1 means that this method can be called again
  //could have used recursive call instead for nicer return value semantics
  //but receiving lots of trash would force many recursive calls
  
  errno = 0;
  int32 size_peek = recv(socket_handle, message, size, MSG_PEEK);

  if(errno == EAGAIN || errno == EWOULDBLOCK){
    //there is no data to return
    return 0;
  }

  bool is_msg_found = false;
  int32 msg_begin = 0;
  int32 msg_end = index_of_ignore_terminate(message, size, '\n') ;
  //printf("%s %i\n", message, msg_end);
  
  
  if(msg_end == -1){
    //has no end of message
    if(size_peek < size){
      //no more data, can be cleared
      //TODO fixme
      recv(socket_handle, message, size_peek, 0);
      return 0;
    }else{
      //may have more data, throw half of current data (garbage) away
      recv(socket_handle, message, size / 2, 0);
      return -1;    //retry
    }
  }

  //find message begin
  if(msg_type(message) > 0){
    is_msg_found = true;

  }else{
    msg_begin = msg_end;

    //figure out how far left we can go
    int32 leftmost_index = msg_end - 60;
    if(leftmost_index < 1){
      leftmost_index = 1;   //we already checked 0
    }

    //go left
    while(!is_msg_found && msg_begin >= leftmost_index){
      --msg_begin;
      is_msg_found = msg_type(message + msg_begin) > 0;
    }
  }

  if(!is_msg_found){
    //there's a \n somewhere no message
    if(size_peek < size){
      //no more data, can be cleared
      recv(socket_handle, message, size_peek, 0);
      return 0;
    }else{
      //may have more data, throw everything up to \n away
      recv(socket_handle, message, msg_end + 1, 0);
      return -1;    //retry
    }
  }

  //printf("begin: %i end: %i\n", msg_begin, msg_end);
  if(msg_begin > 0){
    //clear trash, reload message
    recv(socket_handle, message, msg_begin, 0);   //target char* may not be 0
    msg_end -= msg_begin;
    recv(socket_handle, message, msg_end + 1, MSG_PEEK);  // + 2 : include \n and \0    //TOOD check if this is necessary
  }
  //fetch message
  int32 bytes_read = recv(socket_handle, message, msg_end + 1, 0);
  
  //make sure we didn't do anything silly
  printf("msg: %s", message);
  assert(bytes_read == msg_end + 1);
  assert(message[msg_end] == '\n');
  message[msg_end] = '\0';
  
  return msg_end + 1;   //inclue \n
}
