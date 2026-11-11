#include "Train.h"
#include "RailPath.h"

ATrain::ATrain()
{
	PrimaryActorTick.bCanEverTick = true;
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);
}

void ATrain::Init(ARailPath* InRail)
{
	Rail = InRail;
	Distance = 0.f;
	Speed = 0.f;
	State = EState::Arriving;
	UpdateTransform();
}

void ATrain::Depart()
{
	if (State == EState::Stopped)
	{
		State = EState::Departing;
	}
}

void ATrain::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (!Rail) return;

	switch (State)
	{
	case EState::Arriving:
		{
			const float StationDistance = Rail->GetStationDistance();
			const float Remaining = StationDistance - Distance;

			float TargetSpeed = MaxSpeed;
			if (Remaining < BrakingDistance)
			{
				TargetSpeed = FMath::Max(150.f, MaxSpeed * Remaining / BrakingDistance);
			}

			Speed = FMath::FInterpConstantTo(Speed, TargetSpeed, DeltaTime, Acceleration * 2.f);
			Distance += Speed * DeltaTime;

			if (Distance >= StationDistance)
			{
				Distance = StationDistance;
				Speed = 0.f;
				State = EState::Stopped;
				OnTrainStopped.Broadcast();
			}
			break;
		}
	case EState::Departing:
		Speed = FMath::FInterpConstantTo(Speed, MaxSpeed, DeltaTime, Acceleration);
		Distance += Speed * DeltaTime;
		if (Distance >= Rail->GetLength())
		{
			Destroy();
			return;
		}
		break;
	default:
		break;
	}

	UpdateTransform();
}

void ATrain::UpdateTransform()
{
	if (!Rail) return;
	SetActorLocationAndRotation(Rail->GetLocationAtDistance(Distance), Rail->GetRotationAtDistance(Distance));
}