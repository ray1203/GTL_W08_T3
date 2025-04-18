#include "Core/HAL/PlatformType.h"
#include "EngineLoop.h"
#include "Developer/HotReload/HotReload.h"

FEngineLoop GEngineLoop;


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    // 사용 안하는 파라미터들
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    UNREFERENCED_PARAMETER(nShowCmd);

    FHotReload::Initialize();

    GEngineLoop.Init(hInstance);
    GEngineLoop.Tick();
    GEngineLoop.Exit();

    FHotReload::Shutdown();

    return 0;
}
