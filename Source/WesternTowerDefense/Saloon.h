#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Damageable.h"
#include "Saloon.generated.h"

class UBoxComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSaloonHealthChanged, float, CurrentHealth, float, MaxHealth);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSaloonDestroyed);

UCLASS()
class WESTERNTOWERDEFENSE_API ASaloon : public AActor, public IDamageable
{
	GENERATED_BODY()

public:
	ASaloon();

	virtual void ReceiveDamage(float Amount) override;

	UFUNCTION(BlueprintCallable)
	float GetCurrentHealth() const { return CurrentHealth; }

	UPROPERTY(BlueprintAssignable)
	FOnSaloonHealthChanged OnHealthChanged;

	UPROPERTY(BlueprintAssignable)
	FOnSaloonDestroyed OnSaloonDestroyed;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UBoxComponent> Box;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Saloon")
	float MaxHealth = 100.f;

	float CurrentHealth = 100.f;
};