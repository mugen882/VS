#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "Weapon/VSProjectile.h"
#include "Character/VSStatModifiers.h"
#include "VSCharacter.generated.h"

class UInputAction;
class UInputMappingContext;
class AVSEnemyManager;
class UVSWeaponComponent;
class UVSUpgradeComponent;
class UVSWeaponData;
class UVSUpgradeSelectionWidget;
class UVSUpgradeData;
class AVSGemManager;

UCLASS(Blueprintable)
class VS_API AVSCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AVSCharacter();

	FORCEINLINE class UCameraComponent* GetTopDownCameraComponent() const { return TopDownCameraComponent; }
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

	void AddXP(int32 Amount);
	void TakeDamageFromEnemy(float Damage);
	void OnPlayerDeath();
	UVSWeaponComponent* GetWeaponComponent() { return WeaponComp; }

	bool IsDead() const { return bIsDead; }

	void AddPassive(FName StatName, float Value);
	const FVSStatModifiers& GetStatMods() const { return StatMods; }

public:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Progression")
    int32 CurrentXP = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Progression")
    int32 CurrentLevel = 1;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Progression")
    int32 XPToNextLevel = 5;   // 다음 레벨까지 필요 XP

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Stats")
    float CurrentHealth = 100.f;

    UPROPERTY(EditAnywhere, Category="Stats")
    float MaxHealth = 100.f;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	void LevelUp();

protected:
    UPROPERTY(EditAnywhere, Category="Combat")
    float FireInterval = 0.5f;      // 발사 주기(초)

    UPROPERTY(EditAnywhere, Category="Combat")
    float AttackRange = 1500.f;     // 사거리

    UPROPERTY(EditAnywhere, Category="Combat")
    TSubclassOf<class AVSProjectile> ProjectileClass;

	UPROPERTY(VisibleAnywhere)
	UVSWeaponComponent* WeaponComp;

	UPROPERTY(VisibleAnywhere)
	UVSUpgradeComponent* UpgradeComp;

	UPROPERTY(EditAnywhere, Category="Weapon")
    UVSWeaponData* StartingWeapon = nullptr;

	UPROPERTY(EditAnywhere, Category="UI")
    TSubclassOf<UVSUpgradeSelectionWidget> UpgradeSelectionWidgetClass;

private:
    void ShowUpgradeSelection();

    UFUNCTION()
    void OnUpgradeChosen(UVSUpgradeData* Chosen);

	void RecalculateStats();

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* TopDownCameraComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

    float FireTimer = 0.f;

	TObjectPtr<AVSEnemyManager> EnemyManager;

	UPROPERTY()
	TObjectPtr<AVSGemManager> GemManager;

	bool bIsDead = false;

	UPROPERTY()
    TObjectPtr<UVSUpgradeSelectionWidget> ActiveUpgradeWidget;

	UPROPERTY()
    FVSStatModifiers StatMods;

    // base 값(패시브 적용 전 원본) — 재계산의 기준
    UPROPERTY(EditAnywhere, Category="Stats")
    float BaseMoveSpeed = 600.f;
    UPROPERTY(EditAnywhere, Category="Stats")
    float BaseMaxHealth = 100.f;
};

