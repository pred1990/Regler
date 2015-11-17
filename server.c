#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <fcntl.h>
#include "number_name_defs.h"

int32 main(int32 argL, char** argV){
	struct sockaddr_in test;
	struct sockaddr_in test2;

	memset(&test, 0 ,sizeof(test));
	test.sin_family = AF_INET;
	test.sin_addr.s_addr = INADDR_ANY;
	test.sin_port = htons(4242);
	int32 sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (bind(sockfd, (struct sockaddr *) &test, sizeof(test)) < 0)
		return 1;	//error 1
	listen(sockfd, 5);
	uint32 test2len = sizeof(test2);
  int32 newsockfd = accept(sockfd, (struct sockaddr *) &test2, &test2len);
  if( newsockfd < 0)
    printf("not good %i", newsockfd);

  //get space for message with 4 chars
  int32 bytes = 0;
  //run as long as u can
  while(bytes >= 0){
  char message[100] = {};
    bytes = recv(newsockfd, message, 100, 0);

    if(bytes == 0)
      break;
    printf("message %s recived, bytes: %i\n", message, bytes);
  }


  close(newsockfd);
  close(sockfd);

}
