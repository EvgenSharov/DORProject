// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GridDataReader.h"
#include "Pathfinder.generated.h"

UCLASS(Blueprintable)
class PATHFINDINGMODULE_API APathfinder : public AActor
{
	GENERATED_BODY()

private:
	UPROPERTY(VisibleAnywhere)
	USceneComponent* Root;

#if WITH_EDITOR
	UPROPERTY(Transient)
	class UProceduralMeshComponent* ProceduralMesh;

	UPROPERTY()
	TArray<FColor> GridColors;

	TMap<int32, FColor> PathfindingColors;
#endif

	UPROPERTY()
	class USquareGrid* Grid;

public:
	UPROPERTY(EditAnywhere, Category = Grid)
	AGridDataReader* RegionDataReader;

	UPROPERTY(EditAnywhere, Category = Grid, meta = (ClampMin = "1"))
	int32 Step;

#if WITH_EDITOR
	UPROPERTY(EditAnywhere, Category = Grid)
	bool bClickToRebuild;

	UPROPERTY(EditAnywhere, Category = Debug)
	bool bDrawGrid;

	UPROPERTY(EditAnywhere, Category = Debug)
	FVector DrawOffset;

	UPROPERTY(EditAnywhere, Category = Debug)
	bool bVisualizePathfinding;
#endif

public:
	APathfinder(const FObjectInitializer& ObjectInitializer);

private:
	bool IsValid();
	void BuildGrid();
	class ALevelData* FindLevelData();
	void WorldToGrid(FVector& Point, int32& Out_X, int32& Out_Y);
	
#if WITH_EDITOR
	void BuildGridDebug();
	void ApplyGridColors();
#endif

public:
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void Tick(float DeltaSeconds) override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& E) override;
#endif

	UFUNCTION(BlueprintCallable, Category = Pathfinder)
	void FindPath(FVector From, FVector To, TArray<FVector2D>& Out_Path, bool& Out_bResult);
};
