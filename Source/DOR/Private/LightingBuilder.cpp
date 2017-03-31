#include "DOR.h"

#if WITH_EDITOR

#include "LightingBuilder.h"
#include "RawMesh.h"
#include "PhysicsEngine/BodySetup.h"
#include "StaticMeshResources.h"
#include "DarkOmenLight.h"
#include "RenderResource.h"

#include "UnrealEd.h"

bool Trace(AActor* WorldContext, UWorld* World, FVector& Start, FVector& End)
{
	FHitResult HitOut;

	//FCollisionQueryParams TraceParams(FName(TEXT("DO Lighting Trace")), true);
	//TraceParams.bReturnPhysicalMaterial = false;
	//bool bResult = World->LineTraceSingleByChannel(HitOut, Start, End, ECC_Visibility, TraceParams);

	TArray<AActor*> Actors;
	bool bResult = UKismetSystemLibrary::LineTraceSingle(WorldContext, Start, End, UEngineTypes::ConvertToTraceType(ECC_Visibility), true, Actors, EDrawDebugTrace::None/*ForDuration*/, HitOut, false, FLinearColor::Blue, FLinearColor::White, 10.0f);
	
	if (bResult)
	{
		// Since we don't ignore self, we must check if trace hit end point
		return !FVector::PointsAreNear(HitOut.ImpactPoint, End, 1e-3f);
	}

	return bResult;
}

void AddPointLight(AActor* WorldContext, UWorld* World, FLinearColor& Color, FVector& Point, FVector& Normal, UDarkOmenPointLightComponent* LightComp)
{
	FVector LightPosition = LightComp->GetOwner()->GetActorLocation();
	FVector DirToLight = LightPosition - Point;
	float DistToLight = DirToLight.Size();
	DirToLight /= DistToLight;

	float Attenuation = LightComp->AttenuationCoeff == 0.0f ? 1.0f : (1.0f / (4.0f + DistToLight * LightComp->AttenuationCoeff));
	float Intensity = FMath::Max(0.0f, FVector::DotProduct(DirToLight, Normal));
	Intensity *= Attenuation;
	
	if (LightComp->bCastLight)
	{
		float ShadowFactor = 1.0f;
		if (LightComp->bCastShadow)
		{
			bool bHit = Trace(WorldContext, World, LightPosition, Point);
			ShadowFactor = bHit ? 0.0f : 1.0f;
		}

		FLinearColor ItemColor = (ShadowFactor * Intensity) * LightComp->LightColor;
		ItemColor = ItemColor.GetClamped();
		Color += ItemColor;
	}
	else if (LightComp->bCastShadow)
	{
		bool bHit = Trace(WorldContext, World, LightPosition, Point);
		if (bHit)
		{
			Color *= 0.5f;
		}
	}
}

void AddDirectionalLight(AActor* WorldContext, UWorld* World, FLinearColor& Color, FVector& Point, FVector& Normal, UDarkOmenDirectionalLightComponent* LightComp)
{
	FVector LightDirection = -LightComp->GetForwardVector();
	FVector StartPoint = Point + LightDirection * 10000.0f;

	float Intensity = FMath::Max(0.0f, FVector::DotProduct(LightDirection, Normal));
	
	if (LightComp->bCastLight)
	{
		float ShadowFactor = 1.0f;
		if (LightComp->bCastShadow)
		{		
			bool bHit = Trace(WorldContext, World, StartPoint, Point);
			ShadowFactor = bHit ? 0.0f : 1.0f;
		}

		FLinearColor ItemColor = (Intensity * ShadowFactor) * LightComp->LightColor;
		ItemColor = ItemColor.GetClamped();
		Color += ItemColor;
	}
	else if (LightComp->bCastShadow)
	{
		bool bHit = Trace(WorldContext, World, StartPoint, Point);
		if (bHit)
		{
			Color *= 0.5f;
		}
	}
}

