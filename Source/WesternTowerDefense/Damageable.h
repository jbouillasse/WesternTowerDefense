#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Damageable.generated.h"

UINTERFACE(MinimalAPI, NotBlueprintable)
class UDamageable : public UInterface
{
	GENERATED_BODY()
};

class WESTERNTOWERDEFENSE_API IDamageable
{
	GENERATED_BODY()

public:
	virtual void ReceiveDamage(float Amount) = 0;
};