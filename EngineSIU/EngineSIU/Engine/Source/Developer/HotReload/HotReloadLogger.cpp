#include "HotReloadLogger.h"
#include "UserInterface/Console.h"


void FHotReloadLogger::LogError(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    UE_LOG(LogLevel::Error, format, args);
    va_end(args);
}

void FHotReloadLogger::LogWarning(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    UE_LOG(LogLevel::Warning, format, args);
    va_end(args);
}

void FHotReloadLogger::LogInfo(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    UE_LOG(LogLevel::Display, format, args);
    va_end(args);
}
