#pragma once  
#include "Container/String.h"  
#include "Container/Map.h"  
#include <memory> // Ensure this is included for std::unique_ptr  
#include <DirectXTK/Audio.h> // Corrected path for DirectX::AudioEngine and related classes  

class FSoundManager  
{  
public:  
  static FSoundManager& Instance();  

  void Initialize();  
  void Shutdown();  

  // SFX  
  void LoadSFX(const FString& name, const wchar_t* filePath);  
  void PlaySFX(const FString& name, float volume = 1.0f);  

  // BGM  
  void LoadBGM(const wchar_t* filePath);  
  void PlayBGM(float volume = 1.0f, bool loop = true);  
  void StopBGM();  
  void SetBGMVolume(float volume);  

  void Update();  

private:  
  FSoundManager();  
  ~FSoundManager();  

  std::unique_ptr<DirectX::AudioEngine> AudioEngine;  
  TMap<FString, std::unique_ptr<DirectX::SoundEffect>> SfxMap;  
  std::unique_ptr<DirectX::SoundEffect> BgmSound;  
  std::unique_ptr<DirectX::SoundEffectInstance> BgmInstance;  
};
