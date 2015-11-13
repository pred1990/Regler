#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "number_name_defs.h"

int32 main(int32 argL, char** argV){
	struct sockaddr_in test;
	struct sockaddr_in test2;
	memset(&test, 0 ,sizeof(test));
	test.sin_family = AF_INET;
	test.sin_addr.s_addr = INADDR_ANY;
	test.sin_port = 4242;
	int32 sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (bind(sockfd, (struct sockaddr *) &test, sizeof(test)) < 0)
		return 1;	//error 1
	listen(sockfd, 5);
	int32 test2len = sizeof(test2);
    newsockfd = accept(sockfd, (struct sockaddr *) &test2, &test2len);
}
