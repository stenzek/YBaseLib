#include "YBaseLib/Log.h"
#include "YBaseLib/String.h"
#include "YBaseLib/CString.h"
#include "YBaseLib/Assert.h"

Log *g_pLog = &Log::GetInstance();

Log::Log()
{

}

Log::~Log()
{
    DebugAssert(g_pLog == this);
    g_pLog = NULL;
}

void Log::RegisterCallback(CallbackFunctionType CallbackFunction, void *UserParam)
{
    RegisteredCallback Callback;
    Callback.Function = CallbackFunction;
    Callback.Parameter = UserParam;

    m_CallbackLock.Lock();
    m_liCallbacks.Add(Callback);
    m_CallbackLock.Unlock();
}

void Log::UnregisterCallback(CallbackFunctionType CallbackFunction, void *UserParam)
{
    m_CallbackLock.Lock();

    for (uint32 i = 0; i < m_liCallbacks.GetSize(); i++)
    {
        if (m_liCallbacks[i].Function == CallbackFunction && m_liCallbacks[i].Parameter == UserParam)
        {
            m_liCallbacks.OrderedRemove(i);
            break;
        }
    }

    m_CallbackLock.Unlock();
}

static bool s_bConsoleOutputEnabled = false;
static String s_strConsoleOutputChannelFilter;
static LOGLEVEL s_eConsoleOutputLevelFilter = LOGLEVEL_TRACE;

#if defined(Y_PLATFORM_WINDOWS)

static void ConsoleOutputLogCallback(void *UserParam, const char *ChannelName, LOGLEVEL Level, const char *Message)
{
    if (!s_bConsoleOutputEnabled || Level > s_eConsoleOutputLevelFilter || s_strConsoleOutputChannelFilter.Find(ChannelName) >= 0)
        return;

    HANDLE hConsole = GetStdHandle((Level <= LOGLEVEL_WARNING) ? STD_ERROR_HANDLE : STD_OUTPUT_HANDLE);
    if (hConsole != INVALID_HANDLE_VALUE)
    {
        static const WORD uLogLevelColors[LOGLEVEL_COUNT] =
        {
            FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_GREEN,                        // NONE
            FOREGROUND_RED | FOREGROUND_INTENSITY,                                      // ERROR
            FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY,                   // WARNING
            FOREGROUND_GREEN | FOREGROUND_INTENSITY,                                    // SUCCESS
            FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY, // INFO
            FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY,                    // PERF
            FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_GREEN,                        // DEV
            FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY,                  // PROFILE
            FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_GREEN,                        // TRACE
        };

        CONSOLE_SCREEN_BUFFER_INFO ConsoleScreenBufferInfo;
        GetConsoleScreenBufferInfo(hConsole, &ConsoleScreenBufferInfo);
        SetConsoleTextAttribute(hConsole, uLogLevelColors[(Level > 7) ? 7 : Level]);

        uint32 len;
        DWORD written;

        //char Tmp[32];
        //len = Y_snprintf(Tmp, YCountOf(Tmp), "[%s]: ", ChannelName);
        //WriteConsoleA(hConsole, Tmp, len, &written, NULL);

        len = Y_strlen(Message);
        WriteConsoleA(hConsole, Message, len, &written, NULL);

        static const char *NewlineStr = "\r\n";
        WriteConsoleA(hConsole, NewlineStr, 2, &written, NULL);

        SetConsoleTextAttribute(hConsole, ConsoleScreenBufferInfo.wAttributes);
    }
}

#elif defined(Y_PLATFORM_POSIX)

static void ConsoleOutputLogCallback(void *UserParam, const char *ChannelName, LOGLEVEL Level, const char *Message)
{
    if (!s_bConsoleOutputEnabled || Level > s_eConsoleOutputLevelFilter || s_strConsoleOutputChannelFilter.Find(ChannelName) >= 0)
        return;

    static const char *colorCodes[LOGLEVEL_COUNT] = 
    {
        "\033[0m",         // NONE
        "\033[1;31m",   // ERROR
        "\033[1;33m",   // WARNING
        "\033[1;37m",   // SUCCESS 
        "\033[1;37m",   // INFO
        "\033[1;35m",   // PERF
        "\033[0;37m",   // DEV
        "\033[1;36m",   // PROFILE
        "\033[1;36m",   // TRACE
    };

    int outputFd = (Level <= LOGLEVEL_WARNING) ? STDERR_FILENO : STDOUT_FILENO;

    size_t len = Y_strlen(Message);
    if (len > 0)
    {
        const char *colorCode = colorCodes[Level];
        write(outputFd, colorCode, Y_strlen(colorCode));
        write(outputFd, Message, len);
        write(outputFd, colorCodes[0], Y_strlen(colorCodes[0]));
    }

    write(outputFd, "\n", 1);
}

