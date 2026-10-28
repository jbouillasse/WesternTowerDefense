#include "TDPlayerController.h"
#include "TDHudWidget.h"
#include "TDGameState.h"
#include "TurretSlot.h"
#include "Turret.h"
#include "TurretDataAsset.h"
#include "Camera/CameraActor.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"

void ATDPlayerController::BeginPlay()
{
    Super::BeginPlay();

    bShowMouseCursor = true;
    bEnableClickEvents = true;
    bEnableMouseOverEvents = true;

    FInputModeGameAndUI Mode;
    Mode.SetHideCursorDuringCapture(false);
    Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
    SetInputMode(Mode);

    // Caméra fixe du niveau
    if (AActor* Cam = UGameplayStatics::GetActorOfClass(GetWorld(), ACameraActor::StaticClass()))
    {
        SetViewTargetWithBlend(Cam, 0.f);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Aucune CameraActor dans le niveau"));
    }

    if (HudWidgetClass)
    {
        Hud = CreateWidget<UTDHudWidget>(this, HudWidgetClass);
        if (Hud) Hud->AddToViewport();
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("HudWidgetClass non renseigne dans BP_TDPlayerController"));
    }
}

void ATDPlayerController::SelectTurret(int32 Index)
{
    if (!Buildable.IsValidIndex(Index)) return;
    SelectedIndex = Index;
    OnTurretSelected.Broadcast(SelectedIndex);
}

void ATDPlayerController::TryPlaceOnSlot(ATurretSlot* Slot)
{
    if (!Slot) return;

    if (Slot->IsOccupied())
    {
        OnHudMessage.Broadcast(FText::FromString(TEXT("Emplacement déjà occupé")));
        return;
    }

    if (!Buildable.IsValidIndex(SelectedIndex) || !Buildable[SelectedIndex] || !TurretClass)
    {
        OnHudMessage.Broadcast(FText::FromString(TEXT("Aucun tireur sélectionné")));
        return;
    }

    UTurretDataAsset* Data = Buildable[SelectedIndex];
    ATDGameState* GS = GetWorld()->GetGameState<ATDGameState>();
    if (!GS) return;

    if (!GS->TrySpend(Data->Cost))
    {
        OnHudMessage.Broadcast(FText::FromString(TEXT("Pas assez d'argent")));
        return;
    }

    if (!Slot->Place(TurretClass, Data))
    {
        GS->AddMoney(Data->Cost); // remboursement si le placement échoue
    }
}