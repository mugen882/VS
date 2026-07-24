#include "VSUpgradeComponent.h"
#include "Data/VSUpgradeData.h"
#include "VSWeaponComponent.h"
#include "Character/VSPlayerCharacter.h"
#include "Common/VSLog.h"

TArray<UVSUpgradeData*> UVSUpgradeComponent::RollUpgrades()
{
    TArray<UVSUpgradeData*> Result;
    TArray<UVSUpgradeData*> Pool;

    AVSPlayerCharacter* Player = Cast<AVSPlayerCharacter>(GetOwner());
    UVSWeaponComponent* WC = Player ? Player->GetWeaponComponent() : nullptr;

    for (UVSUpgradeData* U : AllUpgrades)
    {
        if (!U) continue;

        if (WC)
        {
            // 이미 가진 무기의 '획득'은 제외
            if (U->Type == EVSUpgradeType::NewWeapon && WC->HasWeapon(U->TargetWeapon))
                continue;
            // 안 가진 무기의 '강화'는 제외
            if (U->Type == EVSUpgradeType::UpgradeWeapon && !WC->HasWeapon(U->TargetWeapon))
                continue;
        }
        Pool.Add(U);
    }

    for (int32 i = 0; i < ChoiceCount && Pool.Num() > 0; ++i)
    {
        const int32 Idx = FMath::RandRange(0, Pool.Num() - 1);
        Result.Add(Pool[Idx]);

        // 재선택 안되게 제거
        Pool.RemoveAtSwap(Idx);
    }

    return Result;
}

void UVSUpgradeComponent::ApplyUpgrade(UVSUpgradeData* Upgrade)
{
    if (!Upgrade) return;

    AVSPlayerCharacter* Player = Cast<AVSPlayerCharacter>(GetOwner());
    if (!Player) return;

    UVSWeaponComponent* WeaponComp = Player->GetWeaponComponent();
    if (!WeaponComp) return;

    switch (Upgrade->Type)
    {
    case EVSUpgradeType::NewWeapon:
        WeaponComp->AddWeapon(Upgrade->TargetWeapon);
        UE_LOG(VSLog, Warning, TEXT("Upgrade: New Weapon %s"), *Upgrade->Title.ToString());
        break;

    case EVSUpgradeType::UpgradeWeapon:
        if (WeaponComp->UpgradeWeaponByData(Upgrade->TargetWeapon))
        {
            UE_LOG(VSLog, Warning, TEXT("Upgrade: Level up %s"), *Upgrade->Title.ToString());
        }
        break;

    case EVSUpgradeType::Passive:
        Player->AddPassive(Upgrade->PassiveStat, Upgrade->PassiveValue);
        UE_LOG(VSLog, Warning, TEXT("Upgrade: Passive %s +%.2f"),
            *UEnum::GetValueAsString(Upgrade->PassiveStat), Upgrade->PassiveValue);
        break;
    }
}