#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TurretSlot.generated.h"

class UBoxComponent;
class UStaticMeshComponent;
class ATurret;
class UTurretDataAsset;

UCLASS()
class WESTERNTOWERDEFENSE_API ATurretSlot : public AActor
{
	GENERATED_BODY()

public:
	ATurretSlot();

	bool IsOccupied() const { return bOccupied; }
	bool Place(TSubclassOf<ATurret> TurretClass, UTurretDataAsset* Data);

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void HandleBoxClicked(UPrimitiveComponent* TouchedComponent, FKey ButtonPressed);

	UFUNCTION()
	void HandleCursorOver(UPrimitiveComponent* TouchedComponent);

	UFUNCTION()
	void HandleCursorEnd(UPrimitiveComponent* TouchedComponent);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UStaticMeshComponent> Marker;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UBoxComponent> ClickBox;

	bool bOccupied = false;
};