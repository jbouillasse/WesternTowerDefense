#include "PathActor.h"
#include "Components/SplineComponent.h"

APathActor::APathActor()
{
	PrimaryActorTick.bCanEverTick = false;
	Spline = CreateDefaultSubobject<USplineComponent>(TEXT("Spline"));
	SetRootComponent(Spline);
}

int32 APathActor::GetWaypointCount() const
{
	return Spline->GetNumberOfSplinePoints();
}

FVector APathActor::GetWaypoint(int32 Index) const
{
	return Spline->GetLocationAtSplinePoint(Index, ESplineCoordinateSpace::World);
}