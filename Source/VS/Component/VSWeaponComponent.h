#pragma once
#include "CoreMinimal.h"
#include "Data/VSWeaponData.h"
#include "Components/ActorComponent.h"
#include "Weapon/VSOrbitProjectile.h"
#include "Manager/VSEnemyManager.h"
#include "Character/VSStatModifiers.h"
#include "VSWeaponComponent.generated.h"

class AVSDrone;
class AVSShieldArea;

USTRUCT()
struct FVSWeaponInstance
{
    GENERATED_BODY()

    UPROPERTY()
    UVSWeaponData* Data = nullptr;

    int32 Level = 1;
    float CooldownTimer = 0.f;

	// Orbit 무기용----------------------------------------------
    float OrbitAngle = 0.f;   // 현재 회전 각도

    UPROPERTY()
    TArray<TObjectPtr<AVSOrbitProjectile>> OrbitBalls;   // 이 무기의 구슬들
    // Orbit 무기용----------------------------------------------

    TObjectPtr<AVSShieldArea> ShieldActor;

    UPROPERTY()
    TObjectPtr<class AVSDrone> Drone;

    UPROPERTY()
    TObjectPtr<UVSWeaponBehavior> Behavior = nullptr;

    // 현재 레벨 기준 실제 스탯 계산
    float GetDamage(const FVSStatModifiers& Mods) const
    {
        const float Base = Data ? Data->BaseDamage + Data->DamagePerLevel * (Level - 1) : 0.f;
        return Base * (1.f + Mods.Get("GlobalDamage"));
    }
    float GetCooldown(const FVSStatModifiers& Mods) const
    {
        const float Base = Data ? FMath::Max(0.1f, Data->BaseCooldown - Data->CooldownReductionPerLevel * (Level - 1)) : 1.f;
        const float Reduced = Base * (1.f - Mods.Get("GlobalCooldown"));    // 감소니까 -
        return FMath::Max(0.1f, Reduced);   // ← 최소 0.1초 보장
    }

    int32 GetProjectileCount() const
    {
        if (!Data) return 1;
        
        return FMath::Min(Data->ProjectilesPerShot + (Level - 1), Data->DroneConfig.MaxProjCount); // 레벨당 발사 수
    }

    float GetShieldRadius() const
    {
        if (!Data) return 100.f;

        return FMath::Min(Data->ShieldConfig.Radius + (Level - 1) * 50.f, Data->ShieldConfig.MaxRadius);  // 레벨당 +50
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class UVSWeaponComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    UVSWeaponComponent();

    void AddWeapon(UVSWeaponData* WeaponData);     // 새 무기 획득
    void UpgradeWeapon(int32 WeaponIndex);         // 기존 무기 강화
    const TArray<FVSWeaponInstance>& GetWeapons() const { return Weapons; }
    bool UpgradeWeaponByData(UVSWeaponData* WeaponData);
    bool HasWeapon(UVSWeaponData* WeaponData) const;

    void SetEnemyManager(AVSEnemyManager* InEnemyManager) { EnemyManager = InEnemyManager; }

    void ApplyContinuousDamage(const FVector& Center, float Radius, float DamagePerSecond, float DeltaTime);

    AVSEnemyManager* GetEnemyManager() const { return EnemyManager.Get(); }

    FVector GetFloorLocation() const;

    const FVSStatModifiers& GetStatMods() const;

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	TWeakObjectPtr<AVSEnemyManager> EnemyManager;

    UPROPERTY()
    TArray<FVSWeaponInstance> Weapons;
};