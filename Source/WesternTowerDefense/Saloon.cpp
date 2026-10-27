#include "Saloon.h"
#include "Components/BoxComponent.h"

ASaloon::ASaloon()
{
	PrimaryActorTick.bCanEverTick = false;
	Box = CreateDefaultSubobject<UBoxComponent>(TEXT("Box"));
	Box->SetBoxExtent(FVector(300.f, 300.f, 200.f));
	Box->SetCollisionProfileName(TEXT("NoCollision"));
	SetRootComponent(Box);
}

void ASaloon::BeginPlay()
{
	Super::BeginPlay();
	CurrentHealth = MaxHealth;
	OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
}

void ASaloon::ReceiveDamage(float Amount)
{
	if (CurrentHealth <= 0.f) return;

	CurrentHealth = FMath::Clamp(CurrentHealth - Amount, 0.f, MaxHealth);
	UE_LOG(LogTemp, Warning, TEXT("Saloon PV : %.0f / %.0f"), CurrentHealth, MaxHealth);
	OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);

	if (CurrentHealth <= 0.f)
	{
		UE_LOG(LogTemp, Warning, TEXT("Le saloon est detruit !"));
		OnSaloonDestroyed.Broadcast();
	}
}