#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VSDrone.generated.h"

class USkeletalMeshComponent;

UCLASS()
class VS_API AVSDrone : public AActor
{
    GENERATED_BODY()
public:
    AVSDrone();

protected:
    virtual void BeginPlay() override;

protected:
    UPROPERTY(VisibleAnywhere, Category = "Drone")
    TObjectPtr<USkeletalMeshComponent> SkeletalMesh;
};