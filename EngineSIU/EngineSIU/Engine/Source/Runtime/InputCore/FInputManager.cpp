// InputManager.cpp

#include "FInputManager.h"

FInputKeyManager& FInputKeyManager::Get()
{
    static FInputKeyManager Instance;
    return Instance;
}
FInputKeyManager::FInputKeyManager()
{
    InitializeKeyMappings();
}
void FInputKeyManager::Tick()
{
    PreviousKeyStates = CurrentKeyStates;
}

void FInputKeyManager::SetKeyDown(EKeys::Type Key)
{
    CurrentKeyStates[Key] = true;
}

void FInputKeyManager::SetKeyUp(EKeys::Type Key)
{
    CurrentKeyStates[Key] = false;
}

bool FInputKeyManager::GetKey(EKeys::Type Key) const
{
    auto it = CurrentKeyStates.find(Key);
    return it != CurrentKeyStates.end() && it->second;
}

bool FInputKeyManager::GetKeyDown(EKeys::Type Key) const
{
    auto cur = CurrentKeyStates.find(Key);
    auto prev = PreviousKeyStates.find(Key);
    return (cur != CurrentKeyStates.end() && cur->second) &&
        (prev == PreviousKeyStates.end() || !prev->second);
}

bool FInputKeyManager::GetKeyUp(EKeys::Type Key) const
{
    auto cur = CurrentKeyStates.find(Key);
    auto prev = PreviousKeyStates.find(Key);
    return (cur == CurrentKeyStates.end() || !cur->second) &&
        (prev != PreviousKeyStates.end() && prev->second);
}

void FInputKeyManager::InitializeKeyMappings()
{
    KeyMapVirtualToEnum[0x41] = EKeys::A;
    KeyMapVirtualToEnum[0x42] = EKeys::B;
    KeyMapVirtualToEnum[0x43] = EKeys::C;
    KeyMapVirtualToEnum[0x44] = EKeys::D;
    KeyMapVirtualToEnum[0x45] = EKeys::E;
    KeyMapVirtualToEnum[0x46] = EKeys::F;
    KeyMapVirtualToEnum[0x47] = EKeys::G;
    KeyMapVirtualToEnum[0x48] = EKeys::H;
    KeyMapVirtualToEnum[0x49] = EKeys::I;
    KeyMapVirtualToEnum[0x4A] = EKeys::J;
    KeyMapVirtualToEnum[0x4B] = EKeys::K;
    KeyMapVirtualToEnum[0x4C] = EKeys::L;
    KeyMapVirtualToEnum[0x4D] = EKeys::M;
    KeyMapVirtualToEnum[0x4E] = EKeys::N;
    KeyMapVirtualToEnum[0x4F] = EKeys::O;
    KeyMapVirtualToEnum[0x50] = EKeys::P;
    KeyMapVirtualToEnum[0x51] = EKeys::Q;
    KeyMapVirtualToEnum[0x52] = EKeys::R;
    KeyMapVirtualToEnum[0x53] = EKeys::S;
    KeyMapVirtualToEnum[0x54] = EKeys::T;
    KeyMapVirtualToEnum[0x55] = EKeys::U;
    KeyMapVirtualToEnum[0x56] = EKeys::V;
    KeyMapVirtualToEnum[0x57] = EKeys::W;
    KeyMapVirtualToEnum[0x58] = EKeys::X;
    KeyMapVirtualToEnum[0x59] = EKeys::Y;
    KeyMapVirtualToEnum[0x5A] = EKeys::Z;
    KeyMapVirtualToEnum[0x20] = EKeys::SpaceBar;

    KeyMapCharToEnum['A'] = EKeys::A;
    KeyMapCharToEnum['B'] = EKeys::B;
    KeyMapCharToEnum['C'] = EKeys::C;
    KeyMapCharToEnum['D'] = EKeys::D;
    KeyMapCharToEnum['E'] = EKeys::E;
    KeyMapCharToEnum['F'] = EKeys::F;
    KeyMapCharToEnum['G'] = EKeys::G;
    KeyMapCharToEnum['H'] = EKeys::H;
    KeyMapCharToEnum['I'] = EKeys::I;
    KeyMapCharToEnum['J'] = EKeys::J;
    KeyMapCharToEnum['K'] = EKeys::K;
    KeyMapCharToEnum['L'] = EKeys::L;
    KeyMapCharToEnum['M'] = EKeys::M;
    KeyMapCharToEnum['N'] = EKeys::N;
    KeyMapCharToEnum['O'] = EKeys::O;
    KeyMapCharToEnum['P'] = EKeys::P;
    KeyMapCharToEnum['Q'] = EKeys::Q;
    KeyMapCharToEnum['R'] = EKeys::R;
    KeyMapCharToEnum['S'] = EKeys::S;
    KeyMapCharToEnum['T'] = EKeys::T;
    KeyMapCharToEnum['U'] = EKeys::U;
    KeyMapCharToEnum['V'] = EKeys::V;
    KeyMapCharToEnum['W'] = EKeys::W;
    KeyMapCharToEnum['X'] = EKeys::X;
    KeyMapCharToEnum['Y'] = EKeys::Y;
    KeyMapCharToEnum['Z'] = EKeys::Z;
    KeyMapCharToEnum[' '] = EKeys::SpaceBar;
}
