#pragma once
#include "YBaseLib/Common.h"

#if defined(Y_PLATFORM_WINDOWS)
    #include "YBaseLib/Windows/WindowsSubprocess.h"
#elif defined(Y_PLATFORM_POSIX)
    #include "YBaseLib/POSIX/POSIXSubprocess.h"
#else
    #error Unsupported platform
#endif
