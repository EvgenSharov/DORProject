#include "PrjImporter.h"
#include "PrjFile.h"

PrjFile::PrjFile()
{
}

void PrjFile::ReadFile(TArray<uint8> Data, uint32& FilePos, void* Dest, uint32 Size)
{
	FMemory::Memcpy(Dest, &Data[FilePos], Size);
	FilePos += Size;
}

bool PrjFile::LoadHeightMapData(TArray<uint8> DataBinary, uint32& FilePos, bool bShdFile, HeightMapData& OutHeightMapData)
{
	if (bShdFile)
	{
		// Skip header and size
		int64 Temp;
		ReadFile(DataBinary, FilePos, &Temp, sizeof(Temp));
	}

	int32 Width;
	ReadFile(DataBinary, FilePos, &Width, sizeof(Width));
	OutHeightMapData.Width = Width;

	int32 Height;
	ReadFile(DataBinary, FilePos, &Height, sizeof(Height));
	OutHeightMapData.Height = Height;

	int32 MacroBlockCount;
	ReadFile(DataBinary, FilePos, &MacroBlockCount, sizeof(MacroBlockCount));
	OutHeightMapData.MacroBlockCount = MacroBlockCount;

	int32 DataPairCount;
	ReadFile(DataBinary, FilePos, &DataPairCount, sizeof(DataPairCount));
	OutHeightMapData.DataPairCount = DataPairCount;

	int32 MapBlockSize;
	ReadFile(DataBinary, FilePos, &MapBlockSize, sizeof(MapBlockSize));

	if (bShdFile)
	{
		auto& HeightMap1 = OutHeightMapData.HeightMap1;
		HeightMap1.SetNumUninitialized(DataPairCount);
		ReadFile(DataBinary, FilePos, &HeightMap1[0], MapBlockSize);
	}
	else
	{
		auto& HeightMap1 = OutHeightMapData.HeightMap1;
		HeightMap1.SetNumUninitialized(DataPairCount);
		ReadFile(DataBinary, FilePos, &HeightMap1[0], MapBlockSize / 2);

		auto& HeightMap2 = OutHeightMapData.HeightMap2;
		HeightMap2.SetNumUninitialized(DataPairCount);
		ReadFile(DataBinary, FilePos, &HeightMap2[0], MapBlockSize / 2);
	}

	int32 MacroBlockSize;
	ReadFile(DataBinary, FilePos, &MacroBlockSize, sizeof(MacroBlockSize));

	auto& MacroBlockData = OutHeightMapData.MacroBlockData;
	MacroBlockData.SetNumUninitialized(MacroBlockSize);
	ReadFile(DataBinary, FilePos, &MacroBlockData[0], MacroBlockSize);

	return true;
}

