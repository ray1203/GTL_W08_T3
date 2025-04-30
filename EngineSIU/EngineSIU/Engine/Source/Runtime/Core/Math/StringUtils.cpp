#include "StringUtils.h"
void StringUtils::ParseIntoArrayLines(const FString& Source, TArray<FString>& OutLines)
{
    int32 Start = 0;
    while (Start < Source.Len())
    {
        int32 NewlineIdx = Source.Find(TEXT("\n"), ESearchCase::IgnoreCase, ESearchDir::FromStart, Start);
        if (NewlineIdx == -1)
        {
            FString Remaining = Source.Mid(Start);
            if (!Remaining.IsEmpty())
            {
                // 맨 끝에 \r 있으면 제거
                if (Remaining.Len() > 0 && Remaining[Remaining.Len() - 1] == '\r')
                {
                    Remaining = Remaining.Left(Remaining.Len() - 1);
                }
                OutLines.Add(Remaining);
            }
            break;
        }

        FString Line = Source.Mid(Start, NewlineIdx - Start);
        // \r 제거
        if (Line.Len() > 0 && Line[Line.Len() - 1] == '\r')
        {
            Line = Line.Left(Line.Len() - 1);
        }
        OutLines.Add(Line);

        Start = NewlineIdx + 1;
    }
}



void StringUtils::ParseIntoArray(const FString& Source, TArray<FString>& OutArray, const FString& Delimiter, bool bCullEmpty)
{
    int32 Start = 0;
    while (Start < Source.Len())
    {
        int32 DelimIdx = Source.Find(Delimiter, ESearchCase::IgnoreCase, ESearchDir::FromStart, Start);
        if (DelimIdx == -1)
        {
            FString Token = Source.Mid(Start);
            if (!bCullEmpty || !Token.IsEmpty())
                OutArray.Add(Token);
            break;
        }

        FString Token = Source.Mid(Start, DelimIdx - Start);
        if (!bCullEmpty || !Token.IsEmpty())
            OutArray.Add(Token);

        Start = DelimIdx + Delimiter.Len();
    }
}
