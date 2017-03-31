#pragma once

#include "Engine.h"
#include "GridDataReader.generated.h"

UCLASS(Abstract, Blueprintable)
class PATHFINDINGMODULE_API AGridDataReader : public AActor
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, Category = DataReader)
	TArray<int32> Data;

	UPROPERTY(BlueprintReadOnly, Category = DataReader)
	int32 Stride;
};
