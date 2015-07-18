#pragma once
#include "YBaseLib/Common.h"

// Windows Implementation
#if defined(Y_PLATFORM_WINDOWS)
    #include "YBaseLib/Windows/WindowsMutex.h"

// Linux implementation with pthreads
#elif defined(Y_PLATFORM_POSIX)
    #include "YBaseLib/POSIX/POSIXMutex.h"

// Emscripten implementation that is null
#elif defined(Y_PLATFORM_HTML5)
    #include "YBaseLib/HTML5/HTML5Mutex.h"

#else

#error Unknown platform.

#endif


