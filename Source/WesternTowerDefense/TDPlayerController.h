#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "TDPlayerController.generated.h"

class ATurret;
class ATurretSlot;
class UTurretDataAsset;
class UTDHudWidget;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTurretSelected, int32, Index);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHudMessage, FText, Message);

UCLASS()
class WESTERNTOWERDEFENSE_API ATDPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void SelectTurret(int32 Index);

	void TryPlaceOnSlot(ATurretSlot* Slot);

	const TArray<TObjectPtr<UTurretDataAsset>>& GetBuildable() const { return Buildable; }
	int32 GetSelectedIndex() const { return SelectedIndex; }

	UPROPERTY(BlueprintAssignable) FOnTurretSelected OnTurretSelected;
	UPROPERTY(BlueprintAssignable) FOnHudMessage OnHudMessage;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, Category = "TD")
	TSubclassOf<UTDHudWidget> HudWidgetClass;

	UPROPERTY(EditAnywhere, Category = "TD")
	TSubclassOf<ATurret> TurretClass;

	// Tireurs constructibles (index 0 = premier bouton, 1 = deuxième)
	UPROPERTY(EditAnywhere, Category = "TD")
	TArray<TObjectPtr<UTurretDataAsset>> Buildable;

	UPROPERTY()
	TObjectPtr<UTDHudWidget> Hud;

	int32 SelectedIndex = 0;
};