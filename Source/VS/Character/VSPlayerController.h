#pragma once

#include "CoreMinimal.h"
#include "Templates/SubclassOf.h"
#include "GameFramework/PlayerController.h"
#include "InputActionValue.h"
#include "VSPlayerController.generated.h"

class UNiagaraSystem;
class UInputMappingContext;
class UInputAction;

UCLASS()
class AVSPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AVSPlayerController();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* MoveAction;

	// 게임오버 시 띄울 위젯 클래스 (BP에서 지정)
	UPROPERTY(EditAnywhere, Category="UI")
	TSubclassOf<class UVSGameOverWidget> GameOverWidgetClass;
	
protected:
	virtual void SetupInputComponent() override;
	virtual void OnPossess(APawn* InPawn) override;
	
	virtual void BeginPlay();

	void Move(const FInputActionValue& Value);

private:
	void HandlePlayerDied();

private:
	FVector CachedDestination;
};


