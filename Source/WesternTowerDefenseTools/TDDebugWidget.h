#pragma once

#include "CoreMinimal.h"
#include "EditorUtilityWidget.h"
#include "TDDebugWidget.generated.h"

UCLASS()
class WESTERNTOWERDEFENSETOOLS_API UTDDebugWidget : public UEditorUtilityWidget
{
    GENERATED_BODY()

public:
    //Variables de debug
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bShowRanges = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    float DebugDuration = 10.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    FColor RangeColor = FColor::Green;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug", meta = (ClampMin = "1.0", ClampMax = "64.0"))
    float LineThickness = 8.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bShowEnemyPaths = false;

    // --- Boutons ---
    UFUNCTION(BlueprintCallable, Category = "Debug")
    void DrawTurretRanges();

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void DrawEnemyPaths();

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void ClearDebugDrawing();

private:
    UWorld* GetTargetWorld() const;
};
