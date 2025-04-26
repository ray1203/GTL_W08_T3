// InputManager.h
#pragma once

#include "InputCoreTypes.h"
#include <unordered_map>

class FInputKeyManager
{
public:
    static FInputKeyManager& Get();

    void Tick();

    void SetKeyDown(EKeys::Type Key);
    void SetKeyUp(EKeys::Type Key);

    bool GetKey(EKeys::Type Key) const;          // Held
    bool GetKeyDown(EKeys::Type Key) const;      // Just pressed
    bool GetKeyUp(EKeys::Type Key) const;        // Just released

    TMap<uint32, EKeys::Type> KeyMapVirtualToEnum;
    TMap<uint32, EKeys::Type> KeyMapCharToEnum;
private:
    FInputKeyManager();

    std::unordered_map<EKeys::Type, bool> PreviousKeyStates;
    std::unordered_map<EKeys::Type, bool> CurrentKeyStates;

    void InitializeKeyMappings();

};
