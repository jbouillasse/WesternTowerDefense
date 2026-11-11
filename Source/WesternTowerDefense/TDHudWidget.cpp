#include "TDHudWidget.h"
#include "TDGameState.h"
#include "TDPlayerController.h"
#include "TDGameMode.h"
#include "TurretDataAsset.h"
#include "Saloon.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

void UTDHudWidget::NativeConstruct()
{
    Super::NativeConstruct();

    Panel_End->SetVisibility(ESlateVisibility::Collapsed);
    if (Text_Message) Text_Message->SetText(FText::GetEmpty());
    if (Panel_Train) Panel_Train->SetVisibility(ESlateVisibility::Collapsed);
    if (Button_CallTrain) Button_CallTrain->OnClicked.AddDynamic(this, &UTDHudWidget::OnCallTrainClicked);
    if (Button_TrainAccept) Button_TrainAccept->OnClicked.AddDynamic(this, &UTDHudWidget::OnTrainAcceptClicked);
    if (Button_TrainRefuse) Button_TrainRefuse->OnClicked.AddDynamic(this, &UTDHudWidget::OnTrainRefuseClicked);
    if (Text_TrainTitle) Text_TrainTitle->SetText(FText::FromString(TEXT("A train is approaching...")));
    if (Text_TrainDesc) Text_TrainDesc->SetText(FText::FromString(TEXT("Impossible to identify its passengers.")));

    Button_Upgrade->OnClicked.AddDynamic(this, &UTDHudWidget::OnUpgradeClicked);
    Button_Restart->OnClicked.AddDynamic(this, &UTDHudWidget::OnRestartClicked);
    if (Button_Turret0) Button_Turret0->OnClicked.AddDynamic(this, &UTDHudWidget::OnTurret0Clicked);
    if (Button_Turret1) Button_Turret1->OnClicked.AddDynamic(this, &UTDHudWidget::OnTurret1Clicked);

    GameState = GetWorld()->GetGameState<ATDGameState>();
    if (ATDGameState* GS = GameState.Get())
    {
        GS->OnMoneyChanged.AddDynamic(this, &UTDHudWidget::UpdateMoney);
        GS->OnWaveChanged.AddDynamic(this, &UTDHudWidget::UpdateWave);
        GS->OnCountdownChanged.AddDynamic(this, &UTDHudWidget::UpdateCountdown);
        GS->OnPhaseChanged.AddDynamic(this, &UTDHudWidget::UpdatePhase);
        GS->OnUpgradeLevelChanged.AddDynamic(this, &UTDHudWidget::UpdateUpgrade);
        GS->OnGameOver.AddDynamic(this, &UTDHudWidget::ShowGameOver);
        GS->OnTrainStateChanged.AddDynamic(this, &UTDHudWidget::UpdateTrain);
        GS->OnTrainResult.AddDynamic(this, &UTDHudWidget::ShowTrainResult);

        UpdateMoney(GS->GetMoney());
        UpdateWave(GS->GetCurrentWave(), GS->GetTotalWaves());
        UpdatePhase(GS->IsPreparation());
        UpdateCountdown(GS->GetSecondsLeft());
        UpdateUpgrade(GS->GetUpgradeLevel());
        UpdateTrain(GS->GetTrainState(), GS->GetTrainCallsLeft());
    }

    Saloon = Cast<ASaloon>(UGameplayStatics::GetActorOfClass(GetWorld(), ASaloon::StaticClass()));
    if (ASaloon* S = Saloon.Get())
    {
        S->OnHealthChanged.AddDynamic(this, &UTDHudWidget::UpdateSaloon);
        UpdateSaloon(S->GetCurrentHealth(), S->GetMaxHealth());
    }

    PC = Cast<ATDPlayerController>(GetOwningPlayer());
    if (ATDPlayerController* Controller = PC.Get())
    {
        Controller->OnHudMessage.AddDynamic(this, &UTDHudWidget::HandleMessage);
        Controller->OnTurretSelected.AddDynamic(this, &UTDHudWidget::HandleSelected);

        const auto& Buildable = Controller->GetBuildable();
        auto Fill = [&](int32 I, UTextBlock* T)
        {
            if (T && Buildable.IsValidIndex(I) && Buildable[I])
            {
                T->SetText(FText::FromString(FString::Printf(TEXT("%s (%d $)"),
                    *Buildable[I]->DisplayName.ToString(), Buildable[I]->Cost)));
            }
        };
        Fill(0, Text_Turret0);
        Fill(1, Text_Turret1);
        HandleSelected(Controller->GetSelectedIndex());
    }
}

