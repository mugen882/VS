#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Character/VSStatModifiers.h"
#include "VSUpgradeData.generated.h"

class UVSWeaponData;

UENUM(BlueprintType)
enum class EVSUpgradeType : uint8
{
    NewWeapon      UMETA(DisplayName="새 무기 획득"),
    UpgradeWeapon  UMETA(DisplayName="무기 강화"),
    Passive        UMETA(DisplayName="패시브")
};

UCLASS(BlueprintType)
class VS_API UVSUpgradeData : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    // --- UI 표시용 ---
    UPROPERTY(EditAnywhere, Category="Display")
    FText Title;

    UPROPERTY(EditAnywhere, Category="Display", meta=(MultiLine=true))
    FText Description;

    UPROPERTY(EditAnywhere, Category="Display")
    TObjectPtr<UTexture2D> Icon;

    // --- 타입 ---
    UPROPERTY(EditAnywhere, Category="Upgrade")
    EVSUpgradeType Type = EVSUpgradeType::NewWeapon;

    // --- NewWeapon / UpgradeWeapon 공용: 어떤 무기냐 ---
    UPROPERTY(EditAnywhere, Category="Upgrade",
        meta=(EditCondition="Type != EVSUpgradeType::Passive", EditConditionHides))
    TObjectPtr<UVSWeaponData> TargetWeapon;

    // --- Passive: 어떤 스탯을 얼마나 ---
    UPROPERTY(EditAnywhere, Category="Upgrade",
        meta=(EditCondition="Type == EVSUpgradeType::Passive", EditConditionHides))
    EVSStatType PassiveStat = EVSStatType::MoveSpeed;

    UPROPERTY(EditAnywhere, Category="Upgrade",
        meta=(EditCondition="Type == EVSUpgradeType::Passive", EditConditionHides))
    float PassiveValue = 0.f;
};