// Fill out your copyright notice in the Description page of Project Settings.

#include "DOR.h"
#include "LevelData.h"
#include "Engine.h"

ALevelData::ALevelData()
{
	MapExtensionX = FVector2D(-200.0f, 0.0f);
	MapExtensionY = FVector2D(0.0f, 200.0f);
	ClampThreshold = 1.0f;

	LineRangeZ = FVector2D(-100.0f, 200.0f);
	bDebugRaycasts = false;
	DebugDrawDuration = 1.0f;
	
	//PrimaryActorTick.bCanEverTick = true;
}

void ALevelData::BeginPlay()
{
	Super::BeginPlay();
}

void ALevelData::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ALevelData::ClampToMapBounds(float WorldX, float WorldY, float& Out_ClampedWorldX, float& Out_ClampedWorldY)
{
	Out_ClampedWorldX = FMath::Clamp(WorldX, MapExtensionX.X + ClampThreshold, MapExtensionX.Y - ClampThreshold);
	Out_ClampedWorldY = FMath::Clamp(WorldY, MapExtensionY.X + ClampThreshold, MapExtensionY.Y - ClampThreshold);
}

void ALevelData::ProjectToTerrainBase(float WorldX, float WorldY, FVector& Out_Point)
{
	FVector Start(WorldX, WorldY, LineRangeZ.Y);
	FVector End(WorldX, WorldY, LineRangeZ.X);

	bool bHit;
	LineCastToTerrainBase(Start, End, bHit, Out_Point);
	
	Out_Point.X = WorldX;
	Out_Point.Y = WorldY;
}

void ALevelData::LineCastToTerrainBase(FVector Start, FVector End, bool& Out_bHit, FVector& Out_Point)
{
	FHitResult HitResult;
	if (!bDebugRaycasts)
	{
		static const FName LineTraceSingleName(TEXT("LineTraceSingle"));
		FCollisionQueryParams Params(LineTraceSingleName, true);
		Params.bReturnPhysicalMaterial = false;
		Params.bReturnFaceIndex = false;
		Params.bTraceAsyncScene = true;

		FCollisionObjectQueryParams ObjectParams;
		ObjectParams.AddObjectTypesToQuery(UEngineTypes::ConvertToCollisionChannel(TerrainBaseType));

		Out_bHit = GetWorld()->LineTraceSingleByObjectType(HitResult, Start, End, ObjectParams, Params);
	}
	else
	{
		TArray<AActor*> ActorsToIgnore;
		TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
		ObjectTypes.Add(TerrainBaseType);

		Out_bHit = UKismetSystemLibrary::LineTraceSingleForObjects(this, Start, End, ObjectTypes, true, ActorsToIgnore, EDrawDebugTrace::ForDuration, HitResult, false, FLinearColor::Blue, FLinearColor::White, DebugDrawDuration);
	}
	Out_Point = Out_bHit ? HitResult.Location : FVector::ZeroVector;
}

void ALevelData::CameraCastToTerrainBase(float castLength, bool& out_hit, FVector& out_point)
{
	APlayerController* pc = UGameplayStatics::GetPlayerController(this, 0);
	if (pc)
	{
		APlayerCameraManager* pcm = pc->PlayerCameraManager;
		FVector cameraLocation = pcm->GetCameraLocation();
		
		FVector worldPosition;
		FVector worldDirection;
		pc->DeprojectMousePositionToWorld(worldPosition, worldDirection);

		FVector endPoint = cameraLocation + worldDirection * castLength;

		LineCastToTerrainBase(cameraLocation, endPoint, out_hit, out_point);
	}
	else
	{
		out_hit = false;
	}
}

void ALevelData::CameraCastToTerrainBaseDefaultLength(bool& out_hit, FVector& out_point)
{
	CameraCastToTerrainBase(2000, out_hit, out_point);
}