void UTDHudWidget::NativeDestruct()
{
    if (ATDGameState* GS = GameState.Get())
    {
        GS->OnMoneyChanged.RemoveAll(this);
        GS->OnWaveChanged.RemoveAll(this);
        GS->OnCountdownChanged.RemoveAll(this);
        GS->OnPhaseChanged.RemoveAll(this);
        GS->OnUpgradeLevelChanged.RemoveAll(this);
        GS->OnGameOver.RemoveAll(this);
        GS->OnTrainStateChanged.RemoveAll(this);
        GS->OnTrainResult.RemoveAll(this);
    }
    if (ASaloon* S = Saloon.Get()) S->OnHealthChanged.RemoveAll(this);
    if (ATDPlayerController* C = PC.Get())
    {
        C->OnHudMessage.RemoveAll(this);
        C->OnTurretSelected.RemoveAll(this);
    }
    Super::NativeDestruct();
}

void UTDHudWidget::UpdateMoney(int32 NewMoney)
{
    Text_Money->SetText(FText::FromString(FString::Printf(TEXT("$ %d"), NewMoney)));
}

void UTDHudWidget::UpdateWave(int32 CurrentWave, int32 TotalWaves)
{
    Text_Wave->SetText(FText::FromString(FString::Printf(TEXT("WAVE %d / %d"), CurrentWave, TotalWaves)));
}

void UTDHudWidget::UpdateCountdown(int32 SecondsLeft)
{
    if (bTrainBusy) return;
    Text_Countdown->SetText(FText::FromString(FString::Printf(TEXT("Next wave in %d s"), SecondsLeft)));
}

