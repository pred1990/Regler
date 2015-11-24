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
#include <assert.h>
#include "typedefs.h"
#include "string_utils.h"
#include "messages.h"

int32 client_connect(char* address, uint32 port, int32* socket_handle);
int32 socket_unblock_io(int32 socket_handle);

int32 pending_message_receive(int32 socket_handle, char* message, uint32 size);

void message_send(int32 socket_handle, char* message, uint32 msg_size, int32 flags);

#include "network.c"
