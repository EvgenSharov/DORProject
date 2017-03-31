#include "PrjImporter.h"
#include "LitFile.h"

void LitFile::ReadFile(TArray<uint8> Data, uint32& FilePos, void* Dest, uint32 Size)
{
	FMemory::Memcpy(Dest, &Data[FilePos], Size);
	FilePos += Size;
}

bool LitFile::LoadFromLit(FString FilePath, TArray<FLitLight>& OutLights)
{
	TArray<uint8> DataBinary;
	if (!FFileHelper::LoadFileToArray(DataBinary, *FilePath))
	{
		return false;
	}
	uint32 FilePos = 0;

	int32 FileVersion;
	ReadFile(DataBinary, FilePos, &FileVersion, sizeof(FileVersion));

	int32 LightCount;
	ReadFile(DataBinary, FilePos, &LightCount, sizeof(LightCount));

	OutLights.Empty(LightCount);

	struct FLitNative
	{
		int32 PosX, PosY, PosZ;
		uint32 Flags;
		int32 Attenuation;
		int32 ColX, ColY, ColZ;
	};

	for (int32 Index = 0; Index < LightCount; ++Index)
	{
		FLitNative Lit;
		ReadFile(DataBinary, FilePos, &Lit, sizeof(Lit));

		FLitLight LitLight;
		LitLight.Position = FVector(-Lit.PosX / 1024.0f, Lit.PosZ / 1024.0f, Lit.PosY / 1024.0f);
		LitLight.Color = FLinearColor(Lit.ColX / 256.0f, Lit.ColY / 256.0f, Lit.ColZ / 256.0f);
		LitLight.Attenuation = Lit.Attenuation / 1024.0f;
		LitLight.bShadow = (Lit.Flags & 1) != 0;
		LitLight.bLight = (Lit.Flags & 2) != 0;
		LitLight.bDirectional = (Lit.Flags & 4) != 0; // Point light is considered as TruePoint
		LitLight.bFurn = (Lit.Flags & 16) != 0;
		LitLight.bBase = (Lit.Flags & 32) != 0;

		OutLights.Add(LitLight);
	}

	return true;
}