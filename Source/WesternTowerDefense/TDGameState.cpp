#include "TDGameState.h"

void ATDGameState::InitState(int32 StartMoney, int32 InTotalWaves)
{
    Money = StartMoney;
    TotalWaves = InTotalWaves;
    CurrentWave = 0;
    UpgradeLevel = 0;
    OnMoneyChanged.Broadcast(Money);
    OnWaveChanged.Broadcast(CurrentWave, TotalWaves);
    OnUpgradeLevelChanged.Broadcast(UpgradeLevel);
}
//test
void ATDGameState::AddMoney(int32 Amount)
{
    Money += Amount;
    OnMoneyChanged.Broadcast(Money);
}

bool ATDGameState::TrySpend(int32 Amount)
{
    if (Money < Amount) return false;
    Money -= Amount;
    OnMoneyChanged.Broadcast(Money);
    return true;
}

bool ATDGameState::TryUpgrade()
{
    if (!bPreparation || UpgradeLevel >= MaxUpgradeLevel) return false;
    if (!TrySpend(GetUpgradeCost())) return false;

    ++UpgradeLevel;
    OnUpgradeLevelChanged.Broadcast(UpgradeLevel);
    return true;
}

bool ATDGameState::GrantFreeUpgrade()
{
    if (UpgradeLevel >= MaxUpgradeLevel) return false;
    ++UpgradeLevel;
    OnUpgradeLevelChanged.Broadcast(UpgradeLevel);
    return true;
}

void ATDGameState::SetWave(int32 InCurrentWave)
{
    CurrentWave = InCurrentWave;
    OnWaveChanged.Broadcast(CurrentWave, TotalWaves);
}

void ATDGameState::SetCountdown(int32 Seconds)
{
    SecondsLeft = Seconds;
    OnCountdownChanged.Broadcast(Seconds);
}

void ATDGameState::SetPreparation(bool bInPreparation)
{
    bPreparation = bInPreparation;
    OnPhaseChanged.Broadcast(bPreparation);
}

void ATDGameState::NotifyGameOver(bool bVictory)
{
    OnGameOver.Broadcast(bVictory);
}

void ATDGameState::SetTrainCalls(int32 Calls)
{
    TrainCallsLeft = Calls;
    OnTrainStateChanged.Broadcast(TrainState, TrainCallsLeft);
}

void ATDGameState::SetTrainState(ETrainState NewState)
{
    TrainState = NewState;
    OnTrainStateChanged.Broadcast(TrainState, TrainCallsLeft);
}

void ATDGameState::NotifyTrainResult(const FText& Message)
{
    OnTrainResult.Broadcast(Message);
}