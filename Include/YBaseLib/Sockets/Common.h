#pragma once
#include "YBaseLib/Common.h"

#ifdef Y_PLATFORM_WINDOWS
    #include "YBaseLib/Windows/WindowsHeaders.h"
    #include <WinSock2.h>
    #include <WS2tcpip.h>
#else
    #include <sys/socket.h>
    #include <sys/types.h>
    typedef int SOCKET;
    #define SOCKET_ERROR -1
    #define INVALID_SOCKET -1
#endif

// Implementation
#define Y_SOCKET_IMPLEMENTATION_GENERIC
