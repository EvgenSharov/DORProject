#pragma once

#include "GridDataReader.h"
#include "TextureGridDataReader.generated.h"

UCLASS()
class PATHFINDINGMODULE_API ATextureGridDataReader : public AGridDataReader
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = DataReader)
	UTexture2D* Texture;

public:
	ATextureGridDataReader(const FObjectInitializer& ObjectInitializer);

public:
	virtual void OnConstruction(const FTransform& Transform) override;

private:
	void ReadData();
};
