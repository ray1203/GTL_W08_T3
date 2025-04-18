#include "HotReloadLogger.h"
#include "UserInterface/Console.h"

static void LogInternal(LogLevel Level, const char* Format, va_list Args)
{
    UE_LOG(Level, Format, Args);
}

void FHotReloadLogger::LogError(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    LogInternal(LogLevel::Error, format, args);
    va_end(args);
}

void FHotReloadLogger::LogWarning(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    LogInternal(LogLevel::Warning, format, args);
    va_end(args);
}

void FHotReloadLogger::LogInfo(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    LogInternal(LogLevel::Display, format, args);
    va_end(args);
}
