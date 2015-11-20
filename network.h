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

static int32 client_connect(char* address, uint32 port, int32* socket_handle);
static int32 socket_unblock_io(int32 socket_handle);

static int32 pending_message_recive(int32 socket_handle, char* message, uint32 size);

#include "network.c"
