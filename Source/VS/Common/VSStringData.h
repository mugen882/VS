#pragma once

#include "CoreMinimal.h"

// UI 표시용 고정 문자열 모음.
namespace VSString
{
    inline const FString& Gain()    { static const FString S = TEXT("획득"); return S; }
    inline const FString& Upgrade() { static const FString S = TEXT("강화"); return S; }

    inline const FText& SurvivalSecFormat()  { static const FText T = FText::FromString(TEXT("생존시간 : {0}")); return T; }
    inline const FText& KillCountFormat()    { static const FText T = FText::FromString(TEXT("KillCount : {0}")); return T; }
    inline const FText& ReachedLevelFormat() { static const FText T = FText::FromString(TEXT("레벨 : {0}")); return T; }
    inline const FText& ReachedWaveFormat()  { static const FText T = FText::FromString(TEXT("웨이브 : {0}")); return T; }
    inline const FText& VictoryTitle()       { static const FText T = FText::FromString(TEXT("YOU WIN")); return T; }
    inline const FText& DefeatTitle()        { static const FText T = FText::FromString(TEXT("GAME OVER")); return T; }
}
