#include "Turret.h"
#include "Enemy.h"
#include "TurretDataAsset.h"
#include "TDGameState.h"
#include "Components/SphereComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "DrawDebugHelpers.h"
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

    if (Data->Mesh) Mesh->SetSkeletalMesh(Data->Mesh);
}

void ATurret::RecalcStats()
{
    // Chaque niveau : +25 % de dégâts, +15 % de cadence, +10 % de portée
    CurrentRange = Range * (1.f + 0.10f * UpgradeLevel);
    CurrentDamage = Damage * (1.f + 0.25f * UpgradeLevel);
    CurrentFireRate = FireRate * (1.f + 0.15f * UpgradeLevel);

    RangeSphere->SetSphereRadius(CurrentRange);

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

    if (ATDGameState* GS = GetWorld()->GetGameState<ATDGameState>())
    {
        UpgradeLevel = GS->GetUpgradeLevel(); // une tourelle posée tard hérite du niveau actuel
        GS->OnUpgradeLevelChanged.AddDynamic(this, &ATurret::OnUpgradeChanged);
    }

    RangeSphere->OnComponentBeginOverlap.AddDynamic(this, &ATurret::OnRangeBeginOverlap);
    RangeSphere->OnComponentEndOverlap.AddDynamic(this, &ATurret::OnRangeEndOverlap);

    GetWorldTimerManager().SetTimer(TargetTimer, this, &ATurret::UpdateTarget, 0.1f, true);
    GetWorldTimerManager().SetTimer(FireTimer, this, &ATurret::Fire,
        1.f / FMath::Max(CurrentFireRate, 0.01f), true);

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
    if (!Target.IsValid()) return;

    if (FireMontage && Mesh->GetAnimInstance())
    {
        Mesh->GetAnimInstance()->Montage_Play(FireMontage);
    }

    DrawDebugLine(GetWorld(), GetActorLocation() + FVector(0, 0, 120.f),
        Target->GetActorLocation(), FColor::Yellow, false, 0.1f, 0, 3.f);

    Target->ReceiveDamage(CurrentDamage);
}