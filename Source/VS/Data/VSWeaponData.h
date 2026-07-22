#pragma once
#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "VSWeaponData.generated.h"

class AVSOrbitProjectile;
class AVSShieldArea;
class AVSDrone;
class AVSProjectile;
class UVSWeaponBehavior;

UENUM(BlueprintType)
enum class EVSFirePattern : uint8
{
    NearestTarget UMETA(DisplayName = "라이플"),       // 가장 가까운 적에게
    MultiShot UMETA(DisplayName = "샷건"),             // 여러 방향 산탄
    Orbit UMETA(DisplayName = "파이어볼"),             // 주위를 도는 오브젝트
    AreaAroundPlayer UMETA(DisplayName = "실드"),      // 플레이어 주변 장판
	SummonDrone   UMETA(DisplayName = "발사 드론"),     // 드론 소환(드론이 발사체를 발사)
};

USTRUCT(BlueprintType)
struct FVSMultiShotConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere) float SpreadAngle = 30.f;
};

USTRUCT(BlueprintType)
struct FVSOrbitConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere) TSubclassOf<AVSOrbitProjectile> BallClass;
    UPROPERTY(EditAnywhere) int32 MaxCount = 6;
    UPROPERTY(EditAnywhere) float Radius = 200.f;
    UPROPERTY(EditAnywhere) float Speed = 180.f;
    UPROPERTY(EditAnywhere) float HitRadius = 150.f;
};

USTRUCT(BlueprintType)
struct FVSDroneConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere) TSubclassOf<AVSDrone> DroneClass;
    UPROPERTY(EditAnywhere) float Offset = 100.f;
    UPROPERTY(EditAnywhere) float Scale = 0.5f;
    UPROPERTY(EditAnywhere) int32 MaxProjCount = 5;
};

USTRUCT(BlueprintType)
struct FVSShieldConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere) TSubclassOf<AVSShieldArea> ShieldClass;
    UPROPERTY(EditAnywhere) float Radius = 300.f;
    UPROPERTY(EditAnywhere) float MaxRadius = 600.f;
};

UCLASS(BlueprintType)
class UVSWeaponData : public UPrimaryDataAsset
{
    GENERATED_BODY()
public:
    // 공통
    UPROPERTY(EditAnywhere, Category="Weapon") FString WeaponName;
    UPROPERTY(EditAnywhere, Category="Weapon",
        meta = (EditCondition = "FirePattern==EVSFirePattern::NearestTarget || FirePattern==EVSFirePattern::MultiShot || FirePattern==EVSFirePattern::SummonDrone",
        EditConditionHides))
    TSubclassOf<AVSProjectile> ProjectileClass;
    UPROPERTY(EditAnywhere, Category="Weapon") float BaseDamage = 15.f;
    UPROPERTY(EditAnywhere, Category="Weapon") float BaseCooldown = 0.5f;
    UPROPERTY(EditAnywhere, Category="Weapon") float BaseRange = 1500.f;
    UPROPERTY(EditAnywhere, Category="Weapon") int32 ProjectilesPerShot = 1;
    UPROPERTY(EditAnywhere, Category="Weapon") EVSFirePattern FirePattern = EVSFirePattern::NearestTarget;

    UPROPERTY(EditAnywhere, Category="Upgrade") float DamagePerLevel = 5.f;
    UPROPERTY(EditAnywhere, Category="Upgrade") float CooldownReductionPerLevel = 0.05f;

    // 무기별 옵션 구조체
    UPROPERTY(EditAnywhere, Category="MultiShot", meta=(EditCondition="FirePattern==EVSFirePattern::MultiShot", EditConditionHides))
	FVSMultiShotConfig MultiShotConfig;
    
    UPROPERTY(EditAnywhere, Category="Orbit", meta=(EditCondition="FirePattern==EVSFirePattern::Orbit", EditConditionHides))
    FVSOrbitConfig OrbitConfig;

    UPROPERTY(EditAnywhere, Category="Drone", meta=(EditCondition="FirePattern==EVSFirePattern::SummonDrone", EditConditionHides))
    FVSDroneConfig DroneConfig;

    UPROPERTY(EditAnywhere, Category="Area", meta=(EditCondition="FirePattern==EVSFirePattern::AreaAroundPlayer", EditConditionHides))
    FVSShieldConfig ShieldConfig;

    UPROPERTY(EditDefaultsOnly, Category="Behavior")
    TSubclassOf<UVSWeaponBehavior> BehaviorClass;
};