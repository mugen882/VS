#include "Weapon/Behavior/VSOrbitBehavior.h"
#include "Component/VSWeaponComponent.h"
#include "Character/VSPlayerCharacter.h"

UVSOrbitBehavior::UVSOrbitBehavior()
{
	
}

void UVSOrbitBehavior::OnAdded(UVSWeaponComponent* Comp, FVSWeaponInstance& W)
{
    SpawnSingleBall(Comp, W);
}

void UVSOrbitBehavior::OnUpgraded(UVSWeaponComponent* Comp, FVSWeaponInstance& W)
{
    SpawnSingleBall(Comp, W);
}

void UVSOrbitBehavior::OnRemoved(UVSWeaponComponent* Comp, FVSWeaponInstance& W)
{
    for (AVSOrbitProjectile* Ball : OrbitBalls)
    {
        if (IsValid(Ball))
        {
            Ball->Destroy();
        }
    }
    OrbitBalls.Empty();
}

void UVSOrbitBehavior::Tick(UVSWeaponComponent* Comp, FVSWeaponInstance& W, float DeltaTime)
{
    if (!W.Data) return;

    // 회전 각도 증가
    OrbitAngle = FMath::Fmod(OrbitAngle + W.Data->OrbitConfig.Speed * DeltaTime, 360.f);

    PositionBalls(Comp, W);

    CheckHits(Comp, W, DeltaTime);
}

AVSOrbitProjectile* UVSOrbitBehavior::SpawnSingleBall(UVSWeaponComponent* Comp, FVSWeaponInstance& Weapon)
{
	if (!Comp) return nullptr;

    AActor* Owner = Comp->GetOwner();
    if (!Owner || !Weapon.Data || !Weapon.Data->OrbitConfig.BallClass) return nullptr;

    if (OrbitBalls.Num() >= Weapon.Data->OrbitConfig.MaxCount)
    {
        return nullptr;
    }

    AVSOrbitProjectile* Ball = Comp->GetWorld()->SpawnActor<AVSOrbitProjectile>(
        Weapon.Data->OrbitConfig.BallClass,
        Owner->GetActorLocation(), FRotator::ZeroRotator);
    if (Ball)
    {
        Ball->HitRadius = Weapon.Data->OrbitConfig.HitRadius;
        OrbitBalls.Add(Ball);
    }
    return Ball;
}

void UVSOrbitBehavior::PositionBalls(UVSWeaponComponent* Comp, FVSWeaponInstance& Weapon)
{
    if (!Comp) return;
    AActor* Owner = Comp->GetOwner();
    if (!Owner || !Weapon.Data) return;

    const FVector Center = Owner->GetActorLocation();
    const int32 Count = OrbitBalls.Num();
    if (Count <= 0) return;

    const float Radius = Weapon.Data->OrbitConfig.Radius;
    const float AngleStep = 360.f / Count;

    for (int32 i = 0; i < Count; ++i)
    {
        if (!OrbitBalls[i]) continue;

        // 구슬들을 원형으로 균등 분배 + 현재 회전각 적용
        const float AngleDeg = OrbitAngle + AngleStep * i;
        const float Rad = FMath::DegreesToRadians(AngleDeg);
        const FVector Offset(FMath::Cos(Rad) * Radius, FMath::Sin(Rad) * Radius, 0.f);

        OrbitBalls[i]->SetActorLocation(Center + Offset);
    }
}

void UVSOrbitBehavior::CheckHits(UVSWeaponComponent* Comp, FVSWeaponInstance& Weapon, float DeltaTime)
{
    AActor* Owner = Comp->GetOwner();
    if (!Owner) return;

    if (!Comp) return;

    const FVSPassiveStatModifiers& Mods = Comp->GetStatMods();

    for (AVSOrbitProjectile* Ball : OrbitBalls)
    {
        if (!Ball) continue;
        const FVector BallLoc = Ball->GetActorLocation();

        Comp->ApplyContinuousDamage(
            BallLoc,
            Ball->HitRadius,
            Weapon.GetDamage(Mods),
            DeltaTime);
    }
}