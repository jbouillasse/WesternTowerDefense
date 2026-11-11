#include "TDGameMode.h"
#include "TDGameState.h"
#include "WaveTypes.h"
#include "EnemyDataAsset.h"
#include "TurretDataAsset.h"
#include "Enemy.h"
#include "Turret.h"
#include "PathActor.h"
#include "RailPath.h"
#include "Train.h"
#include "Saloon.h"
#include "Engine/DataTable.h"
#include "Engine/World.h"
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

    TArray<AActor*> Found;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APathActor::StaticClass(), Found);
    for (AActor* A : Found)
    {
        APathActor* P = Cast<APathActor>(A);
        if (!P) continue;

        if (P->IsAmbushPath()) AmbushPath = P;
        else Paths.Add(P);
    }
    if (Paths.IsEmpty())
    {
        UE_LOG(LogTemp, Error, TEXT("TDGameMode : aucun PathActor dans le niveau"));
        return;
    }

    RailPath = Cast<ARailPath>(UGameplayStatics::GetActorOfClass(GetWorld(), ARailPath::StaticClass()));
    if (!RailPath)
    {
        UE_LOG(LogTemp, Warning, TEXT("TDGameMode : aucun RailPath, l'evenement du train est desactive"));
    }

    if (ASaloon* Saloon = Cast<ASaloon>(UGameplayStatics::GetActorOfClass(GetWorld(), ASaloon::StaticClass())))
    {
        Saloon->OnSaloonDestroyed.AddDynamic(this, &ATDGameMode::HandleSaloonDestroyed);
    }

    TDState->InitState(StartingMoney, TotalWaves);
    TDState->SetTrainCalls(MaxTrainCalls);
    TDState->SetTrainState(ETrainState::Idle);
    StartPreparation();
}

void ATDGameMode::StartPreparation()
{
    Phase = ETDPhase::Preparation;
    TDState->SetPreparation(true);

    if (TDState->GetTrainState() == ETrainState::Requested)
    {
        LaunchTrain();
    }
    else
    {
        BeginCountdown();
    }
}

void ATDGameMode::BeginCountdown()
{
    if (Phase != ETDPhase::Preparation) return;

    SecondsLeft = FMath::CeilToInt(TimeBetweenWaves);
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

    for (int32 i = SpawnQueue.Num() - 1; i > 0; --i)
    {
        SpawnQueue.Swap(i, FMath::RandRange(0, i));
    }

    Phase = ETDPhase::WaveActive;
    GetWorldTimerManager().SetTimer(SpawnTimer, this, &ATDGameMode::SpawnNext,
        FMath::Max(Row->SpawnInterval, 0.1f), true, 0.f);
}

