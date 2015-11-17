#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>
#include <errno.h>
#include "number_name_defs.h"


int32 client_connect(char* address, uint32 port, int32* socket_handle);
int32 socket_unblock_io(int32 socket_handle);

#include "network.c"
