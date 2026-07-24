#include "Character/VSPlayerCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "Camera/CameraComponent.h"
#include "Components/DecalComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "Materials/Material.h"
#include "Engine/World.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Manager/VSEnemyManager.h"
#include "Kismet/GameplayStatics.h"
#include "Component/VSWeaponComponent.h"
#include "Component/VSUpgradeComponent.h"
#include "Blueprint/UserWidget.h"
#include "UI/VSUpgradeSelectionWidget.h" 
#include "Manager/VSGemManager.h"
#include "Subsystem/VSDifficultySubsystem.h"

AVSPlayerCharacter::AVSPlayerCharacter()
{
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 640.f, 0.f);
	GetCharacterMovement()->bConstrainToPlane = true;
	GetCharacterMovement()->bSnapToPlaneAtStart = true;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->SetUsingAbsoluteRotation(true);
	CameraBoom->TargetArmLength = 1500.f;
	CameraBoom->SetRelativeRotation(FRotator(-60.f, 0.f, 0.f));
	CameraBoom->bDoCollisionTest = false;

	TopDownCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("TopDownCamera"));
	TopDownCameraComponent->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	TopDownCameraComponent->bUsePawnControlRotation = false;

	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = true;

	WeaponComp = CreateDefaultSubobject<UVSWeaponComponent>(TEXT("WeaponComp"));
	UpgradeComp = CreateDefaultSubobject<UVSUpgradeComponent>(TEXT("UpgradeComp"));
}

void AVSPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	EnemyManager = Cast<AVSEnemyManager>(UGameplayStatics::GetActorOfClass(this, AVSEnemyManager::StaticClass()));
	GemManager = Cast<AVSGemManager>(UGameplayStatics::GetActorOfClass(this, AVSGemManager::StaticClass()));

	if (WeaponComp)
	{
		WeaponComp->SetEnemyManager(EnemyManager);
	}

	if (WeaponComp && StartingWeapon)
	{
		WeaponComp->AddWeapon(StartingWeapon);
	}

	if (UWorld* World = GetWorld())
	{
		if (UVSDifficultySubsystem* Diff = World->GetSubsystem<UVSDifficultySubsystem>())
		{
			Diff->RegisterPlayerCharacter(this);
		}
	}
}

void AVSPlayerCharacter::LevelUp()
{
	CurrentLevel++;
	XPToNextLevel = FMath::RoundToInt(XPToNextLevel * 1.2f);
	UE_LOG(LogTemp, Warning, TEXT("LEVEL UP! Level=%d"), CurrentLevel);

	OnLevelChanged.Broadcast(CurrentLevel);

	ShowUpgradeSelection();
}

void AVSPlayerCharacter::AddXP(int32 Amount)
{
	CurrentXP += Amount;
	
	// 레벨업 체크 (여러 레벨 동시에 오를 수도 있으니 while)
	while (CurrentXP >= XPToNextLevel)
	{
		CurrentXP -= XPToNextLevel;
		LevelUp();
	} 

	// XP 비율 갱신 방송
	const int32 Need = XPToNextLevel > 0 ? XPToNextLevel : 1;
	OnXPChanged.Broadcast((float)CurrentXP / (float)Need);
}

void AVSPlayerCharacter::TakeDamageFromEnemy(float Damage)
{
	if (bIsDead) return;

	CurrentHealth -= Damage;

	const float MaxHP = MaxHealth > 0.f ? MaxHealth : 1.f;
	OnHealthChanged.Broadcast(FMath::Clamp(CurrentHealth / MaxHP, 0.f, 1.f));

	if (CurrentHealth <= 0.f)
	{
		CurrentHealth = 0.f;
		bIsDead = true;
		OnPlayerDeath();
	}
}

void AVSPlayerCharacter::OnPlayerDeath()
{
	OnPlayerDied.Broadcast();
}

void AVSPlayerCharacter::AddPassive(EVSStatType StatType, float Value)
{
	StatMods.Add(StatType, Value);
	RecalculateStats();   // 누적 후 재계산
}

void AVSPlayerCharacter::ShowUpgradeSelection()
{
	if (!UpgradeSelectionWidgetClass || !UpgradeComp) return;

	// 후보 3개 뽑기
	TArray<UVSUpgradeData*> Choices = UpgradeComp->RollUpgrades();
	if (Choices.Num() == 0) return;

	// 위젯 생성
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC) return;

	ActiveUpgradeWidget = CreateWidget<UVSUpgradeSelectionWidget>(PC, UpgradeSelectionWidgetClass);
	if (!ActiveUpgradeWidget) return;

	ActiveUpgradeWidget->SetupChoices(Choices);
	ActiveUpgradeWidget->OnUpgradeChosen.AddDynamic(this, &AVSPlayerCharacter::OnUpgradeChosen);
	ActiveUpgradeWidget->AddToViewport();

	UGameplayStatics::SetGamePaused(this, true); // 게임 정지
	if (UVSDifficultySubsystem* Diff = GetWorld()->GetSubsystem<UVSDifficultySubsystem>())
		Diff->SetUpgradeSelecting(true);
	// 마우스 입력 활성화
	PC->SetShowMouseCursor(true);
	PC->SetInputMode(FInputModeUIOnly());   // UI만 입력받기
}

void AVSPlayerCharacter::OnUpgradeChosen(UVSUpgradeData* Chosen)
{
	// 선택한 업그레이드 적용
	if (UpgradeComp)
		UpgradeComp->ApplyUpgrade(Chosen);

	// 위젯 제거 + 게임 재개
	if (ActiveUpgradeWidget)
	{
		ActiveUpgradeWidget->RemoveFromParent();
		ActiveUpgradeWidget = nullptr;
	}

	APlayerController* PC = Cast<APlayerController>(GetController());
	if (PC)
	{
		PC->SetShowMouseCursor(false);
		PC->SetInputMode(FInputModeGameOnly());   // 다시 게임 입력
	}
	UGameplayStatics::SetGamePaused(this, false);
	if (UVSDifficultySubsystem* Diff = GetWorld()->GetSubsystem<UVSDifficultySubsystem>())
		Diff->SetUpgradeSelecting(false);
}

void AVSPlayerCharacter::RecalculateStats()
{
	// 플레이어 스탯: base × (1 + 누적배율)
	if (UCharacterMovementComponent* Move = GetCharacterMovement())
		Move->MaxWalkSpeed = BaseMoveSpeed * (1.f + StatMods.Get(EVSStatType::MoveSpeed));

	// 늘어난 최대치만큼 현재 체력도 더해줌(증가분 회복)
	const float OldMax = MaxHealth;
	MaxHealth = BaseMaxHealth * (1.f + StatMods.Get(EVSStatType::MaxHealth));
	CurrentHealth += (MaxHealth - OldMax);
	CurrentHealth = FMath::Min(CurrentHealth, MaxHealth);

	// 픽업범위
	if (GemManager)
		GemManager->SetMagnetRangeMult(1.f + StatMods.Get(EVSStatType::PickupRange));
}
