#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "TDGameMode.generated.h"

class UDataTable;
class AEnemy;
class APathActor;
class ARailPath;
class ATrain;
class ATurret;
class ATDGameState;
class UEnemyDataAsset;
class UTurretDataAsset;

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
    
    UFUNCTION(BlueprintCallable)
    bool RequestTrain();
    
    UFUNCTION(BlueprintCallable)
    void ResolveTrain(bool bAccept);

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
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Train")
    TSubclassOf<ATrain> TrainClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Train")
    int32 MaxTrainCalls = 1;

    // Probabilités des 3 résultats
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Train|Probabilites")
    int32 WeightSheriffs = 40;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Train|Probabilites")
    int32 WeightOutlaws = 30;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Train|Probabilites")
    int32 WeightMerchants = 30;

    // Shérifs
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Train|Sheriffs")
    TSubclassOf<ATurret> SheriffTurretClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Train|Sheriffs")
    TObjectPtr<UTurretDataAsset> SheriffData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Train|Sheriffs")
    TArray<FVector> SheriffOffsets = { FVector(0.f, -400.f, 0.f), FVector(0.f, 400.f, 0.f) };

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Train|Sheriffs")
    int32 SheriffWaves = 2;

    // Embuscade
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Train|Ambush")
    TObjectPtr<UEnemyDataAsset> AmbushEnemyData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Train|Ambush")
    int32 AmbushCount = 6;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Train|Ambush")
    float AmbushInterval = 1.2f;

    // Marchandises
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Train|Merchants")
    int32 MerchantMoney = 200;

private:
    // Déroulement de la partie
    void StartPreparation();
    void BeginCountdown();
    void TickCountdown();
    void StartWave();
    void SpawnNext();
    void CheckWaveEnd();
    void EndGame(bool bVictory);
    AEnemy* SpawnEnemy(UEnemyDataAsset* Data, APathActor* Path, bool bAmbush);

    UFUNCTION() void HandleEnemyDied(AEnemy* Enemy);
    UFUNCTION() void HandleEnemyReachedEnd(AEnemy* Enemy);
    UFUNCTION() void HandleSaloonDestroyed();

    // Train
    void LaunchTrain();
    void SendTrainAway();
    void FinishTrainEvent();
    void OutcomeSheriffs();
    void OutcomeOutlaws();
    void OutcomeMerchants();
    void SpawnSheriffs();
    void DismissSheriffs();
    void TickSheriffLifetime();
    void StartAmbush();
    void SpawnAmbushEnemy();
    void CheckAmbushEnd();
    void Notify(const FString& Message);

    UFUNCTION() void HandleTrainStopped();
    UFUNCTION() void HandleAmbushEnemyDied(AEnemy* Enemy);
    UFUNCTION() void HandleAmbushEnemyReachedEnd(AEnemy* Enemy);

    UPROPERTY() TArray<TObjectPtr<APathActor>> Paths;
    UPROPERTY() TArray<TObjectPtr<UEnemyDataAsset>> SpawnQueue;
    UPROPERTY() TObjectPtr<ATDGameState> TDState;
    UPROPERTY() TObjectPtr<ARailPath> RailPath;
    UPROPERTY() TObjectPtr<APathActor> AmbushPath;
    UPROPERTY() TObjectPtr<ATrain> CurrentTrain;

    TArray<TWeakObjectPtr<ATurret>> Sheriffs;

    ETDPhase Phase = ETDPhase::Preparation;
    int32 CurrentWave = 0;
    int32 TotalWaves = 0;
    int32 SecondsLeft = 0;
    int32 EnemiesAlive = 0;
    int32 SheriffWavesLeft = 0;
    int32 AmbushRemaining = 0;
    int32 AmbushAlive = 0;

    FTimerHandle CountdownTimer;
    FTimerHandle SpawnTimer;
    FTimerHandle AmbushTimer;
};