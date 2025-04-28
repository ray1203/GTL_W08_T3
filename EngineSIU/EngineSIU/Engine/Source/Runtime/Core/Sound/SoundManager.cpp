#include "SoundManager.h"
#include <UserInterface/Console.h>

FSoundManager& FSoundManager::Instance()
{
    static FSoundManager instance;
    return instance;
}

void FSoundManager::Initialize()
{
    AudioEngine = std::make_unique<DirectX::AudioEngine>();
    if (!AudioEngine)
    {
        UE_LOG(ELogLevel::Error, TEXT("Failed to create AudioEngine"));
        return;
    }
}

void FSoundManager::Shutdown()
{
    SfxMap.Empty();
    BgmSound.reset();
    BgmInstance.reset();
    AudioEngine.reset();
}

void FSoundManager::LoadSFX(const FString& name, const wchar_t* filePath)
{
    if (!AudioEngine)
        return;

    SfxMap[name] = std::make_unique<DirectX::SoundEffect>(AudioEngine.get(), filePath);
}

void FSoundManager::PlaySFX(const FString& name, float volume)
{
    auto it = SfxMap.Find(name);
}

void FSoundManager::LoadBGM(const wchar_t* filePath)
{
}

void FSoundManager::PlayBGM(float volume, bool loop)
{
}

void FSoundManager::StopBGM()
{
}

void FSoundManager::SetBGMVolume(float volume)
{
}

void FSoundManager::Update()
{
}

FSoundManager::FSoundManager()
{
}

FSoundManager::~FSoundManager()
{
    Shutdown();
}

