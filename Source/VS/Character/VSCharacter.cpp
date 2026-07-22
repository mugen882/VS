#include "VSCharacter.h"
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

AVSCharacter::AVSCharacter()
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

	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	WeaponComp = CreateDefaultSubobject<UVSWeaponComponent>(TEXT("WeaponComp"));
	UpgradeComp = CreateDefaultSubobject<UVSUpgradeComponent>(TEXT("UpgradeComp"));
}

void AVSCharacter::BeginPlay()
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
}

void AVSCharacter::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
}

void AVSCharacter::LevelUp()
{
	CurrentLevel++;
	XPToNextLevel = FMath::RoundToInt(XPToNextLevel * 1.2f);
	UE_LOG(LogTemp, Warning, TEXT("LEVEL UP! Level=%d"), CurrentLevel);

	ShowUpgradeSelection();
}

void AVSCharacter::AddXP(int32 Amount)
{
	CurrentXP += Amount;
	
	// 레벨업 체크 (여러 레벨 동시에 오를 수도 있으니 while)
	while (CurrentXP >= XPToNextLevel)
	{
		CurrentXP -= XPToNextLevel;
		LevelUp();
	} 
}

void AVSCharacter::TakeDamageFromEnemy(float Damage)
{
	if (bIsDead) return;

	CurrentHealth -= Damage;
	if (CurrentHealth <= 0.f)
	{
		CurrentHealth = 0.f;
		bIsDead = true;
		OnPlayerDeath();
	}
}

void AVSCharacter::OnPlayerDeath()
{
	UE_LOG(LogTemp, Warning, TEXT("PLAYER DIED - GAME OVER"));
	// 게임오버 처리 (다음)
}

void AVSCharacter::AddPassive(FName StatName, float Value)
{
	StatMods.Add(StatName, Value);
	RecalculateStats();   // 누적 후 재계산
}

void AVSCharacter::ShowUpgradeSelection()
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
	ActiveUpgradeWidget->OnUpgradeChosen.AddDynamic(this, &AVSCharacter::OnUpgradeChosen);
	ActiveUpgradeWidget->AddToViewport();

	UGameplayStatics::SetGamePaused(this, true); // 게임 정지
	// 마우스 입력 활성화
	PC->SetShowMouseCursor(true);
	PC->SetInputMode(FInputModeUIOnly());   // UI만 입력받기
}

void AVSCharacter::OnUpgradeChosen(UVSUpgradeData* Chosen)
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
}

void AVSCharacter::RecalculateStats()
{
	// 플레이어 스탯: base × (1 + 누적배율)
	if (UCharacterMovementComponent* Move = GetCharacterMovement())
		Move->MaxWalkSpeed = BaseMoveSpeed * (1.f + StatMods.Get("MoveSpeed"));

	// 늘어난 최대치만큼 현재 체력도 더해줌(증가분 회복)
	const float OldMax = MaxHealth;
	MaxHealth = BaseMaxHealth * (1.f + StatMods.Get("MaxHealth"));
	CurrentHealth += (MaxHealth - OldMax);
	CurrentHealth = FMath::Min(CurrentHealth, MaxHealth);

	// 픽업범위
	if (GemManager)
		GemManager->SetMagnetRangeMult(1.f + StatMods.Get("PickupRange"));
}
