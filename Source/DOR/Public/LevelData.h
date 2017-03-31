// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "LevelData.generated.h"

UCLASS(BlueprintType)
class DOR_API ALevelData : public AActor
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = Map)
	TEnumAsByte<EObjectTypeQuery> TerrainBaseType;

	UPROPERTY(EditAnywhere, Category = Map)
	FVector2D MapExtensionX;

	UPROPERTY(EditAnywhere, Category = Map)
	FVector2D MapExtensionY;

	UPROPERTY(EditAnywhere, Category = Map)
	float ClampThreshold;

	UPROPERTY(EditAnywhere, Category = Raycast)
	FVector2D LineRangeZ;

	UPROPERTY(EditAnywhere, Category = Raycast)
	bool bDebugRaycasts;

	UPROPERTY(EditAnywhere, Category = Raycast)
	float DebugDrawDuration;

public:	
	ALevelData();

public:
	virtual void BeginPlay() override;
	
	virtual void Tick( float DeltaSeconds ) override;

	UFUNCTION(BlueprintCallable, Category = LevelData)
	void ClampToMapBounds(float WorldX, float WorldY, float& Out_ClampedWorldX, float& Out_ClampedWorldY);

	UFUNCTION(BlueprintCallable, Category = LevelData)
	void ProjectToTerrainBase(float WorldX, float WorldY, FVector& Out_Point);

	UFUNCTION(BlueprintCallable, Category = LevelData)
	void LineCastToTerrainBase(FVector Start, FVector End, bool& Out_bHit, FVector& Out_Point);

	UFUNCTION(BlueprintCallable, Category = LevelData)
	void CameraCastToTerrainBase(float castLength, bool& out_hit, FVector& out_point);

	UFUNCTION(BlueprintCallable, Category = LevelData)
	void CameraCastToTerrainBaseDefaultLength(bool& out_hit, FVector& out_point);
};
