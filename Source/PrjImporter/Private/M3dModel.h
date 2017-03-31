#pragma once

#include "Core.h"

class M3dModel
{
public:
	struct Texture
	{
		FString			Path;
		FString			Name;
	};

	struct Vertex
	{
		FVector			Point;
		FVector			Normal;
		uint8			Rgba[4];
		FVector2D		Uv;
		int32			Spare1;
		int32			Spare2;
	};

	struct Face
	{
		int16			Vertex[3];
		int16			Texture;
		FVector			Normal;
		int32			Spare1;
		int32			Spare2;
	};

	struct Object
	{
		FString			Name;
		int16			Parent;
		FVector			Pivot;
		int16			NumVertices;
		int16			NumFaces;
		int32			Flags;
		int32			Spare1;
		int32			Spare2;
		TArray<Face>	Faces;
		TArray<Vertex>	Vertices;
	};

public:
	int32			ID;
	int32			Magic;
	int32			Version;
	int32			CRC;
	int32			NOT_CRC;
	int16			NumTextures;
	int16			NumObjects;

	TArray<Texture>	Textures;
	TArray<Object>	Objects;

	public:
		M3dModel();

	public:
		static bool LoadFromM3d(FString FilePath, M3dModel& OutModel);
	//	bool SaveToObj(std::string& filePath);
};