#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Navigation/PathFollowingComponent.h"
#include "Damageable.h"
#include "Enemy.generated.h"

class APathActor;
class AEnemy;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEnemyDied, AEnemy*, Enemy);

UCLASS()
class WESTERNTOWERDEFENSE_API AEnemy : public ACharacter, public IDamageable
{
	GENERATED_BODY()

public:
	AEnemy();

	virtual void ReceiveDamage(float Amount) override;

	UFUNCTION(BlueprintCallable)
	void SetPath(APathActor* NewPath) { Path = NewPath; }

	UFUNCTION(BlueprintCallable)
	int32 GetReward() const { return Reward; }

	UPROPERTY(BlueprintAssignable)
	FOnEnemyDied OnEnemyDied;

protected:
	virtual void BeginPlay() override;
	virtual void PossessedBy(AController* NewController) override;
	FTimerHandle RetryTimer;
	UFUNCTION()
	void OnMoveCompleted(FAIRequestID RequestID, EPathFollowingResult::Type Result);

	void MoveToNextWaypoint();
	void ReachedEnd();
	void Die();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
	float MaxHealth = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
	float MoveSpeed = 300.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
	float DamageToSaloon = 10.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
	int32 Reward = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
	TObjectPtr<APathActor> Path;

	float CurrentHealth = 100.f;
	int32 CurrentWaypoint = 0;
	bool bStarted = false;
	bool bDead = false;
};