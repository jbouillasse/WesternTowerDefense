#include "TDHudWidget.h"
#include "TDGameState.h"
#include "TDPlayerController.h"
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

    Button_Upgrade->OnClicked.AddDynamic(this, &UTDHudWidget::OnUpgradeClicked);
    Button_Restart->OnClicked.AddDynamic(this, &UTDHudWidget::OnRestartClicked);
    if (Button_Turret0) Button_Turret0->OnClicked.AddDynamic(this, &UTDHudWidget::OnTurret0Clicked);
    if (Button_Turret1) Button_Turret1->OnClicked.AddDynamic(this, &UTDHudWidget::OnTurret1Clicked);

    // Delegates du GameState
    GameState = GetWorld()->GetGameState<ATDGameState>();
    if (ATDGameState* GS = GameState.Get())
    {
        GS->OnMoneyChanged.AddDynamic(this, &UTDHudWidget::UpdateMoney);
        GS->OnWaveChanged.AddDynamic(this, &UTDHudWidget::UpdateWave);
        GS->OnCountdownChanged.AddDynamic(this, &UTDHudWidget::UpdateCountdown);
        GS->OnPhaseChanged.AddDynamic(this, &UTDHudWidget::UpdatePhase);
        GS->OnUpgradeLevelChanged.AddDynamic(this, &UTDHudWidget::UpdateUpgrade);
        GS->OnGameOver.AddDynamic(this, &UTDHudWidget::ShowGameOver);

        // Valeurs initiales (le GameMode a pu diffuser avant la création du widget)
        UpdateMoney(GS->GetMoney());
        UpdateWave(GS->GetCurrentWave(), GS->GetTotalWaves());
        UpdatePhase(GS->IsPreparation());
        UpdateCountdown(GS->GetSecondsLeft());
        UpdateUpgrade(GS->GetUpgradeLevel());
    }

    // Delegate du saloon
    Saloon = Cast<ASaloon>(UGameplayStatics::GetActorOfClass(GetWorld(), ASaloon::StaticClass()));
    if (ASaloon* S = Saloon.Get())
    {
        S->OnHealthChanged.AddDynamic(this, &UTDHudWidget::UpdateSaloon);
        UpdateSaloon(S->GetCurrentHealth(), S->GetMaxHealth());
    }

    // Controleur : sélection et messages
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
    Text_Wave->SetText(FText::FromString(FString::Printf(TEXT("VAGUE %d / %d"), CurrentWave, TotalWaves)));
}

void UTDHudWidget::UpdateCountdown(int32 SecondsLeft)
{
    Text_Countdown->SetText(FText::FromString(FString::Printf(TEXT("Prochaine vague dans %d s"), SecondsLeft)));
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
        Text_Upgrade->SetText(FText::FromString(TEXT("Tireurs au niveau maximum")));
    }
    else
    {
        Text_Upgrade->SetText(FText::FromString(FString::Printf(
            TEXT("Améliorer tous les tireurs (niv. %d) - %d $"),
            GS->GetUpgradeLevel() + 1, GS->GetUpgradeCost())));
    }
    Button_Upgrade->SetIsEnabled(bPreparation && !bMax);
}

void UTDHudWidget::UpdateSaloon(float CurrentHealth, float MaxHealth)
{
    ProgressBar_Saloon->SetPercent(MaxHealth > 0.f ? CurrentHealth / MaxHealth : 0.f);
    if (Text_SaloonHP)
    {
        Text_SaloonHP->SetText(FText::FromString(FString::Printf(TEXT("Saloon : %.0f PV"), CurrentHealth)));
    }
}

void UTDHudWidget::ShowGameOver(bool bVictory)
{
    Text_EndTitle->SetText(FText::FromString(bVictory ? TEXT("VICTOIRE !") : TEXT("DÉFAITE...")));
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
        Text_Selected->SetText(FText::FromString(FString::Printf(TEXT("Sélection : %s"),
            *Buildable[Index]->DisplayName.ToString())));
    }
}

void UTDHudWidget::OnUpgradeClicked()
{
    ATDGameState* GS = GameState.Get();
    if (GS && !GS->TryUpgrade())
    {
        HandleMessage(FText::FromString(TEXT("Amélioration impossible (argent insuffisant)")));
    }
}
//test
void UTDHudWidget::OnRestartClicked()
{
    UGameplayStatics::SetGamePaused(this, false);
    UGameplayStatics::OpenLevel(this, FName(*UGameplayStatics::GetCurrentLevelName(this, true)));
}

void UTDHudWidget::OnTurret0Clicked() { if (PC.IsValid()) PC->SelectTurret(0); }
void UTDHudWidget::OnTurret1Clicked() { if (PC.IsValid()) PC->SelectTurret(1); }

//test