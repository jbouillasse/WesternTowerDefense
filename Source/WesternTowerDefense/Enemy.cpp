#include "NavigationSystem.h"
#include "Enemy.h"
#include "PathActor.h"
#include "Saloon.h"
#include "AIController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/CharacterMovementComponent.h"

AEnemy::AEnemy()
{
    PrimaryActorTick.bCanEverTick = false;

    AIControllerClass = AAIController::StaticClass();
    AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

    bUseControllerRotationYaw = false;
    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0.f, 540.f, 0.f);
}

void AEnemy::BeginPlay()
{
    Super::BeginPlay();
    CurrentHealth = MaxHealth;
    GetCharacterMovement()->MaxWalkSpeed = MoveSpeed;
}

void AEnemy::PossessedBy(AController* NewController)
{
    Super::PossessedBy(NewController);

    CurrentHealth = MaxHealth;
    GetCharacterMovement()->MaxWalkSpeed = MoveSpeed;

    if (AAIController* AI = Cast<AAIController>(NewController))
    {
        AI->ReceiveMoveCompleted.AddUniqueDynamic(this, &AEnemy::OnMoveCompleted);
    }

    if (!Path)
    {
        UE_LOG(LogTemp, Error, TEXT("Enemy %s n'a pas de Path assigne"), *GetName());
        return;
    }

    if (!bStarted)
    {
        bStarted = true;
        CurrentWaypoint = 1; // le point 0 est le point de depart
        MoveToNextWaypoint();
    }
}

void AEnemy::MoveToNextWaypoint()
{
    if (!Path || bDead) return;

    if (CurrentWaypoint >= Path->GetWaypointCount())
    {
        ReachedEnd();
        return;
    }

    const FVector Dest = Path->GetWaypoint(CurrentWaypoint);

    UNavigationSystemV1* Nav = UNavigationSystemV1::GetCurrent(GetWorld());
    FNavLocation Projected;

    if (!Nav || !Nav->ProjectPointToNavigation(Dest, Projected, FVector(500.f, 500.f, 3000.f)))
    {
        UE_LOG(LogTemp, Warning, TEXT("NavMesh pas pret pour le waypoint %d, nouvel essai..."), CurrentWaypoint);
        GetWorldTimerManager().SetTimer(RetryTimer, this, &AEnemy::MoveToNextWaypoint, 0.5f, false);
        return;
    }

    if (AAIController* AI = Cast<AAIController>(GetController()))
    {
        AI->MoveToLocation(Projected.Location, 50.f, true, true, true);
    }
}

void AEnemy::OnMoveCompleted(FAIRequestID RequestID, EPathFollowingResult::Type Result)
{
    if (bDead) return;

    if (Result == EPathFollowingResult::Success)
    {
        CurrentWaypoint++;
        MoveToNextWaypoint();
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Enemy %s: deplacement echoue vers le waypoint %d"), *GetName(), CurrentWaypoint);
        GetWorldTimerManager().SetTimer(RetryTimer, this, &AEnemy::MoveToNextWaypoint, 0.5f, false);
    }
}

void AEnemy::ReachedEnd()
{
    if (AActor* Found = UGameplayStatics::GetActorOfClass(GetWorld(), ASaloon::StaticClass()))
    {
        if (IDamageable* Target = Cast<IDamageable>(Found))
        {
            Target->ReceiveDamage(DamageToSaloon);
        }
    }
    bDead = true;
    Destroy();
}

void AEnemy::ReceiveDamage(float Amount)
{
    if (bDead) return;

    CurrentHealth = FMath::Clamp(CurrentHealth - Amount, 0.f, MaxHealth);
    if (CurrentHealth <= 0.f)
    {
        Die();
    }
}

void AEnemy::Die()
{
    bDead = true;
    OnEnemyDied.Broadcast(this);
    Destroy();
}