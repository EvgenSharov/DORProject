#pragma once

#include "Core.h"

struct FLitLight
{
	bool bDirectional;
	FVector Position;
	FLinearColor Color;
	bool bLight;
	bool bShadow;
	bool bBase;
	bool bFurn;
	float Attenuation;
};

class LitFile
{
private:
	static void ReadFile(TArray<uint8> Data, uint32& FilePos, void* Dest, uint32 Size);

public:
	static bool LoadFromLit(FString FilePath, TArray<FLitLight>& OutLights);
};
