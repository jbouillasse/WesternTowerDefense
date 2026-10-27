#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Turret.generated.h"

class USphereComponent;
class USkeletalMeshComponent;
class AEnemy;
class UTurretDataAsset;

UCLASS()
class WESTERNTOWERDEFENSE_API ATurret : public AActor
{
	GENERATED_BODY()

public:
	ATurret();

	virtual void OnConstruction(const FTransform& Transform) override;

	float GetRange() const { return Range; }

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnRangeBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnRangeEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turret")
	TObjectPtr<UTurretDataAsset> Data;

	void ApplyData();
	void UpdateTarget();
	void Fire();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USkeletalMeshComponent> Mesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USphereComponent> RangeSphere;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turret")
	float Range = 800.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turret")
	float Damage = 25.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turret")
	float FireRate = 1.f; // tirs par seconde

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turret")
	int32 Cost = 50;

	// Optionnel : animation de tir jouée à chaque coup
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turret")
	TObjectPtr<UAnimMontage> FireMontage;

	TArray<TWeakObjectPtr<AEnemy>> EnemiesInRange;
	TWeakObjectPtr<AEnemy> Target;

	FTimerHandle TargetTimer;
	FTimerHandle FireTimer;
};