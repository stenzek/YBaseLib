#pragma once
#include "YBaseLib/Common.h"

// Windows Implementation
#if defined(Y_PLATFORM_WINDOWS)
    #include "YBaseLib/Windows/WindowsRecursiveMutex.h"

// Linux implementation with pthreads
#elif defined(Y_PLATFORM_POSIX)
    #include "YBaseLib/POSIX/POSIXRecursiveMutex.h"

// HTML5 implementation no-op
#elif defined(Y_PLATFORM_HTML5)
    #include "YBaseLib/HTML5/HTML5RecursiveMutex.h"

#else

#error Unknown platform.

#endif


