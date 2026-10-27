#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "WaveTypes.generated.h"

class UEnemyDataAsset;

USTRUCT(BlueprintType)
struct FWaveEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UEnemyDataAsset> Enemy = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "1"))
	int32 Count = 1;
};

USTRUCT(BlueprintType)
struct FWaveRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FWaveEntry> Entries;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpawnInterval = 1.5f;
};