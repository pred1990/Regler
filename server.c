#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <fcntl.h>
#include "typedefs.h"


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
  if( client == -1)
    printf("accept failed");

  int32 bytes_read = 0;
  while(bytes_read >= 0){
  char message[100] = {};
    //bytes_read = message_recive_pending(client, message, 10);
    bytes_read = recv(client, message, 10, 0);
    if(bytes_read == 0)
      break;
    printf("message %s recived, bytes: %i\n", message, bytes_read);
    sleep(2);
    char* test = "OK\n";
    send(client, test, 3, 0);
  }

  close(client);
  close(server);

}


