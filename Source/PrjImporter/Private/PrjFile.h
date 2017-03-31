#pragma once

#include "Core.h"

class PrjFile
{
public:
	struct FurnitureInstance
	{
		FVector Position;
		FRotator Rotation;
		int32 MeshSlot;
		bool bAttackable;
		int32 Toughness;
		bool bBurns;
		int32 SFXCode;
		int32 GFXCode;
		int32 ExcludeFromTerrainShadow;
		int32 ExcludeFromWalk;
		int32 MagicItemCode;
		int32 ParticleEffectCode;
		int32 DeadMeshSlot;
	};


	struct HeightMapData
	{
		int32 Width;
		int32 Height;
		int32 DataPairCount;
		TArray<TPair<int32, int32>> HeightMap1;
		TArray<TPair<int32, int32>> HeightMap2;
		int32 MacroBlockCount;
		TArray<uint8> MacroBlockData;
	};

	struct HeightMapDataFlattened
	{
		int32 Width;
		int32 Height;
		TArray<int32> Heightmap;
	};

	struct AttrData
	{
		int32 Width;
		int32 Height;
		TArray<uint8> Map;
		uint8 Footer[64];
	};

public:
	FString Header;
	FString BaseFileName;
	FString WaterFileName;
	TArray<FString> FurnFileNames;
	TArray<FurnitureInstance> FurnitureInstances;
	HeightMapData TerrainData;
	HeightMapData ShadowData;
	AttrData AttrBlockData;

public:
	PrjFile();

public:
	static bool LoadFromPrj(FString FilePath, PrjFile& OutPrjFile);

	static bool FlattenHeightMapData(HeightMapData& Source, int32 HeightMapIndex, HeightMapDataFlattened& OutDest);

private:
	static void ReadFile(TArray<uint8> Data, uint32& FilePos, void* Dest, uint32 Size);
	static bool LoadHeightMapData(TArray<uint8> DataBinary, uint32& FilePos, bool bShdFile, HeightMapData& OutHeightMapData);
};