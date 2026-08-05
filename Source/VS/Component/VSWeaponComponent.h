#pragma once
#include "CoreMinimal.h"
#include "Data/VSWeaponData.h"
#include "Components/ActorComponent.h"
#include "Weapon/VSOrbitProjectile.h"
#include "Manager/VSEnemyManager.h"
#include "Character/VSPassiveStatModifiers.h"
#include "Common/VSDefine.h"
#include "VSWeaponComponent.generated.h"

class AVSDrone;
class AVSShieldAura;

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
    TArray<TObjectPtr<AVSOrbitProjectile>> OrbitBalls;
    // Orbit 무기용----------------------------------------------

    UPROPERTY()
    TObjectPtr<AVSShieldAura> ShieldActor;

    UPROPERTY()
    TObjectPtr<AVSDrone> Drone;

    UPROPERTY()
    TObjectPtr<UVSWeaponBehavior> Behavior = nullptr;

    float GetDamage(const FVSPassiveStatModifiers& Mods) const
    {
        const float Base = Data ? Data->BaseDamage + Data->DamagePerLevel * (Level - 1) : 0.f;
        return Base * (1.f + Mods.Get(EVSPassiveStatType::GlobalDamage));
    }

    float GetCooldown(const FVSPassiveStatModifiers& Mods) const
    {
        const float Base = Data ? FMath::Max(MIN_COOLDOWN_TIME, Data->BaseCooldown - Data->CooldownReductionPerLevel * (Level - 1)) : 1.f;
        const float Reduced = Base * (1.f - Mods.Get(EVSPassiveStatType::GlobalCooldown));
        return FMath::Max(MIN_COOLDOWN_TIME, Reduced);   // 최소 시간 보장
    }

    int32 GetProjectileCount() const
    {
        if (!Data) return 1;
        
        return FMath::Min(Data->ProjectilesPerShot + (Level - 1), Data->DroneConfig.MaxProjCount);
    }

    float GetShieldRadius() const
    {
        if (!Data) return 100.f;

        return FMath::Min(Data->ShieldConfig.Radius + (Level - 1) * ADD_SHIELD_RADIUS, Data->ShieldConfig.MaxRadius);
    }
};

UCLASS(ClassGroup=(VS), meta=(BlueprintSpawnableComponent))
class UVSWeaponComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    UVSWeaponComponent();

    void AddWeapon(UVSWeaponData* WeaponData);
    const TArray<FVSWeaponInstance>& GetWeapons() const { return Weapons; }
    // 1레벨 강화. 미보유거나 MAX_WEAPON_LEVEL에 도달했으면 false.
    bool UpgradeWeaponByData(UVSWeaponData* WeaponData);
    bool HasWeapon(UVSWeaponData* WeaponData) const;

    // 미보유면 0
    int32 GetWeaponLevel(UVSWeaponData* WeaponData) const;
    // 보유 중이고 상한에 도달했으면 true (미보유면 false)
    bool IsWeaponMaxLevel(UVSWeaponData* WeaponData) const;

    // 보유한 모든 무기가 MIN_COOLDOWN_TIME에 걸려 있으면 true.
    // 쿨다운 감소 패시브를 더 찍어도 실제 발사 간격이 변하지 않는 상태를 뜻한다.
    bool IsCooldownSaturated() const;

    void SetEnemyManager(AVSEnemyManager* InEnemyManager) { EnemyManager = InEnemyManager; }

    void ApplyContinuousDamage(const FVector& Center, float Radius, float DamagePerSecond, float DeltaTime);

    AVSEnemyManager* GetEnemyManager() const { return EnemyManager.Get(); }

    FVector GetFloorLocation() const;

    const FVSPassiveStatModifiers& GetStatMods() const;

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	TWeakObjectPtr<AVSEnemyManager> EnemyManager;

    UPROPERTY()
    TArray<FVSWeaponInstance> Weapons;
};