bool PrjFile::LoadFromPrj(FString FilePath, PrjFile& OutPrjFile)
{
	TArray<uint8> DataBinary;
	if (!FFileHelper::LoadFileToArray(DataBinary, *FilePath))
	{
		return false;
	}
	uint32 FilePos = 0;
	
	char Header[32];
	ReadFile(DataBinary, FilePos, Header, sizeof(Header));
	Header[31] = '\0';

	OutPrjFile.Header = FString(Header);

	char BlockId[5];
	BlockId[4] = '\0';
	int32 BlockSize;
	char Temp[256];

	while (true)
	{
		ReadFile(DataBinary, FilePos, BlockId, 4);
		ReadFile(DataBinary, FilePos, &BlockSize, 4);

		FString BlockIdString(BlockId);
		if (BlockIdString == TEXT("BASE"))
		{
			ReadFile(DataBinary, FilePos, Temp, BlockSize);
			FString BaseFileName(Temp);
			OutPrjFile.BaseFileName = BaseFileName;
		}
		else if (BlockIdString == TEXT("WATR"))
		{
			ReadFile(DataBinary, FilePos, Temp, BlockSize);
			FString WaterFileName(Temp);
			OutPrjFile.WaterFileName = WaterFileName;
		}
		else if (BlockIdString == TEXT("FURN"))
		{
			int32 FurnCount;
			ReadFile(DataBinary, FilePos, &FurnCount, sizeof(FurnCount));

			int32 FurnNameCount;
			for (int FurnIndex = 0; FurnIndex < FurnCount; ++FurnIndex)
			{
				ReadFile(DataBinary, FilePos, &FurnNameCount, sizeof(FurnNameCount));
				ReadFile(DataBinary, FilePos, Temp, FurnNameCount);
				FString FurnFileName(Temp);
				OutPrjFile.FurnFileNames.Add(FurnFileName);
			}
		}
		else if (BlockIdString == TEXT("INST"))
		{
			int32 InstCount;
			ReadFile(DataBinary, FilePos, &InstCount, sizeof(InstCount));

			int32 InstSize;
			ReadFile(DataBinary, FilePos, &InstSize, sizeof(InstSize));

			// A Furniture instance slot
			typedef int32 S32;
			struct FURNITUREINSTANCE
			{
				S32		Prev;
				S32		Next;
				S32		Selected;
				S32		ExcludeFromTerrain;
				S32		X;
				S32		Y;
				S32		Z;
				S32		RotX;
				S32		RotY;
				S32		RotZ;
				S32		MinX; // mesh max/mins - stored here !
				S32		MinY;
				S32		MinZ;
				S32		MaxX;
				S32		MaxY;
				S32		MaxZ;
				S32		MeshSlot;	// used to store a mesh slot number in the SAVED project
				S32		Mesh;
				//FURNITUREMESH *Mesh;

				S32		MakeAttackable;
				S32		Toughness;		// used if the furniture is to be attackable
				S32		Wounds;
				S32		Pad;
				S32		OwnerUnitIndex;
				S32		Burns;
				S32		SFXCode;
				S32		GFXCode;
				S32		Locked;				// cannot be dragged while locked
				S32		ExcludeFromTerrainShadow;
				S32		ExcludeFromWalk;
				S32		MagicItemCode;
				S32		ParticleEffectCode;
				S32		DeadMeshSlot;	// used to store a mesh slot number in the SAVED project
				S32		DeadMesh;
				//RNITUREMESH *DeadMesh;
				S32		Light;
				S32		LightRadius;
				S32		LightAmbient;

				S32		Pad15;
				S32		Pad16; 	// future proofing

			};

			for (int32 InstIndex = 0; InstIndex < InstCount; ++InstIndex)
			{
				FURNITUREINSTANCE FurnInst;
				ReadFile(DataBinary, FilePos, &FurnInst, InstSize);

				FurnitureInstance Instance;
				Instance.MeshSlot = FurnInst.MeshSlot;
				float TwoPi = 3.14159f * 2.0f;
				Instance.Position = FVector(FurnInst.X / -1024.f, FurnInst.Z / 1024.f, FurnInst.Y / 1024.f);
				FVector EulerAnglesDeg = FVector(-FMath::RadiansToDegrees(FurnInst.RotX / 4096.f * TwoPi), -FMath::RadiansToDegrees(FurnInst.RotZ / 4096.f * TwoPi), -FMath::RadiansToDegrees(FurnInst.RotY / 4096.f * TwoPi));
				Instance.Rotation = FRotator::MakeFromEuler(EulerAnglesDeg);

				//TODO add all other required fields

				OutPrjFile.FurnitureInstances.Add(Instance);
			}
		}
		else if (BlockIdString == TEXT("TERR"))
		{
			if (!LoadHeightMapData(DataBinary, FilePos, false, OutPrjFile.TerrainData))
			{
				UE_LOG(PrjImporter, Warning, TEXT("TERR data failed to parse"));
				return false;
			}
		}
		else if (BlockIdString == TEXT("ATTR"))
		{
			int32 Width;
			ReadFile(DataBinary, FilePos, &Width, sizeof(Width));

			int32 Height;
			ReadFile(DataBinary, FilePos, &Height, sizeof(Height));

			int32 ArraySize = Width * Height;
			int32 FileArraySize = ArraySize / 2;

			TArray<uint8> Array;
			Array.SetNumUninitialized(FileArraySize);
			ReadFile(DataBinary, FilePos, &Array[0], FileArraySize);

			OutPrjFile.AttrBlockData.Map.SetNumUninitialized(ArraySize);
			for (int32 Index = 0; Index < FileArraySize; ++Index)
			{
				uint8 Item = Array[Index];
				uint8 Value0 = (uint8)(Item & 0xF);
				uint8 Value1 = (uint8)(Item >> 4);

				OutPrjFile.AttrBlockData.Map[Index * 2] = Value1;
				OutPrjFile.AttrBlockData.Map[Index * 2 + 1] = Value0;
			}

			ReadFile(DataBinary, FilePos, OutPrjFile.AttrBlockData.Footer, 64);
			OutPrjFile.AttrBlockData.Width = Width;
			OutPrjFile.AttrBlockData.Height = Height;
		}
		else
		{
			//TODO
			break;
		}
	}

	// Load SHD file if present
	FString FolderPath = FPaths::GetPath(FilePath);
	IFileManager& FileManager = IFileManager::Get();
	TArray<FString> FoundFiles;
	FileManager.FindFiles(FoundFiles, *FolderPath, TEXT("SHD"));
	if (FoundFiles.Num() > 0)
	{
		FString ShdFilePath = FolderPath / FoundFiles[0];
		TArray<uint8> ShdDataBinary;
		if (!FFileHelper::LoadFileToArray(ShdDataBinary, *ShdFilePath))
		{
			UE_LOG(PrjImporter, Warning, TEXT("%s SHD file failed to load"), *ShdFilePath);
		}
		else
		{
			uint32 ShdFilePos = 0;
			if (!LoadHeightMapData(ShdDataBinary, ShdFilePos, true, OutPrjFile.ShadowData))
			{
				UE_LOG(PrjImporter, Warning, TEXT("SHD data failed to parse"));
			}
		}
	}

	return true;
}

bool PrjFile::FlattenHeightMapData(HeightMapData& Source, int32 HeightMapIndex, HeightMapDataFlattened& OutDest)
{
	int32 BlocksX = Source.Width / 8;
	int32 Width = Source.Width;
	int32 Height = Source.Height;

	auto& HeightMap = HeightMapIndex == 1 ? Source.HeightMap1 : Source.HeightMap2;

	OutDest.Width = Width;
	OutDest.Height = Height;

	for (int32 Y = 0; Y < Height; ++Y)
	{
		for (int32 X = 0; X < Width; ++X)
		{
			int32 Index = X / 8 + Y / 8 * BlocksX;
			int32 IndexOffset = X % 8 + (Y % 8) * 8;

			auto& Pair = HeightMap[Index];
			int32 MacroBlockIndex = Pair.Value;
			
			int32 BaseHeight = Pair.Key / 1024;
			uint8 HeightOffset = Source.MacroBlockData[MacroBlockIndex + IndexOffset] / 8;

			int32 ResultingHeight = BaseHeight + HeightOffset;

			OutDest.Heightmap.Add(ResultingHeight);
		}
	}

	return true;
}
