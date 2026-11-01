#include "RenameAssetAction.h"
#include "EditorUtilityLibrary.h"

void URenameAssetAction::AddBPPrefix()
{
    const TArray<UObject*> Selected = UEditorUtilityLibrary::GetSelectedAssets();
    const FString Prefix = TEXT("BP_");
    int32 Renamed = 0;

    for (UObject* Asset : Selected)
    {
        if (!Asset) continue;

        const FString Name = Asset->GetName();
        if (Name.StartsWith(Prefix)) continue; // deja prefixe

        UEditorUtilityLibrary::RenameAsset(Asset, Prefix + Name);
        ++Renamed;
    }

    UE_LOG(LogTemp, Display, TEXT("AddBPPrefix : %d asset(s) renomme(s)"), Renamed);
}
