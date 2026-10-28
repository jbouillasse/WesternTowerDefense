#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Turret.generated.h"

class USphereComponent;
class USkeletalMeshComponent;
class UAnimMontage;
class UTurretDataAsset;
class AEnemy;

UCLASS()
class WESTERNTOWERDEFENSE_API ATurret : public AActor
{
    GENERATED_BODY()

public:
    ATurret();

    virtual void OnConstruction(const FTransform& Transform) override;

    // Appelé par l'emplacement avant FinishSpawning
    void SetData(UTurretDataAsset* NewData);

    float GetRange() const { return CurrentRange; }
    int32 GetCost() const { return Cost; }

protected:
    virtual void BeginPlay() override;

    UFUNCTION()
    void OnRangeBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnRangeEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    UFUNCTION()
    void OnUpgradeChanged(int32 NewLevel);

    void ApplyData();
    void RecalcStats();
    void UpdateTarget();
    void Fire();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TObjectPtr<USceneComponent> Root;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TObjectPtr<USkeletalMeshComponent> Mesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TObjectPtr<USphereComponent> RangeSphere;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turret")
    TObjectPtr<UTurretDataAsset> Data;

    // Valeurs de base (remplacées par le Data Asset s'il y en a un)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turret")
    float Range = 800.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turret")
    float Damage = 25.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turret")
    float FireRate = 1.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turret")
    int32 Cost = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turret")
    TObjectPtr<UAnimMontage> FireMontage;

    // Valeurs réellement utilisées (base + amélioration)
    float CurrentRange = 800.f;
    float CurrentDamage = 25.f;
    float CurrentFireRate = 1.f;
    int32 UpgradeLevel = 0;

    TArray<TWeakObjectPtr<AEnemy>> EnemiesInRange;
    TWeakObjectPtr<AEnemy> Target;

    FTimerHandle TargetTimer;
    FTimerHandle FireTimer;
};