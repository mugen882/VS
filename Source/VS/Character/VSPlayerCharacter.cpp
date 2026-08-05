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
#include "Common/VSLog.h"

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

	// 보스 텔레그래프 같은 바닥 데칼이 캐릭터 위로 투영되지 않게 한다
	GetMesh()->SetReceivesDecals(false);

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

	RecalculateStats();
	CurrentHealth = MaxHealth;
}

void AVSPlayerCharacter::LevelUp()
{
	CurrentLevel++;
	XPToNextLevel = FMath::RoundToInt(XPToNextLevel * XP_TO_NEXTLV_MULIPLY);
	UE_LOG(LogTemp, Warning, TEXT("LEVEL UP! Level=%d"), CurrentLevel);

	OnLevelChanged.Broadcast(CurrentLevel);
}

void AVSPlayerCharacter::AddXP(int32 Amount)
{
	if (bSkipLevelUp) return;

	CurrentXP += Amount;
	
	// 레벨업 체크 (여러 레벨 동시 상승 가능)
	while (CurrentXP >= XPToNextLevel)
	{
		CurrentXP -= XPToNextLevel;
		LevelUp();
		++PendingLevelUps;   // 처리 대기 큐에 누적
	} 

	// XP 비율 갱신
	const int32 Need = XPToNextLevel > 0 ? XPToNextLevel : 1;
	OnXPChanged.Broadcast((float)CurrentXP / (float)Need);

	// 지금 업그레이드 선택 중이 아니면 하나 시작.
	if (PendingLevelUps > 0 && !ActiveUpgradeWidget)
		ShowUpgradeSelection();
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

bool AVSPlayerCharacter::AddPassive(EVSPassiveStatType StatType, float Value)
{
	// FVSPassiveStatModifiers::Add가 상한을 판정한다.
	if (!StatMods.Add(StatType, Value))
		return false;

	RecalculateStats();
	return true;
}

bool AVSPlayerCharacter::ShowUpgradeSelection()
{
	// 카드를 띄울 수 없는 상황이면 큐를 비우고 실패를 알린다.
	// 안 그러면 PendingLevelUps가 남아 일시정지가 풀리지 않는다.
	if (!UpgradeSelectionWidgetClass || !UpgradeComp)
	{
		UE_LOG(VSLog, Error, TEXT("ShowUpgradeSelection: UpgradeSelectionWidgetClass 또는 UpgradeComp가 없습니다."));
		PendingLevelUps = 0;
		return false;
	}

	// 후보 n개 뽑기
	TArray<UVSUpgradeData*> Choices = UpgradeComp->RollUpgrades();
	if (Choices.Num() == 0)
	{
		UE_LOG(VSLog, Warning, TEXT("ShowUpgradeSelection: 남은 업그레이드 후보가 없습니다."));
		PendingLevelUps = 0;
		return false;
	}

	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC)
	{
		PendingLevelUps = 0;
		return false;
	}

	ActiveUpgradeWidget = CreateWidget<UVSUpgradeSelectionWidget>(PC, UpgradeSelectionWidgetClass);
	if (!ActiveUpgradeWidget)
	{
		PendingLevelUps = 0;
		return false;
	}

	ActiveUpgradeWidget->SetupChoices(Choices);
	ActiveUpgradeWidget->OnUpgradeChosen.AddDynamic(this, &AVSPlayerCharacter::OnUpgradeChosen);
	ActiveUpgradeWidget->AddToViewport();

	UGameplayStatics::SetGamePaused(this, true); // 일시정지
	if (UVSDifficultySubsystem* Diff = GetWorld()->GetSubsystem<UVSDifficultySubsystem>())
		Diff->SetUpgradeSelecting(true);

	PC->SetShowMouseCursor(true);
	PC->SetInputMode(FInputModeUIOnly());   // UI만 입력받기

	return true;
}

void AVSPlayerCharacter::OnUpgradeChosen(UVSUpgradeData* Chosen)
{
	// 선택한 업그레이드 적용
	if (UpgradeComp)
		UpgradeComp->ApplyUpgrade(Chosen);

	// 위젯 제거
	if (ActiveUpgradeWidget)
	{
		ActiveUpgradeWidget->RemoveFromParent();
		ActiveUpgradeWidget = nullptr;
	}

	--PendingLevelUps;
	// 대기 중인 레벨업 창 띄움. 띄우지 못하면 아래로 내려가 게임을 재개한다.
	if (PendingLevelUps > 0 && ShowUpgradeSelection())
	{
		return;
	}
	PendingLevelUps = 0;

	// 게임 재개
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (PC)
	{
		PC->SetShowMouseCursor(false);
		PC->SetInputMode(FInputModeGameOnly());   // 다시 게임 입력
	}
	UGameplayStatics::SetGamePaused(this, false);	// 일시정지 해제
	if (UVSDifficultySubsystem* Diff = GetWorld()->GetSubsystem<UVSDifficultySubsystem>())
		Diff->SetUpgradeSelecting(false);
}

void AVSPlayerCharacter::RecalculateStats()
{
	// 플레이어 스탯: base × (1 + 누적배율)
	if (UCharacterMovementComponent* Move = GetCharacterMovement())
		Move->MaxWalkSpeed = BaseMoveSpeed * (1.f + StatMods.Get(EVSPassiveStatType::MoveSpeed));
	
	const float OldMax = MaxHealth;
	MaxHealth = BaseMaxHealth * (1.f + StatMods.Get(EVSPassiveStatType::MaxHealth));	// 최대체력 계산
	CurrentHealth += (MaxHealth - OldMax);	// 늘어난 최대치만큼 현재 체력도 더해줌
	CurrentHealth = FMath::Min(CurrentHealth, MaxHealth);

	// 픽업범위
	if (GemManager)
		GemManager->SetMagnetRangeMult(1.f + StatMods.Get(EVSPassiveStatType::PickupRange));
}
