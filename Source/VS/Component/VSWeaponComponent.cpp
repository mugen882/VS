#include "Component/VSWeaponComponent.h"
#include "Manager/VSEnemyManager.h"
#include "Kismet/GameplayStatics.h"
#include "Weapon/VSProjectile.h"
#include "Weapon/Behavior/VSWeaponBehavior.h"
#include "Character/VSPlayerCharacter.h"

UVSWeaponComponent::UVSWeaponComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UVSWeaponComponent::BeginPlay()
{
    Super::BeginPlay();
    
}


void UVSWeaponComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    for (FVSWeaponInstance& W : Weapons)
    {
        if (W.Behavior)
        {
            W.Behavior->OnRemoved(this, W);
        }
	}

	Super::EndPlay(EndPlayReason);
}

void UVSWeaponComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    for (FVSWeaponInstance& W : Weapons)
    {
        if (W.Behavior)
        {
            W.Behavior->Tick(this, W, DeltaTime);
        }
    }   
}

FVector UVSWeaponComponent::GetFloorLocation() const
{
    AActor* Owner = GetOwner();
    if (!Owner) return FVector::ZeroVector;

    FVector Location = Owner->GetActorLocation();
	Location.Z -= Owner->GetSimpleCollisionHalfHeight();

    return Location;
}


const FVSPassiveStatModifiers& UVSWeaponComponent::GetStatMods() const
{
    if (AVSPlayerCharacter* PC = Cast<AVSPlayerCharacter>(GetOwner()))
        return PC->GetStatMods();

    static const FVSPassiveStatModifiers Empty;
    return Empty;
}

void UVSWeaponComponent::AddWeapon(UVSWeaponData* WeaponData)
{
    if (!WeaponData) return;

    FVSWeaponInstance W;
    W.Data = WeaponData;
    W.Level = 1;
    if (WeaponData->BehaviorClass)
        W.Behavior = NewObject<UVSWeaponBehavior>(this, WeaponData->BehaviorClass);

    const int32 Index = Weapons.Add(MoveTemp(W));
    FVSWeaponInstance& Added = Weapons[Index];
    if (Added.Behavior)
        Added.Behavior->OnAdded(this, Added);
}

bool UVSWeaponComponent::UpgradeWeaponByData(UVSWeaponData* WeaponData)
{
    for (FVSWeaponInstance& W : Weapons)
    {
        if (W.Data == WeaponData)
        {
            // 상한 도달. 여기서 막아두면 업그레이드 카드·치트 어느 경로로 들어와도 안전하다.
            if (W.Level >= MAX_WEAPON_LEVEL)
                return false;

            W.Level++;
            if (W.Behavior) W.Behavior->OnUpgraded(this, W);
            return true;
        }
    }
        
    return false;
}

int32 UVSWeaponComponent::GetWeaponLevel(UVSWeaponData* WeaponData) const
{
    for (const FVSWeaponInstance& W : Weapons)
    {
        if (W.Data == WeaponData)
        {
            return W.Level;
        }
    }

    return 0;
}

bool UVSWeaponComponent::IsWeaponMaxLevel(UVSWeaponData* WeaponData) const
{
    const int32 Level = GetWeaponLevel(WeaponData);
    return Level > 0 && Level >= MAX_WEAPON_LEVEL;
}

bool UVSWeaponComponent::IsCooldownSaturated() const
{
    const FVSPassiveStatModifiers& Mods = GetStatMods();
    bool bHasAnyWeapon = false;

    for (const FVSWeaponInstance& W : Weapons)
    {
        if (!W.Data) continue;
        bHasAnyWeapon = true;

        // 여유가 남은 무기가 하나라도 있으면 감소 패시브는 아직 유효하다.
        if (W.GetCooldown(Mods) > MIN_COOLDOWN_TIME + UE_KINDA_SMALL_NUMBER)
            return false;
    }

    // 무기가 없으면 판단 근거가 없다. 앞으로 얻을 무기에 적용되므로 막지 않는다.
    return bHasAnyWeapon;
}

bool UVSWeaponComponent::HasWeapon(UVSWeaponData* WeaponData) const
{
    for (const FVSWeaponInstance& W : Weapons)
    {
        if (W.Data == WeaponData)
        {
            return true;
        }
    }
        
    return false;
}

void UVSWeaponComponent::ApplyContinuousDamage(const FVector& Center, float Radius, float DamagePerSecond, float DeltaTime)
{
    if (!EnemyManager.IsValid()) return;

    EnemyManager->ApplyDamageInRadius(Center, Radius, DamagePerSecond * DeltaTime);
}