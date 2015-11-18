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
#include "typedefs.h"

static int32 client_connect(char* address, uint32 port, int32* socket_handle);
static int32 socket_unblock_io(int32 socket_handle);

static int32 message_recive_pending(int32 socket_handle, char* message, uint32 size);

#include "network.c"
