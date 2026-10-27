#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "TDGameMode.generated.h"

class UDataTable;
class AEnemy;
class APathActor;
class ATDGameState;
class UEnemyDataAsset;

UENUM(BlueprintType)
enum class ETDPhase : uint8
{
	Preparation,
	WaveActive,
	Victory,
	Defeat
};

UCLASS()
class WESTERNTOWERDEFENSE_API ATDGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ATDGameMode();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waves")
	TObjectPtr<UDataTable> WaveTable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waves")
	TSubclassOf<AEnemy> EnemyClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waves")
	float TimeBetweenWaves = 10.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Economy")
	int32 StartingMoney = 150;

private:
	void StartCountdown();
	void TickCountdown();
	void StartWave();
	void SpawnNext();
	void CheckWaveEnd();
	void EndGame(bool bVictory);

	UFUNCTION()
	void HandleEnemyDied(AEnemy* Enemy);

	UFUNCTION()
	void HandleEnemyReachedEnd(AEnemy* Enemy);

	UFUNCTION()
	void HandleSaloonDestroyed();

	UPROPERTY()
	TArray<TObjectPtr<APathActor>> Paths;

	UPROPERTY()
	TArray<TObjectPtr<UEnemyDataAsset>> SpawnQueue;

	UPROPERTY()
	TObjectPtr<ATDGameState> TDState;

	ETDPhase Phase = ETDPhase::Preparation;
	int32 CurrentWave = 0;
	int32 TotalWaves = 0;
	int32 SecondsLeft = 0;
	int32 EnemiesAlive = 0;

	FTimerHandle CountdownTimer;
	FTimerHandle SpawnTimer;
};