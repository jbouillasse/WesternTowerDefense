#include "Turret.h"
#include "Enemy.h"
#include "TurretDataAsset.h"
#include "TDGameState.h"
#include "Components/SphereComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "Animation/AnimInstance.h"
#include "Kismet/KismetMathLibrary.h"
#include "TimerManager.h"
#include "Engine/World.h"

ATurret::ATurret()
{
    PrimaryActorTick.bCanEverTick = false;

    Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    SetRootComponent(Root);

    Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));
    Mesh->SetupAttachment(Root);
    Mesh->SetCollisionProfileName(TEXT("NoCollision"));
    Mesh->SetCanEverAffectNavigation(false);

    RangeSphere = CreateDefaultSubobject<USphereComponent>(TEXT("RangeSphere"));
    RangeSphere->SetupAttachment(Root);
    RangeSphere->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
    RangeSphere->SetGenerateOverlapEvents(true);
    RangeSphere->SetHiddenInGame(true);
    RangeSphere->SetCanEverAffectNavigation(false);

    TracerBeam = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TracerBeam"));
    TracerBeam->SetupAttachment(Root);
    TracerBeam->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    TracerBeam->SetCanEverAffectNavigation(false);
    TracerBeam->SetCastShadow(false);
    TracerBeam->SetVisibility(false);
}

void ATurret::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);
    ApplyData();
    RecalcStats();
}

void ATurret::SetData(UTurretDataAsset* NewData)
{
    Data = NewData;
    ApplyData();
    RecalcStats();
}

void ATurret::ApplyData()
{
    if (!Data) return;

    Range = Data->Range;
    Damage = Data->Damage;
    FireRate = Data->FireRate;
    Cost = Data->Cost;

    if (Data->Mesh && Mesh)
    {
        Mesh->SetSkeletalMesh(Data->Mesh);
    }
}

void ATurret::RecalcStats()
{
    CurrentRange = Range * (1.f + 0.5f * UpgradeLevel);
    CurrentDamage = Damage * (1.f + 0.25f * UpgradeLevel);
    CurrentFireRate = FireRate * (1.f + 0.15f * UpgradeLevel);

    if (RangeSphere)
    {
        RangeSphere->SetSphereRadius(CurrentRange);
    }

    if (HasActorBegunPlay())
    {
        GetWorldTimerManager().SetTimer(FireTimer, this, &ATurret::Fire,
            1.f / FMath::Max(CurrentFireRate, 0.01f), true);
    }
}

void ATurret::BeginPlay()
{
    Super::BeginPlay();

    ApplyData();

    if (TracerBeam && TracerMesh)
    {
        TracerBeam->SetStaticMesh(TracerMesh);
        if (TracerMaterial)
        {
            TracerBeam->SetMaterial(0, TracerMaterial);
        }
    }

    if (ATDGameState* GS = GetWorld()->GetGameState<ATDGameState>())
    {
        UpgradeLevel = GS->GetUpgradeLevel(); // une tourelle posée tard hérite du niveau actuel
        GS->OnUpgradeLevelChanged.AddDynamic(this, &ATurret::OnUpgradeChanged);
    }

    RangeSphere->OnComponentBeginOverlap.AddDynamic(this, &ATurret::OnRangeBeginOverlap);
    RangeSphere->OnComponentEndOverlap.AddDynamic(this, &ATurret::OnRangeEndOverlap);

    GetWorldTimerManager().SetTimer(TargetTimer, this, &ATurret::UpdateTarget, 0.1f, true);

    RecalcStats();
}

void ATurret::OnUpgradeChanged(int32 NewLevel)
{
    UpgradeLevel = NewLevel;
    RecalcStats();
}

void ATurret::OnRangeBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (AEnemy* Enemy = Cast<AEnemy>(OtherActor))
    {
        EnemiesInRange.AddUnique(Enemy);
    }
}

void ATurret::OnRangeEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (AEnemy* Enemy = Cast<AEnemy>(OtherActor))
    {
        EnemiesInRange.Remove(Enemy);
    }
}

void ATurret::UpdateTarget()
{
    EnemiesInRange.RemoveAll([](const TWeakObjectPtr<AEnemy>& E) { return !E.IsValid(); });

    Target = nullptr;
    float BestDistSq = TNumericLimits<float>::Max();

    for (const TWeakObjectPtr<AEnemy>& E : EnemiesInRange)
    {
        const float DistSq = FVector::DistSquared(GetActorLocation(), E->GetActorLocation());
        if (DistSq < BestDistSq)
        {
            BestDistSq = DistSq;
            Target = E;
        }
    }

    if (Target.IsValid())
    {
        const FRotator Look = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), Target->GetActorLocation());
        SetActorRotation(FRotator(0.f, Look.Yaw, 0.f));
    }
}

void ATurret::Fire()
{
    AEnemy* Enemy = Target.Get();
    if (!Enemy) return;

    if (FireMontage && Mesh && Mesh->GetAnimInstance())
    {
        Mesh->GetAnimInstance()->Montage_Play(FireMontage);
    }

    if (TracerBeam && TracerBeam->GetStaticMesh())
    {
        const FVector Muzzle = GetActorLocation() + FVector(0.f, 0.f, 120.f);
        const FVector Hit = Enemy->GetActorLocation() + FVector(0.f, 0.f, 80.f);
        const FVector Delta = Hit - Muzzle;
        const float Length = Delta.Size();

        if (Length > 1.f)
        {
            TracerBeam->SetWorldLocation(Muzzle + Delta * 0.5f);
            TracerBeam->SetWorldRotation(FRotationMatrix::MakeFromZ(Delta.GetSafeNormal()).Rotator());
            TracerBeam->SetWorldScale3D(FVector(TracerThickness / 100.f, TracerThickness / 100.f, Length / 100.f));
            TracerBeam->SetVisibility(true);

            GetWorldTimerManager().SetTimer(TracerTimer, this, &ATurret::HideTracer, TracerDuration, false);
        }
    }

    Enemy->ReceiveDamage(CurrentDamage);
}

void ATurret::HideTracer()
{
    if (TracerBeam)
    {
        TracerBeam->SetVisibility(false);
    }
}