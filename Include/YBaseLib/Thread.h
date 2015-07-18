#pragma once
#include "YBaseLib/Common.h"

// Windows Implementation
#if defined(Y_PLATFORM_WINDOWS)
    #include "YBaseLib/Windows/WindowsThread.h"

// Linux implementation with pthreads
#elif defined(Y_PLATFORM_POSIX)
    #include "YBaseLib/POSIX/POSIXThread.h"

// HTML5 implementation no-op
#elif defined(Y_PLATFORM_HTML5)
    #include "YBaseLib/HTML5/HTML5Thread.h"

#else

#error Unknown platform.

#endif


