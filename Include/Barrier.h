#pragma once
#include "YBaseLib/Common.h"

#if defined(Y_PLATFORM_WINDOWS)
    #include "YBaseLib/Windows/WindowsBarrier.h"
#else
    #include "YBaseLib/POSIX/POSIXBarrier.h"
#endif


