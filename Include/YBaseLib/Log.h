#pragma once

#include "YBaseLib/Common.h"
#include "YBaseLib/Singleton.h"
#include "YBaseLib/MemArray.h"
#include "YBaseLib/Mutex.h"

enum LOGLEVEL
{
    LOGLEVEL_NONE               = 0,        // Silences all log traffic
    LOGLEVEL_ERROR              = 1,        // "ErrorPrint"
    LOGLEVEL_WARNING            = 2,        // "WarningPrint"
    LOGLEVEL_SUCCESS            = 3,        // "SuccessPrint"
    LOGLEVEL_INFO               = 4,        // "LogPrint"
    LOGLEVEL_PERF               = 5,        // "PerfPrint"
    LOGLEVEL_DEV                = 6,        // "DevPrint"
    LOGLEVEL_PROFILE            = 7,        // "ProfilePrint"
    LOGLEVEL_TRACE              = 8,        // "TracePrint"
    LOGLEVEL_COUNT              = 9,
};

class Log : public Singleton<Log>
{
public:
    Log();
    ~Log();

    // log message callback type
    typedef void(*CallbackFunctionType)(void *UserParam, const char *ChannelName, LOGLEVEL Level, const char *Message);

    // registers a log callback
    void RegisterCallback(CallbackFunctionType CallbackFunction, void *UserParam);

    // unregisters a log callback
    void UnregisterCallback(CallbackFunctionType CallbackFunction, void *UserParam);

    // adds a standard console output
    void SetConsoleOutputParams(bool Enabled, const char *ChannelFilter = NULL, LOGLEVEL LevelFilter = LOGLEVEL_TRACE);

    // adds a debug console output [win32 only]
    void SetDebugOutputParams(bool Enabled, const char *ChannelFilter = NULL, LOGLEVEL LevelFilter = LOGLEVEL_TRACE);

    // writes a message to the log
    void Write(const char *ChannelName, LOGLEVEL Level, const char *Message);
    void Writef(const char *ChannelName, LOGLEVEL Level, const char *Format, ...);
    void Writev(const char *ChannelName, LOGLEVEL Level, const char *Format, va_list ArgPtr);

private:
    struct RegisteredCallback
    {
        CallbackFunctionType Function;
        void *Parameter;
    };

    typedef MemArray<RegisteredCallback> RegisteredCallbackList;
    RegisteredCallbackList m_liCallbacks;
    Mutex m_CallbackLock;

    void ExecuteCallbacks(const char *ChannelName, LOGLEVEL Level, const char *Message);
};

extern Log *g_pLog;

// log wrappers
#define Log_SetChannel(ChannelName) static const char *___LogChannel___ = #ChannelName;
#define Log_ErrorPrint(msg) Log::GetInstance().Write(___LogChannel___, LOGLEVEL_ERROR, msg)
#define Log_WarningPrint(msg) Log::GetInstance().Write(___LogChannel___, LOGLEVEL_WARNING, msg)
#define Log_SuccessPrint(msg) Log::GetInstance().Write(___LogChannel___, LOGLEVEL_SUCCESS, msg)
#define Log_InfoPrint(msg) Log::GetInstance().Write(___LogChannel___, LOGLEVEL_INFO, msg)
#define Log_PerfPrint(msg) Log::GetInstance().Write(___LogChannel___, LOGLEVEL_PERF, msg)
#define Log_DevPrint(msg) Log::GetInstance().Write(___LogChannel___, LOGLEVEL_DEV, msg)
#define Log_ProfilePrint(msg) Log::GetInstance().Write(___LogChannel___, LOGLEVEL_PROFILE, msg)
#define Log_TracePrint(msg) Log::GetInstance().Write(___LogChannel___, LOGLEVEL_TRACE, msg)
#define Log_ErrorPrintf(...) Log::GetInstance().Writef(___LogChannel___, LOGLEVEL_ERROR, __VA_ARGS__)
#define Log_WarningPrintf(...) Log::GetInstance().Writef(___LogChannel___, LOGLEVEL_WARNING, __VA_ARGS__)
#define Log_SuccessPrintf(...) Log::GetInstance().Writef(___LogChannel___, LOGLEVEL_SUCCESS, __VA_ARGS__)
#define Log_InfoPrintf(...) Log::GetInstance().Writef(___LogChannel___, LOGLEVEL_INFO, __VA_ARGS__)
#define Log_PerfPrintf(...) Log::GetInstance().Writef(___LogChannel___, LOGLEVEL_PERF, __VA_ARGS__)
#define Log_DevPrintf(...) Log::GetInstance().Writef(___LogChannel___, LOGLEVEL_DEV, __VA_ARGS__)
#define Log_ProfilePrintf(...) Log::GetInstance().Writef(___LogChannel___, LOGLEVEL_PROFILE, __VA_ARGS__)
#define Log_TracePrintf(...) Log::GetInstance().Writef(___LogChannel___, LOGLEVEL_TRACE, __VA_ARGS__)
