#pragma once

#include "CoreMinimal.h"
#include "AssetActionUtility.h"
#include "RenameAssetAction.generated.h"

UCLASS()
class WESTERNTOWERDEFENSETOOLS_API URenameAssetAction : public UAssetActionUtility
{
    GENERATED_BODY()

public:
    // Ajoute BP_ à tous les assets sélectionnés
    UFUNCTION(CallInEditor, Category = "Western Defense")
    void AddBPPrefix();
};
