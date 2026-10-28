#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TDHudWidget.generated.h"

class UTextBlock;
class UProgressBar;
class UButton;
class UWidget;
class ATDGameState;
class ATDPlayerController;
class ASaloon;

UCLASS()
class WESTERNTOWERDEFENSE_API UTDHudWidget : public UUserWidget
{
    GENERATED_BODY()

protected:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

    // --- Obligatoires ---
    UPROPERTY(meta = (BindWidget)) TObjectPtr<UTextBlock> Text_Money;
    UPROPERTY(meta = (BindWidget)) TObjectPtr<UTextBlock> Text_Wave;
    UPROPERTY(meta = (BindWidget)) TObjectPtr<UTextBlock> Text_Countdown;
    UPROPERTY(meta = (BindWidget)) TObjectPtr<UProgressBar> ProgressBar_Saloon;
    UPROPERTY(meta = (BindWidget)) TObjectPtr<UButton> Button_Upgrade;
    UPROPERTY(meta = (BindWidget)) TObjectPtr<UTextBlock> Text_Upgrade;
    UPROPERTY(meta = (BindWidget)) TObjectPtr<UWidget> Panel_End;
    UPROPERTY(meta = (BindWidget)) TObjectPtr<UTextBlock> Text_EndTitle;
    UPROPERTY(meta = (BindWidget)) TObjectPtr<UButton> Button_Restart;

    // --- Optionnels ---
    UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UTextBlock> Text_SaloonHP;
    UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UTextBlock> Text_Message;
    UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UTextBlock> Text_Selected;
    UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UButton> Button_Turret0;
    UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UButton> Button_Turret1;
    UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UTextBlock> Text_Turret0;
    UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UTextBlock> Text_Turret1;

private:
    UFUNCTION() void UpdateMoney(int32 NewMoney);
    UFUNCTION() void UpdateWave(int32 CurrentWave, int32 TotalWaves);
    UFUNCTION() void UpdateCountdown(int32 SecondsLeft);
    UFUNCTION() void UpdatePhase(bool bPreparation);
    UFUNCTION() void UpdateUpgrade(int32 NewLevel);
    UFUNCTION() void UpdateSaloon(float CurrentHealth, float MaxHealth);
    UFUNCTION() void ShowGameOver(bool bVictory);
    UFUNCTION() void HandleMessage(FText Message);
    UFUNCTION() void HandleSelected(int32 Index);

    UFUNCTION() void OnUpgradeClicked();
    UFUNCTION() void OnRestartClicked();
    UFUNCTION() void OnTurret0Clicked();
    UFUNCTION() void OnTurret1Clicked();

    void RefreshUpgradeButton();
    void ClearMessage();

    TWeakObjectPtr<ATDGameState> GameState;
    TWeakObjectPtr<ATDPlayerController> PC;
    TWeakObjectPtr<ASaloon> Saloon;

    bool bPreparation = false;
    FTimerHandle MessageTimer;
};