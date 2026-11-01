#include "TDDebugWidget.h"
#include "Turret.h"
#include "PathActor.h"
#include "Editor.h"
#include "DrawDebugHelpers.h"
#include "EngineUtils.h"

UWorld* UTDDebugWidget::GetTargetWorld() const
{
    if (!GEditor) return nullptr;

    // En jeu (PIE) on dessine dans le monde de jeu, sinon dans le monde de l'éditeur
    if (GEditor->PlayWorld) return GEditor->PlayWorld;
    return GEditor->GetEditorWorldContext().World();
}

void UTDDebugWidget::DrawTurretRanges()
{
    UWorld* World = GetTargetWorld();
    if (!World || !bShowRanges) return;

    int32 Count = 0;
    for (TActorIterator<ATurret> It(World); It; ++It)
    {
        const ATurret* Turret = *It;
        DrawDebugSphere(World, Turret->GetActorLocation(), Turret->GetRange(), 32,
            RangeColor, false, DebugDuration, 0, LineThickness);
        ++Count;
    }

    UE_LOG(LogTemp, Display, TEXT("DrawTurretRanges : %d tourelle(s)"), Count);
}

void UTDDebugWidget::DrawEnemyPaths()
{
    UWorld* World = GetTargetWorld();
    if (!World || !bShowEnemyPaths) return;

    for (TActorIterator<APathActor> It(World); It; ++It)
    {
        const APathActor* Path = *It;
        const int32 N = Path->GetWaypointCount();
        for (int32 i = 0; i < N; ++i)
        {
            DrawDebugSphere(World, Path->GetWaypoint(i), 60.f, 8, FColor::Cyan, false, DebugDuration, 0, 6.f);
            if (i > 0)
            {
                DrawDebugLine(World, Path->GetWaypoint(i - 1), Path->GetWaypoint(i),
                    FColor::Cyan, false, DebugDuration, 0, 8.f);
            }
        }
    }
}

void UTDDebugWidget::ClearDebugDrawing()
{
    if (UWorld* World = GetTargetWorld())
    {
        FlushPersistentDebugLines(World);
    }
}
