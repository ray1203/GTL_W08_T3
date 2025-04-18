#include "HotReload.h"
#include "Container/Array.h"
#include "RuntimeObjectSystem.h"

SystemTable FHotReload::SystemTableInstance = SystemTable();

IRuntimeObjectSystem* FHotReload::RuntimeObjectSystemInstance = nullptr;
FHotReloadLogger FHotReload::RCLogger = FHotReloadLogger();


bool FHotReload::Initialize()
{
    // RCC++ Initialization
    RuntimeObjectSystemInstance = new RuntimeObjectSystem;
    if (!RuntimeObjectSystemInstance->Initialise(&RCLogger, &SystemTableInstance))
    {
        delete RuntimeObjectSystemInstance;
        RuntimeObjectSystemInstance = nullptr;
        return false;
    }

    // ensure include directories are set - use location of this file as starting point
    const FileSystemUtils::Path BasePath =
        RuntimeObjectSystemInstance->FindFile( __FILE__ )
        .ParentPath()
        .ParentPath()
        .ParentPath()
        .ParentPath()
        .ParentPath();

    TArray Includes = 
    {
        BasePath,
        BasePath / "Engine/Source",
        BasePath / "Engine/Source/Editor",
        BasePath / "Engine/Source/Runtime",
        BasePath / "Engine/Source/Runtime/Core",
        BasePath / "Engine/Source/Runtime/CoreUObject",
        BasePath / "Engine/Source/Runtime/Engine",
        BasePath / "Engine/Source/Runtime/Engine/Classes",
        BasePath / "Engine/Source/Runtime/InputCore",
        BasePath / "Engine/Source/Runtime/InteractiveToolsFramework",
        BasePath / "Engine/Source/Runtime/Launch",
        BasePath / "Engine/Source/Runtime/Renderer",
        BasePath / "Engine/Source/Runtime/Serialization",
        BasePath / "Engine/Source/Runtime/Slate",
        BasePath / "Engine/Source/Runtime/SlateCore",
        BasePath / "Engine/Source/Runtime/Windows",
        BasePath / "Engine/Source/ThirdParty/tinyfiledialogs/include",
        BasePath / "Engine/Source/ThirdParty/Json/include",
        BasePath / "Engine/Source/ThirdParty/DirectXTK/Include",
        BasePath / "Engine/Source/ThirdParty/ImGui/include",
    };

    for (const auto& Include : Includes)
    {
        RuntimeObjectSystemInstance->AddIncludeDir( Include.c_str() );
    }

    return true;
}

void FHotReload::Shutdown()
{
    delete RuntimeObjectSystemInstance;
    RuntimeObjectSystemInstance = nullptr;
}

void FHotReload::Update(float DeltaTime)
{
    if (!IsInitialized()) return;

    if (RuntimeObjectSystemInstance->GetIsCompiledComplete())
    {
        RuntimeObjectSystemInstance->LoadCompiledModule();
    }

    if (!RuntimeObjectSystemInstance->GetIsCompiling())
    {
        RuntimeObjectSystemInstance->GetFileChangeNotifier()->Update(DeltaTime);
    }
}