AEnemy* ATDGameMode::SpawnEnemy(UEnemyDataAsset* Data, APathActor* Path, bool bAmbush)
{
    if (!Data || !Path || !EnemyClass) return nullptr;

    const FTransform SpawnTransform(FRotator::ZeroRotator, Path->GetWaypoint(0) + FVector(0.f, 0.f, 90.f));

    AEnemy* Enemy = GetWorld()->SpawnActorDeferred<AEnemy>(
        EnemyClass, SpawnTransform, nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
    if (!Enemy) return nullptr;
    
    Enemy->SetPath(Path);
    Enemy->InitFromData(Data);

    if (bAmbush)
    {
        Enemy->OnEnemyDied.AddDynamic(this, &ATDGameMode::HandleAmbushEnemyDied);
        Enemy->OnEnemyReachedEnd.AddDynamic(this, &ATDGameMode::HandleAmbushEnemyReachedEnd);
    }
    else
    {
        Enemy->OnEnemyDied.AddDynamic(this, &ATDGameMode::HandleEnemyDied);
        Enemy->OnEnemyReachedEnd.AddDynamic(this, &ATDGameMode::HandleEnemyReachedEnd);
    }

    Enemy->FinishSpawning(SpawnTransform);
    return Enemy;
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

    if (SpawnEnemy(Data, Path, false))
    {
        ++EnemiesAlive;
    }
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
        TickSheriffLifetime();
        StartPreparation();
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
    GetWorldTimerManager().ClearTimer(AmbushTimer);

    UE_LOG(LogTemp, Warning, TEXT("%s"), bVictory ? TEXT("VICTOIRE !") : TEXT("DEFAITE..."));
    TDState->NotifyGameOver(bVictory);
}

void ATDGameMode::Notify(const FString& Message)
{
    if (TDState)
    {
        TDState->NotifyTrainResult(FText::FromString(Message));
    }
}

bool ATDGameMode::RequestTrain()
{
    if (!TDState) return false;

    if (Phase == ETDPhase::Victory || Phase == ETDPhase::Defeat) return false;

    if (!RailPath || !TrainClass)
    {
        Notify(TEXT("No railway is available."));
        return false;
    }

    if (TDState->GetTrainState() != ETrainState::Idle)
    {
        Notify(TEXT("A train is already on its way."));
        return false;
    }

    if (TDState->GetTrainCallsLeft() <= 0)
    {
        Notify(TEXT("You have already used your train call."));
        return false;
    }

    const int32 PausesLeft = (Phase == ETDPhase::WaveActive)
        ? TotalWaves - CurrentWave
        : TotalWaves - CurrentWave - 1;
    if (PausesLeft <= 0)
    {
        Notify(TEXT("Too late: there are no more breaks before the end of the game."));
        return false;
    }

    TDState->SetTrainCalls(TDState->GetTrainCallsLeft() - 1);
    TDState->SetTrainState(ETrainState::Requested);
    Notify(TEXT("The train has been called: it will arrive during the next break between waves."));
    return true;
}

void ATDGameMode::LaunchTrain()
{
    if (!RailPath || !TrainClass)
    {
        TDState->SetTrainState(ETrainState::Idle);
        BeginCountdown();
        return;
    }

    TDState->SetTrainState(ETrainState::Arriving);

    const FTransform T(RailPath->GetRotationAtDistance(0.f), RailPath->GetLocationAtDistance(0.f));
    ATrain* Train = GetWorld()->SpawnActorDeferred<ATrain>(
        TrainClass, T, nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
    if (!Train)
    {
        TDState->SetTrainState(ETrainState::Idle);
        BeginCountdown();
        return;
    }

    Train->Init(RailPath);
    Train->OnTrainStopped.AddDynamic(this, &ATDGameMode::HandleTrainStopped);
    Train->FinishSpawning(T);
    CurrentTrain = Train;
}

void ATDGameMode::HandleTrainStopped()
{
    if (TDState)
    {
        TDState->SetTrainState(ETrainState::WaitingChoice);
    }
}

void ATDGameMode::SendTrainAway()
{
    if (CurrentTrain)
    {
        CurrentTrain->Depart();
        CurrentTrain = nullptr;
    }
}

void ATDGameMode::FinishTrainEvent()
{
    SendTrainAway();
    TDState->SetTrainState(ETrainState::Idle);
    BeginCountdown();
}

void ATDGameMode::ResolveTrain(bool bAccept)
{
    if (!TDState || TDState->GetTrainState() != ETrainState::WaitingChoice) return;

    if (!bAccept)
    {
        Notify(TEXT("You declined the help. The train leaves without stopping."));
        FinishTrainEvent();
        return;
    }

    const int32 Total = FMath::Max(1, WeightSheriffs + WeightOutlaws + WeightMerchants);
    const int32 Roll = FMath::RandRange(0, Total - 1);

    if (Roll < WeightSheriffs) OutcomeSheriffs();
    else if (Roll < WeightSheriffs + WeightOutlaws) OutcomeOutlaws();
    else OutcomeMerchants();
}

void ATDGameMode::OutcomeSheriffs()
{
    SpawnSheriffs();
    Notify(FString::Printf(
        TEXT("Good news! Sheriffs are stepping off the train and will shoot at the bandits for %d waves."),
        SheriffWaves));
    FinishTrainEvent();
}

void ATDGameMode::OutcomeMerchants()
{
    TDState->AddMoney(MerchantMoney);
    const bool bUpgraded = TDState->GrantFreeUpgrade();
    Notify(FString::Printf(
        TEXT("Merchandise! +%d $%s"), MerchantMoney,
        bUpgraded ? TEXT(" and your shooters are upgraded for free.") : TEXT(".")));
    FinishTrainEvent();
}

void ATDGameMode::OutcomeOutlaws()
{
    Notify(TEXT("Bad surprise! Outlaws were on board and are attacking from the station!"));
    SendTrainAway();
    StartAmbush();
}

void ATDGameMode::SpawnSheriffs()
{
    if (!SheriffTurretClass || !SheriffData || !CurrentTrain)
    {
        UE_LOG(LogTemp, Warning, TEXT("Sherifs : SheriffTurretClass ou SheriffData non renseigne"));
        return;
    }

    DismissSheriffs();

    const FTransform TrainTransform = CurrentTrain->GetActorTransform();

    for (const FVector& Offset : SheriffOffsets)
    {
        FVector Location = TrainTransform.TransformPositionNoScale(Offset);
        
        FHitResult Hit;
        FCollisionQueryParams Params;
        Params.AddIgnoredActor(CurrentTrain);
        if (GetWorld()->LineTraceSingleByChannel(Hit, Location + FVector(0.f, 0.f, 800.f),
            Location - FVector(0.f, 0.f, 3000.f), ECC_Visibility, Params))
        {
            Location = Hit.ImpactPoint;
        }

        const FTransform T(FRotator::ZeroRotator, Location);
        ATurret* Sheriff = GetWorld()->SpawnActorDeferred<ATurret>(
            SheriffTurretClass, T, nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
        if (!Sheriff) continue;

        Sheriff->SetData(SheriffData);
        Sheriff->FinishSpawning(T);
        Sheriffs.Add(Sheriff);
    }

    SheriffWavesLeft = SheriffWaves;
}

void ATDGameMode::DismissSheriffs()
{
    for (const TWeakObjectPtr<ATurret>& S : Sheriffs)
    {
        if (S.IsValid()) S->Destroy();
    }
    Sheriffs.Reset();
}

void ATDGameMode::TickSheriffLifetime()
{
    if (SheriffWavesLeft > 0 && --SheriffWavesLeft == 0)
    {
        DismissSheriffs();
        Notify(TEXT("The sheriffs have left with the train."));
    }
}

void ATDGameMode::StartAmbush()
{
    if (!AmbushEnemyData)
    {
        UE_LOG(LogTemp, Warning, TEXT("Embuscade : AmbushEnemyData non renseigne"));
        TDState->SetTrainState(ETrainState::Idle);
        BeginCountdown();
        return;
    }

    TDState->SetTrainState(ETrainState::Ambush);
    AmbushRemaining = AmbushCount;
    AmbushAlive = 0;
    GetWorldTimerManager().SetTimer(AmbushTimer, this, &ATDGameMode::SpawnAmbushEnemy,
        FMath::Max(AmbushInterval, 0.1f), true, 1.f);
}

void ATDGameMode::SpawnAmbushEnemy()
{
    if (AmbushRemaining <= 0)
    {
        GetWorldTimerManager().ClearTimer(AmbushTimer);
        CheckAmbushEnd();
        return;
    }

    --AmbushRemaining;

    APathActor* Path = AmbushPath ? AmbushPath.Get() : Paths[FMath::RandRange(0, Paths.Num() - 1)].Get();
    if (SpawnEnemy(AmbushEnemyData, Path, true))
    {
        ++AmbushAlive;
    }
}

void ATDGameMode::HandleAmbushEnemyDied(AEnemy* Enemy)
{
    if (TDState && Enemy)
    {
        TDState->AddMoney(Enemy->GetReward());
    }
    --AmbushAlive;
    CheckAmbushEnd();
}

void ATDGameMode::HandleAmbushEnemyReachedEnd(AEnemy* Enemy)
{
    --AmbushAlive;
    CheckAmbushEnd();
}

void ATDGameMode::CheckAmbushEnd()
{
    if (Phase != ETDPhase::Preparation) return;
    if (TDState->GetTrainState() != ETrainState::Ambush) return;
    if (AmbushRemaining > 0 || AmbushAlive > 0) return;

    GetWorldTimerManager().ClearTimer(AmbushTimer);
    Notify(TEXT("Ambush repelled!"));
    TDState->SetTrainState(ETrainState::Idle);
    BeginCountdown();
}