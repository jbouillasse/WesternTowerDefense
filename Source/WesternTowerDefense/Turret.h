#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Turret.generated.h"

class USphereComponent;
class USkeletalMeshComponent;
class UStaticMeshComponent;
class UStaticMesh;
class UMaterialInterface;
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
    void HideTracer();
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TObjectPtr<USceneComponent> Root;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TObjectPtr<USkeletalMeshComponent> Mesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TObjectPtr<USphereComponent> RangeSphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TObjectPtr<UStaticMeshComponent> TracerBeam;

    //Data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turret")
    TObjectPtr<UTurretDataAsset> Data;

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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turret|Tracer")
    TObjectPtr<UStaticMesh> TracerMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turret|Tracer")
    TObjectPtr<UMaterialInterface> TracerMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turret|Tracer")
    float TracerThickness = 10.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turret|Tracer")
    float TracerDuration = 0.08f;
    
    float CurrentRange = 800.f;
    float CurrentDamage = 25.f;
    float CurrentFireRate = 1.f;
    int32 UpgradeLevel = 0;

    TArray<TWeakObjectPtr<AEnemy>> EnemiesInRange;
    TWeakObjectPtr<AEnemy> Target;

    FTimerHandle TargetTimer;
    FTimerHandle FireTimer;
    FTimerHandle TracerTimer;
};