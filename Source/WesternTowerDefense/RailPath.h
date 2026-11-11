#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RailPath.generated.h"

class USplineComponent;

UCLASS()
class WESTERNTOWERDEFENSE_API ARailPath : public AActor
{
	GENERATED_BODY()

public:
	ARailPath();

	float GetLength() const;
	float GetStationDistance() const;
	FVector GetLocationAtDistance(float Distance) const;
	FRotator GetRotationAtDistance(float Distance) const;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USplineComponent> Spline;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rail")
	int32 StationPointIndex = 1;
};