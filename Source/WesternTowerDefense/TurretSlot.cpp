#include "TurretSlot.h"
#include "Turret.h"
#include "TurretDataAsset.h"
#include "TDPlayerController.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "InputCoreTypes.h"
#include "Engine/World.h"

ATurretSlot::ATurretSlot()
{
    PrimaryActorTick.bCanEverTick = false;

    Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    SetRootComponent(Root);

    Marker = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Marker"));
    Marker->SetupAttachment(Root);
    Marker->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Marker->SetCanEverAffectNavigation(false);

    ClickBox = CreateDefaultSubobject<UBoxComponent>(TEXT("ClickBox"));
    ClickBox->SetupAttachment(Root);
    ClickBox->SetBoxExtent(FVector(200.f, 200.f, 100.f));
    ClickBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    ClickBox->SetCollisionResponseToAllChannels(ECR_Ignore);
    ClickBox->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
    ClickBox->SetCanEverAffectNavigation(false);
}

void ATurretSlot::BeginPlay()
{
    Super::BeginPlay();

    ClickBox->OnClicked.AddDynamic(this, &ATurretSlot::HandleBoxClicked);
    ClickBox->OnBeginCursorOver.AddDynamic(this, &ATurretSlot::HandleCursorOver);
    ClickBox->OnEndCursorOver.AddDynamic(this, &ATurretSlot::HandleCursorEnd);
}

void ATurretSlot::HandleBoxClicked(UPrimitiveComponent* TouchedComponent, FKey ButtonPressed)
{
    if (ButtonPressed != EKeys::LeftMouseButton) return;

    if (ATDPlayerController* PC = Cast<ATDPlayerController>(GetWorld()->GetFirstPlayerController()))
    {
        PC->TryPlaceOnSlot(this);
    }
}

void ATurretSlot::HandleCursorOver(UPrimitiveComponent* TouchedComponent)
{
    if (!bOccupied) Marker->SetRelativeScale3D(FVector(1.2f));
}

void ATurretSlot::HandleCursorEnd(UPrimitiveComponent* TouchedComponent)
{
    Marker->SetRelativeScale3D(FVector::OneVector);
}

bool ATurretSlot::Place(TSubclassOf<ATurret> TurretClass, UTurretDataAsset* Data)
{
    if (bOccupied || !TurretClass) return false;

    FTransform T = GetActorTransform();
    T.SetScale3D(FVector::OneVector);

    ATurret* Turret = GetWorld()->SpawnActorDeferred<ATurret>(
        TurretClass, T, nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
    if (!Turret) return false;

    Turret->SetData(Data);
    Turret->FinishSpawning(T);

    bOccupied = true;
    Marker->SetVisibility(false);
    ClickBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    return true;
}