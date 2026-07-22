#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VSDrone.generated.h"

class UStaticMeshComponent;

UCLASS()
class VS_API AVSDrone : public AActor
{
    GENERATED_BODY()
public:
    AVSDrone();

protected:
    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UStaticMeshComponent> Mesh;
};