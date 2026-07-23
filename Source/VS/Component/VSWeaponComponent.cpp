#include "Component/VSWeaponComponent.h"
#include "Manager/VSEnemyManager.h"
#include "Kismet/GameplayStatics.h"
#include "Weapon/VSProjectile.h"
#include "Weapon/VSDrone.h"
#include "Weapon/VSShieldAura.h"
#include "Weapon/Behavior/VSWeaponBehavior.h"
#include "Character/VSCharacter.h"

UVSWeaponComponent::UVSWeaponComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UVSWeaponComponent::BeginPlay()
{
    Super::BeginPlay();
    
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


const FVSStatModifiers& UVSWeaponComponent::GetStatMods() const
{
    if (AVSCharacter* PC = Cast<AVSCharacter>(GetOwner()))
        return PC->GetStatMods();

    static const FVSStatModifiers Empty;
    return Empty;
}

void UVSWeaponComponent::AddWeapon(UVSWeaponData* WeaponData)
{
    FVSWeaponInstance W;
    W.Data = WeaponData;
    W.Level = 1;
    if (WeaponData->BehaviorClass)
        W.Behavior = NewObject<UVSWeaponBehavior>(this, WeaponData->BehaviorClass);
    if (W.Behavior) W.Behavior->OnAdded(this, W);
    Weapons.Add(W);
}

void UVSWeaponComponent::UpgradeWeapon(int32 WeaponIndex)
{
    if (Weapons.IsValidIndex(WeaponIndex))
        Weapons[WeaponIndex].Level++;
}

bool UVSWeaponComponent::UpgradeWeaponByData(UVSWeaponData* WeaponData)
{
    for (FVSWeaponInstance& W : Weapons)
        if (W.Data == WeaponData)
        {
            W.Level++;
            if (W.Behavior) W.Behavior->OnUpgraded(this, W);
            return true;
        }
    return false;
}

bool UVSWeaponComponent::HasWeapon(UVSWeaponData* WeaponData) const
{
    for (const FVSWeaponInstance& W : Weapons)
        if (W.Data == WeaponData) return true;
    return false;
}

void UVSWeaponComponent::ApplyContinuousDamage(const FVector& Center, float Radius, float DamagePerSecond, float DeltaTime)
{
    if (!EnemyManager.IsValid()) return;

    EnemyManager->ApplyDamageInRadius(Center, Radius, DamagePerSecond * DeltaTime);
}