FLinearColor CalcLighting(AActor* WorldContext, UWorld* World, bool bBase, FVector& Point, FVector& Normal, FLinearColor& AmbientColor, TArray<UDarkOmenLightComponent*>& Lights)
{
	FLinearColor TotalColor(0, 0, 0);

	for (int32 Index = 0; Index < Lights.Num(); ++Index)
	{
		UDarkOmenLightComponent* LightComp = Lights[Index];
		if ((LightComp->bAffectsBase && bBase) || (LightComp->bAffectsFurn && !bBase))
		{
			EDarkOmenLightType LightType = LightComp->GetLightType();
			if (LightType == EDarkOmenLightType::Point)
			{
				AddPointLight(WorldContext, World, TotalColor, Point, Normal, Cast<UDarkOmenPointLightComponent>(LightComp));
			}
			else if (LightType == EDarkOmenLightType::Directional)
			{
				AddDirectionalLight(WorldContext, World, TotalColor, Point, Normal, Cast<UDarkOmenDirectionalLightComponent>(LightComp));
			}
		}
	}

	TotalColor += AmbientColor;
	TotalColor = TotalColor.GetClamped();

	return TotalColor;
}

void ULightingBuilder::BuildDOLighting(UWorld* World)
{
	ULevel* Level = World->PersistentLevel;
	FLinearColor AmbientColor(0.3f, 0.3f, 0.3f);
	
	// Gather all enabled lights
	TArray<UDarkOmenLightComponent*> Lights;
	for (TObjectIterator<UDarkOmenLightComponent> It; It; ++It)
	{
		UDarkOmenLightComponent* LightComp = *It;
		AActor* Owner = LightComp->GetOwner();
		if (Owner)
		{
			bool bIsValid = World->ContainsActor(Owner) && !Owner->IsPendingKill();
			if (bIsValid)
			{
				if (LightComp->bEnabled)
				{
					Lights.Add(LightComp);
				}
			}
		}
	}
	auto SortFunc = [](UDarkOmenLightComponent& v0, UDarkOmenLightComponent& v1)
	{
		return v0.GetOwner()->GetActorLabel().Compare(v1.GetOwner()->GetActorLabel()) <= 0;
	};
	Lights.Sort(SortFunc); // Sort lights by name

	// Iterate over all geometry
	for (TObjectIterator<UStaticMeshComponent> It; It; ++It)
	{
		UStaticMeshComponent* MeshComp = *It;
		AActor* Owner = MeshComp->GetOwner();
		if (Owner)
		{
			bool bIsValid = World->ContainsActor(Owner) && !Owner->IsPendingKill();
			if (bIsValid)
			{
				// Hacky check
				bool bBase = Owner->GetActorLabel().Contains(FString(TEXT("base"))) || Owner->GetActorLabel().Contains(FString(TEXT("water")));
				FTransform ActorTransform = Owner->GetActorTransform();

				// Paint mesh instance vertices in code. They will contain actual lighting.
				// https://forums.unrealengine.com/showthread.php?14851-Painting-Vertex-Colours-From-Code
				// https://github.com/alanedwardes/UE4VertexColorSpread/blob/master/Source/VertexColorSpread/VertexColorSpreadMesh.cpp

				if (MeshComp->LODData.Num() == 0)
				{
					MeshComp->SetLODDataCount(1, MeshComp->LODData.Num());
				}
				FStaticMeshLODResources& LODModel = MeshComp->GetStaticMesh()->RenderData->LODResources[0];
				FStaticMeshComponentLODInfo* InstanceMeshLODInfo = &MeshComp->LODData[0];
				if (InstanceMeshLODInfo->OverrideVertexColors == nullptr)
				{
					InstanceMeshLODInfo->OverrideVertexColors = new FColorVertexBuffer();
					InstanceMeshLODInfo->OverrideVertexColors->InitFromSingleColor(FColor::Black, LODModel.GetNumVertices());
					BeginInitResource(InstanceMeshLODInfo->OverrideVertexColors);
					MeshComp->MarkRenderStateDirty();
				}
				
				for (uint32 i = 0, len = InstanceMeshLODInfo->OverrideVertexColors->GetNumVertices(); i < len; ++i)
				{
					FVector Position = LODModel.PositionVertexBuffer.VertexPosition(i);
					Position = ActorTransform.TransformPosition(Position);

					FVector Normal = LODModel.VertexBuffer.VertexTangentZ(i);
					Normal = ActorTransform.TransformVector(Normal);

					// Calc lighting per vertex
					FLinearColor LinearColor = CalcLighting(Owner, World, bBase, Position, Normal, AmbientColor, Lights);
					FColor Color = LinearColor.ToFColor(false);

					InstanceMeshLODInfo->OverrideVertexColors->VertexColor(i) = Color;
				}
				BeginUpdateResourceRHI(InstanceMeshLODInfo->OverrideVertexColors);	
			}
		}
	}

	Level->MarkPackageDirty();
}

#else

void ULightingBuilder::BuildDOLighting(UWorld* World)
{
}

#endif