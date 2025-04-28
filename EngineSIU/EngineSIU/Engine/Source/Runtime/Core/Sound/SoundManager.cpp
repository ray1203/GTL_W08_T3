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

    LoadBGM(L"Contents/Sounds/elinia.wav");
    LoadSFX(TEXT("Jump"), L"Contents/Sounds/jump.wav");
}

void FSoundManager::Shutdown()
{
    SfxMap.Empty();
}

void FSoundManager::LoadSFX(const FString& name, const wchar_t* filePath)
{
    if (!AudioEngine)
        return;

    SfxMap[name] = std::make_unique<DirectX::SoundEffect>(AudioEngine.get(), filePath);
}

void FSoundManager::PlaySFX(const FString& name, float volume)
{
    auto sfx = SfxMap.Find(name);

    if (sfx && AudioEngine)
    {
        auto SoundEffect = sfx->get();
        auto Instance = SoundEffect->CreateInstance();
        Instance->SetVolume(volume);
        Instance->Play();
        SfxInstances.Add(std::move(Instance));
    }
}

void FSoundManager::LoadBGM(const wchar_t* filePath)
{
    if (!AudioEngine)
        return;
    BgmSound = std::make_unique<DirectX::SoundEffect>(AudioEngine.get(), filePath);
    BgmInstance = BgmSound->CreateInstance();
}

void FSoundManager::PlayBGM(float volume, bool loop)
{
    if (BgmInstance)
    {
        BgmInstance->SetVolume(volume);
        BgmInstance->Play();
    }
}

void FSoundManager::StopBGM()
{
    if (BgmInstance)
    {
        BgmInstance->Stop();
    }
}

void FSoundManager::SetBGMVolume(float volume)
{
    if (BgmInstance)
    {
        BgmInstance->SetVolume(volume);
    }
}

void FSoundManager::Update()
{
    if (AudioEngine)
    {
        AudioEngine->Update();
    }

    // Remove finished SFX instances
    for (int i = SfxInstances.Num() - 1; i >= 0; i--)
    {
        if (SfxInstances[i]->GetState() == DirectX::SoundState::STOPPED)
        {
            SfxInstances.RemoveAt(i);
        }
    }
}



