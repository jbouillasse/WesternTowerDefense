#include "TDGameMode.h"
#include "TDGameState.h"
#include "WaveTypes.h"
#include "EnemyDataAsset.h"
#include "Enemy.h"
#include "PathActor.h"
#include "Saloon.h"
#include "Engine/DataTable.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

ATDGameMode::ATDGameMode()
{
    GameStateClass = ATDGameState::StaticClass();
}

void ATDGameMode::BeginPlay()
{
    Super::BeginPlay();

    TDState = GetGameState<ATDGameState>();
    TotalWaves = WaveTable ? WaveTable->GetRowNames().Num() : 0;

    if (!TDState || !WaveTable || !EnemyClass)
    {
        UE_LOG(LogTemp, Error, TEXT("TDGameMode : WaveTable, EnemyClass ou GameState manquant"));
        return;
    }

    // Recherche unique des chemins (3 entrees) au demarrage
    TArray<AActor*> Found;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APathActor::StaticClass(), Found);
    for (AActor* A : Found)
    {
        Paths.Add(Cast<APathActor>(A));
    }
    if (Paths.IsEmpty())
    {
        UE_LOG(LogTemp, Error, TEXT("TDGameMode : aucun PathActor dans le niveau"));
        return;
    }

    if (ASaloon* Saloon = Cast<ASaloon>(UGameplayStatics::GetActorOfClass(GetWorld(), ASaloon::StaticClass())))
    {
        Saloon->OnSaloonDestroyed.AddDynamic(this, &ATDGameMode::HandleSaloonDestroyed);
    }

    TDState->InitState(StartingMoney, TotalWaves);
    StartCountdown();
}

void ATDGameMode::StartCountdown()
{
    Phase = ETDPhase::Preparation;
    SecondsLeft = FMath::CeilToInt(TimeBetweenWaves);
    TDState->SetPreparation(true);
    TDState->SetCountdown(SecondsLeft);
    GetWorldTimerManager().SetTimer(CountdownTimer, this, &ATDGameMode::TickCountdown, 1.f, true);
}

void ATDGameMode::TickCountdown()
{
    --SecondsLeft;
    TDState->SetCountdown(FMath::Max(SecondsLeft, 0));

    if (SecondsLeft <= 0)
    {
        GetWorldTimerManager().ClearTimer(CountdownTimer);
        StartWave();
    }
}

void ATDGameMode::StartWave()
{
    ++CurrentWave;
    TDState->SetPreparation(false);
    TDState->SetWave(CurrentWave);

    const FWaveRow* Row = WaveTable->FindRow<FWaveRow>(FName(*FString::FromInt(CurrentWave)), TEXT("StartWave"));
    if (!Row)
    {
        UE_LOG(LogTemp, Error, TEXT("Ligne %d introuvable dans la Data Table (les noms de lignes doivent etre 1, 2, 3...)"), CurrentWave);
        return;
    }

    SpawnQueue.Reset();
    for (const FWaveEntry& Entry : Row->Entries)
    {
        for (int32 i = 0; i < Entry.Count; ++i)
        {
            SpawnQueue.Add(Entry.Enemy);
        }
    }

    // Melange pour alterner les types d'ennemis
    for (int32 i = SpawnQueue.Num() - 1; i > 0; --i)
    {
        SpawnQueue.Swap(i, FMath::RandRange(0, i));
    }

    Phase = ETDPhase::WaveActive;
    GetWorldTimerManager().SetTimer(SpawnTimer, this, &ATDGameMode::SpawnNext,
        FMath::Max(Row->SpawnInterval, 0.1f), true, 0.f);
}

void ATDGameMode::SpawnNext()
{
    if (SpawnQueue.IsEmpty())
    {
        GetWorldTimerManager().ClearTimer(SpawnTimer);
        CheckWaveEnd();
        return;
    }

    UEnemyDataAsset* Data = SpawnQueue.Pop();
    APathActor* Path = Paths[FMath::RandRange(0, Paths.Num() - 1)];
    if (!Data || !Path) return;

    const FTransform SpawnTransform(FRotator::ZeroRotator, Path->GetWaypoint(0) + FVector(0.f, 0.f, 90.f));

    AEnemy* Enemy = GetWorld()->SpawnActorDeferred<AEnemy>(
        EnemyClass, SpawnTransform, nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
    if (!Enemy) return;

    // Tout est regle AVANT FinishSpawning, donc avant la possession par l'IA
    Enemy->SetPath(Path);
    Enemy->InitFromData(Data);
    Enemy->OnEnemyDied.AddDynamic(this, &ATDGameMode::HandleEnemyDied);
    Enemy->OnEnemyReachedEnd.AddDynamic(this, &ATDGameMode::HandleEnemyReachedEnd);
    ++EnemiesAlive;

    Enemy->FinishSpawning(SpawnTransform);
}

void ATDGameMode::HandleEnemyDied(AEnemy* Enemy)
{
    if (TDState && Enemy)
    {
        TDState->AddMoney(Enemy->GetReward());
    }
    --EnemiesAlive;
    CheckWaveEnd();
}

void ATDGameMode::HandleEnemyReachedEnd(AEnemy* Enemy)
{
    --EnemiesAlive;
    CheckWaveEnd();
}

void ATDGameMode::CheckWaveEnd()
{
    if (Phase != ETDPhase::WaveActive) return;
    if (!SpawnQueue.IsEmpty() || EnemiesAlive > 0) return;

    GetWorldTimerManager().ClearTimer(SpawnTimer);

    if (CurrentWave >= TotalWaves)
    {
        EndGame(true);
    }
    else
    {
        StartCountdown();
    }
}

void ATDGameMode::HandleSaloonDestroyed()
{
    EndGame(false);
}

void ATDGameMode::EndGame(bool bVictory)
{
    if (Phase == ETDPhase::Victory || Phase == ETDPhase::Defeat) return;

    Phase = bVictory ? ETDPhase::Victory : ETDPhase::Defeat;
    GetWorldTimerManager().ClearTimer(SpawnTimer);
    GetWorldTimerManager().ClearTimer(CountdownTimer);

    UE_LOG(LogTemp, Warning, TEXT("%s"), bVictory ? TEXT("VICTOIRE !") : TEXT("DEFAITE..."));
    TDState->NotifyGameOver(bVictory);
}