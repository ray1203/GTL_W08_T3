#pragma once
#include "HotReloadLogger.h"

struct IRuntimeObjectSystem;


/**
 * 런타임에 C++를 Hot Reload 할 수 있는
 * RCCpp 라이브러리 Wrapper 입니다.
 */
class FHotReload
{
    FHotReload() = default;

public:
    static bool Initialize();
    static void Shutdown();

private:
    // RCC++ Data
    static IRuntimeObjectSystem* RuntimeObjectSystemInstance;
    static FHotReloadLogger RCLogger;
};
