#pragma once
#include "HotReloadLogger.h"
#include "HotReloadSystemTable.h"

struct IRuntimeObjectSystem;


/**
 * 런타임에 C++를 Hot Reload 할 수 있는
 * RCCpp 라이브러리 Wrapper 입니다.
 *
 * @note https://github.com/dougbinks/RCCpp_DX11_Example
 */
class FHotReload
{
    FHotReload() = default;

public:
    static bool Initialize();
    static void Shutdown();

    static void Update(float DeltaTime);

public:
    static bool IsInitialized()
    {
        return RuntimeObjectSystemInstance != nullptr;
    }

    static SystemTable& GetSystemTable()
    {
        return SystemTableInstance;
    }

private:
    // RCC++ Data
    static SystemTable SystemTableInstance;

    static IRuntimeObjectSystem* RuntimeObjectSystemInstance;
    static FHotReloadLogger RCLogger;
};
