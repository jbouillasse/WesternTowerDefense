#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PathActor.generated.h"

class USplineComponent;

UCLASS()
class WESTERNTOWERDEFENSE_API APathActor : public AActor
{
    GENERATED_BODY()

public:
    APathActor();
    bool IsAmbushPath() const { return bAmbushPath; }
    UFUNCTION(BlueprintCallable)
    int32 GetWaypointCount() const;

    UFUNCTION(BlueprintCallable)
    FVector GetWaypoint(int32 Index) const;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TObjectPtr<USplineComponent> Spline;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Path")
    bool bAmbushPath = false;
};