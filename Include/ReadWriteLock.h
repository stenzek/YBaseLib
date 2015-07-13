#pragma once
#include "YBaseLib/Common.h"

// Windows Implementation
#if defined(Y_PLATFORM_WINDOWS)
    #include "YBaseLib/Windows/WindowsReadWriteLock.h"

// Linux implementation with pthreads
#elif defined(Y_PLATFORM_POSIX)
    #include "YBaseLib/POSIX/POSIXReadWriteLock.h"

// HTML5 implementation no-op
#elif defined(Y_PLATFORM_HTML5)
    #include "YBaseLib/HTML5/HTML5ReadWriteLock.h"

#else

#error Unknown platform.

#endif