#elif defined(Y_PLATFORM_HTML5)

static void ConsoleOutputLogCallback(void *UserParam, const char *ChannelName, LOGLEVEL Level, const char *Message)
{
    if (!s_bConsoleOutputEnabled || Level > s_eConsoleOutputLevelFilter || s_strConsoleOutputChannelFilter.Find(ChannelName) >= 0)
        return;

    size_t len = Y_strlen(Message);
    if (len > 0)
        write(STDOUT_FILENO, Message, len);

    write(STDOUT_FILENO, "\n", 1);
}

#endif

void Log::SetConsoleOutputParams(bool Enabled, const char *ChannelFilter, LOGLEVEL LevelFilter)
{
    if (s_bConsoleOutputEnabled != Enabled)
    {
        s_bConsoleOutputEnabled = Enabled;
        if (Enabled)
            RegisterCallback(ConsoleOutputLogCallback, NULL);
        else
            UnregisterCallback(ConsoleOutputLogCallback, NULL);

#if defined(Y_PLATFORM_WINDOWS)
        // On windows, no console is allocated by default on a windows based application
        static bool consoleWasAllocated = false;
        if (Enabled)
        {
            if (GetConsoleWindow() == NULL)
            {
                DebugAssert(!consoleWasAllocated);
                consoleWasAllocated = true;
                AllocConsole();

                freopen("CONIN$", "r", stdin);
                freopen("CONOUT$", "w", stdout);
                freopen("CONOUT$", "w", stderr);
            }
        }
        else
        {
            if (consoleWasAllocated)
            {
                FreeConsole();
                consoleWasAllocated = false;
            }
        }
#endif
    }

    s_strConsoleOutputChannelFilter = (ChannelFilter != NULL) ? ChannelFilter : "";
    s_eConsoleOutputLevelFilter = LevelFilter;
}

static bool s_bDebugOutputEnabled = false;
static String s_strDebugOutputChannelFilter;
static LOGLEVEL s_eDebugOutputLevelFilter = LOGLEVEL_TRACE;

#if defined(Y_PLATFORM_WINDOWS)

static void DebugOutputLogCallback(void *UserParam, const char *ChannelName, LOGLEVEL Level, const char *Message)
{
    if (!s_bDebugOutputEnabled || Level > s_eDebugOutputLevelFilter || s_strDebugOutputChannelFilter.Find(ChannelName) >= 0)
        return;

    OutputDebugStringA(Message);
    OutputDebugStringA("\n");
}

#elif defined(Y_PLATFORM_POSIX)

static void DebugOutputLogCallback(void *UserParam, const char *ChannelName, LOGLEVEL Level, const char *Message)
{
}

#elif defined(Y_PLATFORM_HTML5)

static void DebugOutputLogCallback(void *UserParam, const char *ChannelName, LOGLEVEL Level, const char *Message)
{
}

#endif

void Log::SetDebugOutputParams(bool Enabled, const char *ChannelFilter, LOGLEVEL LevelFilter)
{
    if (s_bDebugOutputEnabled != Enabled)
    {
        s_bDebugOutputEnabled = Enabled;
        if (Enabled)
            RegisterCallback(DebugOutputLogCallback, NULL);
        else
            UnregisterCallback(DebugOutputLogCallback, NULL);
    }

    s_strDebugOutputChannelFilter = (ChannelFilter != NULL) ? ChannelFilter : "";
    s_eDebugOutputLevelFilter = LevelFilter;
}

void Log::ExecuteCallbacks(const char *ChannelName, LOGLEVEL Level, const char *Message)
{
    m_CallbackLock.Lock();

    for (RegisteredCallback &callback : m_liCallbacks)
        callback.Function(callback.Parameter, ChannelName, Level, Message);

    m_CallbackLock.Unlock();
}

void Log::Write(const char *ChannelName, LOGLEVEL Level, const char *Message)
{
    ExecuteCallbacks(ChannelName, Level, Message);
}

void Log::Writef(const char *ChannelName, LOGLEVEL Level, const char *Format, ...)
{
    char logFormatBuffer[512];

    va_list ap;
    va_start(ap, Format);
    Y_vsnprintf(logFormatBuffer, countof(logFormatBuffer), Format, ap);
    va_end(ap);

    ExecuteCallbacks(ChannelName, Level, logFormatBuffer);
}

void Log::Writev(const char *ChannelName, LOGLEVEL Level, const char *Format, va_list ArgPtr)
{
    char logFormatBuffer[512];
    Y_vsnprintf(logFormatBuffer, countof(logFormatBuffer), Format, ArgPtr);
    ExecuteCallbacks(ChannelName, Level, logFormatBuffer);
}
