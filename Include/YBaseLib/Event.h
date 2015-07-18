#pragma once
#include "YBaseLib/Common.h"

// Windows Implementation
#if defined(Y_PLATFORM_WINDOWS)
    #include "YBaseLib/Windows/WindowsEvent.h"

// Linux implementation with pthreads
#elif defined(Y_PLATFORM_POSIX)
    #include "YBaseLib/POSIX/POSIXEvent.h"

// HTML5 implementation, a no-op
#elif defined(Y_PLATFORM_HTML5)
    #include "YBaseLib/HTML5/HTML5Event.h"

#else

#error Unknown platform.

#endif


