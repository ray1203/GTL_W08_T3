#include "FileHelper.h"
#include <fstream>

#include "Container/String.h"

bool FFileHelper::LoadFileToString(FString& Result, const TCHAR* Filename)
{
    std::ifstream file(Filename);
    if (!file.is_open())
        return false;

    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    Result = FString(content);
    return true;
}

bool FFileHelper::SaveStringToFile(const FString& Text, const TCHAR* Filename)
{
    std::ofstream file(Filename);
    if (!file.is_open())
        return false;

    std::string ansi = GetData(Text);
    file << ansi;
    return true;
}
