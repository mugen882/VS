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
            // 상한에 도달한 무기의 '강화'는 제외
            if (U->Type == EVSUpgradeType::UpgradeWeapon && WC->IsWeaponMaxLevel(U->TargetWeapon))
                continue;
        }

        if (Player && U->Type == EVSUpgradeType::Passive)
        {
            // 스택 상한에 도달한 패시브는 제외
            if (Player->GetStatMods().IsMaxLevel(U->PassiveStatType))
                continue;

            // 상한 전이라도 효과가 포화됐으면 제외 (더 찍어도 수치가 안 변함)
            if (IsPassiveSaturated(U->PassiveStatType))
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

bool UVSUpgradeComponent::IsPassiveSaturated(EVSPassiveStatType StatType) const
{
    AVSPlayerCharacter* Player = Cast<AVSPlayerCharacter>(GetOwner());
    const UVSWeaponComponent* WC = Player ? Player->GetWeaponComponent() : nullptr;
    if (!WC) return false;

    switch (StatType)
    {
    case EVSPassiveStatType::GlobalCooldown:
        // 보유 무기가 전부 MIN_COOLDOWN_TIME에 걸려 있으면 더 깎아도 발사 간격이 그대로다.
        return WC->IsCooldownSaturated();

    default:
        // 나머지 스탯은 상한 클램프가 없어 항상 유효하다.
        // 새 클램프를 추가하면 여기에 case를 늘린다.
        return false;
    }
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
        break;

    case EVSUpgradeType::UpgradeWeapon:
        if (!WeaponComp->UpgradeWeaponByData(Upgrade->TargetWeapon))
        {
            // RollUpgrades가 걸러주므로 정상 흐름에서는 안 나온다.
            UE_LOG(VSLog, Warning, TEXT("Upgrade: %s 강화 실패 (미보유 또는 Lv.%d 상한)"),
                *Upgrade->Title.ToString(), MAX_WEAPON_LEVEL);
        }
        break;

    case EVSUpgradeType::Passive:
        if (!Player->AddPassive(Upgrade->PassiveStatType, Upgrade->PassiveValue))
        {
            UE_LOG(VSLog, Warning, TEXT("Upgrade: Passive %s 상한(Lv.%d) 도달"),
                *UEnum::GetValueAsString(Upgrade->PassiveStatType), MAX_PASSIVE_LEVEL);
        }
        break;
    }
}

// --- 치트 ---
void UVSUpgradeComponent::CheatGiveAllWeapons(int32 TargetLevel)
{
    TargetLevel = FMath::Clamp(TargetLevel, 1, MAX_WEAPON_LEVEL);

    AVSPlayerCharacter* Player = Cast<AVSPlayerCharacter>(GetOwner());
    UVSWeaponComponent* WC = Player ? Player->GetWeaponComponent() : nullptr;
    if (!WC)
    {
        UE_LOG(VSLog, Warning, TEXT("VSAllSkillUp: UVSWeaponComponent를 찾을 수 없습니다."));
        return;
    }

    // NewWeapon 무기를 전부 획득
    int32 AddedCount = 0;
    for (UVSUpgradeData* U : AllUpgrades)
    {
        if (!U || U->Type != EVSUpgradeType::NewWeapon || !U->TargetWeapon)
            continue;

        if (WC->HasWeapon(U->TargetWeapon))
            continue;

        WC->AddWeapon(U->TargetWeapon);
        ++AddedCount;
    }

    // 보유 무기를 각각 TargetLevel까지 강화
    TArray<TPair<UVSWeaponData*, int32>> Targets;
    Targets.Reserve(WC->GetWeapons().Num());
    for (const FVSWeaponInstance& W : WC->GetWeapons())
    {
        if (W.Data)
            Targets.Emplace(W.Data, W.Level);
    }

    for (const TPair<UVSWeaponData*, int32>& Target : Targets)
    {
        // UpgradeWeaponByData는 1레벨씩만 올리므로 차이만큼 반복한다.
        // 각 호출이 Behavior::OnUpgraded를 태우므로 오브 추가·실드 반경 갱신도 함께 반영된다.
        for (int32 Level = Target.Value; Level < TargetLevel; ++Level)
        {
            WC->UpgradeWeaponByData(Target.Key);
        }
    }

    UE_LOG(VSLog, Warning,
        TEXT("VSAllSkillUp: 무기 %d종 신규 획득, 보유 무기 %d종 전부 Lv.%d"),
        AddedCount, Targets.Num(), TargetLevel);

    for (const FVSWeaponInstance& W : WC->GetWeapons())
    {
        if (W.Data)
        {
            UE_LOG(VSLog, Warning, TEXT("  - %s Lv.%d"), *W.Data->WeaponName, W.Level);
        }
    }
}

void UVSUpgradeComponent::CheatGiveAllPassives(int32 TargetLevel)
{
    TargetLevel = FMath::Clamp(TargetLevel, 1, MAX_PASSIVE_LEVEL);

    AVSPlayerCharacter* Player = Cast<AVSPlayerCharacter>(GetOwner());
    if (!Player)
    {
        UE_LOG(VSLog, Warning, TEXT("VSGiveAllPassive: AVSPlayerCharacter를 찾을 수 없습니다."));
        return;
    }

    // 같은 StatType카드가 여러 장 있으면 증가량이 가장 큰 카드를 찾아 적용한다.
    TMap<EVSPassiveStatType, float> BestValues;
    for (UVSUpgradeData* U : AllUpgrades)
    {
        if (!U || U->Type != EVSUpgradeType::Passive)
            continue;

		// Find성공하면 비교후 업데이트, 실패하면 새로 추가
        if (float* Existing = BestValues.Find(U->PassiveStatType))
        {
			// 기존 값보다 절댓값이 큰 경우만 갱신한다.
            if (FMath::Abs(U->PassiveValue) > FMath::Abs(*Existing))
            {
                *Existing = U->PassiveValue;   // 부호는 원본 유지
            }   
        }
        else
        {
            BestValues.Add(U->PassiveStatType, U->PassiveValue);
        }
    }

    if (BestValues.Num() == 0)
    {
        UE_LOG(VSLog, Warning, TEXT("VSGiveAllPassive: AllUpgrades에 Passive 항목이 없습니다."));
        return;
    }

    for (const TPair<EVSPassiveStatType, float>& Entry : BestValues)
    {
        // AddPassive가 상한에서 false를 반환하므로 무한 루프에 빠지지 않는다.
        while (Player->GetStatMods().GetLevel(Entry.Key) < TargetLevel)
        {
            if (!Player->AddPassive(Entry.Key, Entry.Value))
                break;
        }
    }

    UE_LOG(VSLog, Warning, TEXT("VSGiveAllPassive: 패시브 %d종 -> Lv.%d"),
        BestValues.Num(), TargetLevel);

    const FVSPassiveStatModifiers& Mods = Player->GetStatMods();
    for (const TPair<EVSPassiveStatType, float>& Entry : BestValues)
    {
        UE_LOG(VSLog, Warning, TEXT("  - %s Lv.%d (스택 %+.2f, 누적 %.2f)"),
            *UEnum::GetValueAsString(Entry.Key),
            Mods.GetLevel(Entry.Key),
            Entry.Value,
            Mods.Get(Entry.Key));
    }
}
