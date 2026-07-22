#include "Weapon/Behavior/VSOrbitBehavior.h"

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

void UVSOrbitBehavior::Tick(UVSWeaponComponent* Comp, FVSWeaponInstance& W, float DeltaTime)
{
    if (!W.Data) return;

    // 회전 각도 증가
    W.OrbitAngle += W.Data->OrbitConfig.Speed * DeltaTime;
    if (W.OrbitAngle >= 360.f)
        W.OrbitAngle -= 360.f;

    PositionBalls(Comp, W);

    CheckHits(Comp, W, DeltaTime);
}

AVSOrbitProjectile* UVSOrbitBehavior::SpawnSingleBall(UVSWeaponComponent* Comp, FVSWeaponInstance& Weapon)
{
	if (!Comp) return nullptr;

    AActor* Owner = Comp->GetOwner();
    if (!Owner || !Weapon.Data || !Weapon.Data->OrbitConfig.BallClass) return nullptr;

    if (Weapon.OrbitBalls.Num() >= Weapon.Data->OrbitConfig.MaxCount)
    {
        return nullptr;
    }

    AVSOrbitProjectile* Ball = Comp->GetWorld()->SpawnActor<AVSOrbitProjectile>(
        Weapon.Data->OrbitConfig.BallClass,
        Owner->GetActorLocation(), FRotator::ZeroRotator);
    if (Ball)
    {
        Ball->HitRadius = Weapon.Data->OrbitConfig.HitRadius;
        Weapon.OrbitBalls.Add(Ball);
    }
    return Ball;
}

void UVSOrbitBehavior::PositionBalls(UVSWeaponComponent* Comp, FVSWeaponInstance& Weapon)
{
    if (!Comp) return;

    AActor* Owner = Comp->GetOwner();
    if (!Owner || !Weapon.Data) return;

    const FVector Center = Owner->GetActorLocation();
    const int32 Count = Weapon.OrbitBalls.Num();
    const float Radius = Weapon.Data->OrbitConfig.Radius;

    for (int32 i = 0; i < Count; ++i)
    {
        if (!Weapon.OrbitBalls[i]) continue;

        // 구슬들을 원형으로 균등 분배 + 현재 회전각 적용
        const float AngleDeg = Weapon.OrbitAngle + (360.f / Count) * i;
        const float Rad = FMath::DegreesToRadians(AngleDeg);
        const FVector Offset(FMath::Cos(Rad) * Radius, FMath::Sin(Rad) * Radius, 0.f);

        Weapon.OrbitBalls[i]->SetActorLocation(Center + Offset);
    }
}

void UVSOrbitBehavior::CheckHits(UVSWeaponComponent* Comp, FVSWeaponInstance& Weapon, float DeltaTime)
{
    AActor* Owner = Comp->GetOwner();
    if (!Owner) return;

    for (AVSOrbitProjectile* Ball : Weapon.OrbitBalls)
    {
        if (!Ball) continue;
        const FVector BallLoc = Ball->GetActorLocation();

        Comp->ApplyContinuousDamage(
            BallLoc,
            Ball->HitRadius,
            Weapon.GetDamage(),
            DeltaTime);
    }
}