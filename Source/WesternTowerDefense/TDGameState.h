#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "TDGameState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMoneyChanged, int32, NewMoney);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWaveChanged, int32, CurrentWave, int32, TotalWaves);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCountdownChanged, int32, SecondsLeft);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUpgradeLevelChanged, int32, NewLevel);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGameOver, bool, bVictory);

UCLASS()
class WESTERNTOWERDEFENSE_API ATDGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	void InitState(int32 StartMoney, int32 InTotalWaves);

	UFUNCTION(BlueprintCallable)
	void AddMoney(int32 Amount);

	UFUNCTION(BlueprintCallable)
	bool TrySpend(int32 Amount);

	void SetWave(int32 InCurrentWave);
	void SetCountdown(int32 Seconds);
	void NotifyGameOver(bool bVictory);

	UFUNCTION(BlueprintCallable)
	void IncreaseUpgradeLevel();

	UFUNCTION(BlueprintPure) int32 GetMoney() const { return Money; }
	UFUNCTION(BlueprintPure) int32 GetCurrentWave() const { return CurrentWave; }
	UFUNCTION(BlueprintPure) int32 GetTotalWaves() const { return TotalWaves; }
	UFUNCTION(BlueprintPure) int32 GetUpgradeLevel() const { return UpgradeLevel; }

	UPROPERTY(BlueprintAssignable) FOnMoneyChanged OnMoneyChanged;
	UPROPERTY(BlueprintAssignable) FOnWaveChanged OnWaveChanged;
	UPROPERTY(BlueprintAssignable) FOnCountdownChanged OnCountdownChanged;
	UPROPERTY(BlueprintAssignable) FOnUpgradeLevelChanged OnUpgradeLevelChanged;
	UPROPERTY(BlueprintAssignable) FOnGameOver OnGameOver;

private:
	int32 Money = 0;
	int32 CurrentWave = 0;
	int32 TotalWaves = 0;
	int32 UpgradeLevel = 0;
};