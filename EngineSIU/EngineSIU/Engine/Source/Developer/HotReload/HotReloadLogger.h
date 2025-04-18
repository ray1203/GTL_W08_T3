#pragma once
#include "ICompilerLogger.h"


/**
 * Hot Reload시 사용될 Logging 클래스 입니다.
 */
class FHotReloadLogger : public ICompilerLogger
{
public:
    FHotReloadLogger() = default;

    virtual void LogError(const char* format, ...) override;
    virtual void LogWarning(const char* format, ...) override;
    virtual void LogInfo(const char* format, ...) override;
};
