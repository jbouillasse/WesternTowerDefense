#include "Turret.h"
#include "Enemy.h"
#include "Components/SphereComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"
#include "TimerManager.h"

ATurret::ATurret()
{
    PrimaryActorTick.bCanEverTick = false;

    Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    SetRootComponent(Root);

    Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));
    Mesh->SetupAttachment(Root);
    Mesh->SetCollisionProfileName(TEXT("NoCollision"));

    RangeSphere = CreateDefaultSubobject<USphereComponent>(TEXT("RangeSphere"));
    RangeSphere->SetupAttachment(Root);
    RangeSphere->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
    RangeSphere->SetGenerateOverlapEvents(true);
    RangeSphere->SetHiddenInGame(true);
}

void ATurret::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);
    RangeSphere->SetSphereRadius(Range);
}

void ATurret::BeginPlay()
{
    Super::BeginPlay();

    RangeSphere->SetSphereRadius(Range);
    RangeSphere->OnComponentBeginOverlap.AddDynamic(this, &ATurret::OnRangeBeginOverlap);
    RangeSphere->OnComponentEndOverlap.AddDynamic(this, &ATurret::OnRangeEndOverlap);

    GetWorldTimerManager().SetTimer(TargetTimer, this, &ATurret::UpdateTarget, 0.1f, true);
    GetWorldTimerManager().SetTimer(FireTimer, this, &ATurret::Fire, 1.f / FMath::Max(FireRate, 0.01f), true);
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
        // Le tireur entier se tourne vers l'ennemi le plus proche
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

    Target->ReceiveDamage(Damage);
}
//1234