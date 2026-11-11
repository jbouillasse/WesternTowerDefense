#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Train.generated.h"

class ARailPath;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTrainStopped);

UCLASS()
class WESTERNTOWERDEFENSE_API ATrain : public AActor
{
	GENERATED_BODY()

public:
	ATrain();
	virtual void Tick(float DeltaTime) override;

	void Init(ARailPath* InRail);
	void Depart();

	UPROPERTY(BlueprintAssignable)
	FOnTrainStopped OnTrainStopped;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USceneComponent> Root;

	// Vitesse de croisière (unités par seconde)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Train")
	float MaxSpeed = 1500.f;

	// Distance avant la gare à partir de laquelle le train freine
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Train")
	float BrakingDistance = 2500.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Train")
	float Acceleration = 600.f;

private:
	enum class EState : uint8 { Arriving, Stopped, Departing };

	void UpdateTransform();

	UPROPERTY()
	TObjectPtr<ARailPath> Rail;

	EState State = EState::Arriving;
	float Distance = 0.f;
	float Speed = 0.f;
};