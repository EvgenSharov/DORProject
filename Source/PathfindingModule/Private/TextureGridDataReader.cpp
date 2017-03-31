#include "PathfindingModule.h"
#include "TextureGridDataReader.h"

ATextureGridDataReader::ATextureGridDataReader(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void ATextureGridDataReader::OnConstruction(const FTransform& Transform)
{
	ReadData();
}

void ATextureGridDataReader::ReadData()
{
	// https://answers.unrealengine.com/questions/25594/accessing-pixel-values-of-texture2d.html

	if (Texture == nullptr)
	{
		return;
	}

	FTexture2DMipMap* MipMap = &Texture->PlatformData->Mips[0];
	FByteBulkData* RawImageData = &MipMap->BulkData;
	//FColor* FormatedImageData = static_cast<FColor*>(RawImageData->Lock(LOCK_READ_ONLY));
	uint8* FormatedImageData = static_cast<uint8*>(RawImageData->Lock(LOCK_READ_ONLY));

	uint32 TextureWidth = MipMap->SizeX;
	uint32 TextureHeight = MipMap->SizeY;
	uint32 Length = TextureWidth * TextureHeight;

	Data.Empty(Length);
	Data.SetNumUninitialized(Length);
	Stride = TextureWidth;

	for (uint32 Y = 0; Y < TextureHeight; ++Y)
	{
		for (uint32 X = 0; X < TextureWidth; ++X)
		{
			//FColor Pixel = FormatedImageData[X + (TextureHeight - 1 - Y) * TextureWidth];
			//Data[X + TextureWidth * Y] = Pixel.R;

			uint8 Pixel = FormatedImageData[X + (TextureHeight - 1 - Y) * TextureWidth];
			Data[X + TextureWidth * Y] = Pixel;
		}
	}

	RawImageData->Unlock();
}