void UTDHudWidget::UpdatePhase(bool bInPreparation)
{
    bPreparation = bInPreparation;
    Text_Countdown->SetVisibility(bPreparation ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
    RefreshUpgradeButton();
}

void UTDHudWidget::UpdateUpgrade(int32 NewLevel)
{
    RefreshUpgradeButton();
}

void UTDHudWidget::RefreshUpgradeButton()
{
    ATDGameState* GS = GameState.Get();
    if (!GS) return;

    const bool bMax = GS->GetUpgradeLevel() >= GS->GetMaxUpgradeLevel();
    if (bMax)
    {
        Text_Upgrade->SetText(FText::FromString(TEXT("Shooters at max level")));
    }
    else
    {
        Text_Upgrade->SetText(FText::FromString(FString::Printf(
            TEXT("Upgrade all shooters (lvl %d) - %d $"),
            GS->GetUpgradeLevel() + 1, GS->GetUpgradeCost())));
    }
    Button_Upgrade->SetIsEnabled(bPreparation && !bMax);
}

void UTDHudWidget::UpdateSaloon(float CurrentHealth, float MaxHealth)
{
    ProgressBar_Saloon->SetPercent(MaxHealth > 0.f ? CurrentHealth / MaxHealth : 0.f);
    if (Text_SaloonHP)
    {
        Text_SaloonHP->SetText(FText::FromString(FString::Printf(TEXT("Saloon: %.0f HP"), CurrentHealth)));
    }
}

void UTDHudWidget::ShowGameOver(bool bVictory)
{
    Text_EndTitle->SetText(FText::FromString(bVictory ? TEXT("VICTORY!") : TEXT("DEFEAT...")));
    Panel_End->SetVisibility(ESlateVisibility::Visible);
    UGameplayStatics::SetGamePaused(this, true);
}

void UTDHudWidget::HandleMessage(FText Message)
{
    if (!Text_Message) return;
    Text_Message->SetText(Message);
    GetWorld()->GetTimerManager().SetTimer(MessageTimer, this, &UTDHudWidget::ClearMessage, 2.f, false);
}

void UTDHudWidget::ClearMessage()
{
    if (Text_Message) Text_Message->SetText(FText::GetEmpty());
}

void UTDHudWidget::HandleSelected(int32 Index)
{
    ATDPlayerController* Controller = PC.Get();
    if (!Text_Selected || !Controller) return;

    const auto& Buildable = Controller->GetBuildable();
    if (Buildable.IsValidIndex(Index) && Buildable[Index])
    {
        Text_Selected->SetText(FText::FromString(FString::Printf(TEXT("Selected: %s"),
            *Buildable[Index]->DisplayName.ToString())));
    }
}

void UTDHudWidget::OnUpgradeClicked()
{
    ATDGameState* GS = GameState.Get();
    if (GS && !GS->TryUpgrade())
    {
        HandleMessage(FText::FromString(TEXT("Upgrade impossible (not enough money)")));
    }
}

void UTDHudWidget::OnRestartClicked()
{
    UGameplayStatics::SetGamePaused(this, false);
    UGameplayStatics::OpenLevel(this, FName(*UGameplayStatics::GetCurrentLevelName(this, true)));
}

void UTDHudWidget::OnTurret0Clicked() { if (PC.IsValid()) PC->SelectTurret(0); }
void UTDHudWidget::OnTurret1Clicked() { if (PC.IsValid()) PC->SelectTurret(1); }

void UTDHudWidget::UpdateTrain(ETrainState NewState, int32 CallsLeft)
{
    bTrainBusy = (NewState == ETrainState::Arriving
        || NewState == ETrainState::WaitingChoice
        || NewState == ETrainState::Ambush);

    if (Panel_Train)
    {
        Panel_Train->SetVisibility(NewState == ETrainState::WaitingChoice
            ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
    }

    if (Button_CallTrain)
    {
        Button_CallTrain->SetIsEnabled(NewState == ETrainState::Idle && CallsLeft > 0);
    }

    if (Text_CallTrain)
    {
        FString Label;
        switch (NewState)
        {
        case ETrainState::Idle:
            Label = CallsLeft > 0 ? FString::Printf(TEXT("Call the train (%d)"), CallsLeft)
                                  : TEXT("Train unavailable");
            break;
        case ETrainState::Requested: Label = TEXT("Train requested..."); break;
        default: Label = TEXT("Train in progress"); break;
        }
        Text_CallTrain->SetText(FText::FromString(Label));
    }

    if (bTrainBusy && Text_Countdown)
    {
        FString Info;
        switch (NewState)
        {
        case ETrainState::Arriving: Info = TEXT("A train is approaching..."); break;
        case ETrainState::WaitingChoice: Info = TEXT("The train is at the station: your call"); break;
        default: Info = TEXT("Ambush! Fight off the outlaws"); break;
        }
        Text_Countdown->SetText(FText::FromString(Info));
    }
    else if (ATDGameState* GS = GameState.Get())
    {
        UpdateCountdown(GS->GetSecondsLeft());
    }
}

void UTDHudWidget::ShowTrainResult(FText Message)
{
    if (!Text_Message) return;
    Text_Message->SetText(Message);
    GetWorld()->GetTimerManager().SetTimer(MessageTimer, this, &UTDHudWidget::ClearMessage, 5.f, false);
}

void UTDHudWidget::OnCallTrainClicked()
{
    if (ATDGameMode* GM = GetWorld()->GetAuthGameMode<ATDGameMode>())
    {
        GM->RequestTrain();
    }
}

void UTDHudWidget::OnTrainAcceptClicked()
{
    if (ATDGameMode* GM = GetWorld()->GetAuthGameMode<ATDGameMode>())
    {
        GM->ResolveTrain(true);
    }
}

void UTDHudWidget::OnTrainRefuseClicked()
{
    if (ATDGameMode* GM = GetWorld()->GetAuthGameMode<ATDGameMode>())
    {
        GM->ResolveTrain(false);
    }
}