#include "RailPath.h"
#include "Components/SplineComponent.h"

ARailPath::ARailPath()
{
	PrimaryActorTick.bCanEverTick = false;
	Spline = CreateDefaultSubobject<USplineComponent>(TEXT("Spline"));
	SetRootComponent(Spline);
}

float ARailPath::GetLength() const
{
	return Spline->GetSplineLength();
}

float ARailPath::GetStationDistance() const
{
	const int32 Index = FMath::Clamp(StationPointIndex, 0, Spline->GetNumberOfSplinePoints() - 1);
	return Spline->GetDistanceAlongSplineAtSplinePoint(Index);
}

FVector ARailPath::GetLocationAtDistance(float Distance) const
{
	return Spline->GetLocationAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);
}

FRotator ARailPath::GetRotationAtDistance(float Distance) const
{
	return Spline->GetRotationAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);
}