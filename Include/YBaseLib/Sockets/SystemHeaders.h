#pragma once
#include "YBaseLib/Common.h"

#ifdef Y_PLATFORM_WINDOWS
    #include "YBaseLib/Windows/WindowsHeaders.h"
    #include <WinSock2.h>
    #include <WS2tcpip.h>
#else
    #include <sys/socket.h>
    #include <sys/types.h>
    #include <sys/uio.h>
    #include <sys/ioctl.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <errno.h>
    #include <unistd.h>

    typedef int SOCKET;
    #define SOCKET_ERROR -1
    #define INVALID_SOCKET -1
#endif

