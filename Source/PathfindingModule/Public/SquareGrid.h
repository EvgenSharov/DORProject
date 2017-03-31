#pragma once

#include "Engine.h"
#include "SquareGrid.generated.h"

struct PATHFINDINGMODULE_API FGridIndex
{
	int32 X;
	int32 Y;

	//TODO add move constructor

	FGridIndex()
		: X(0), Y(0)
	{
	}

	FGridIndex(int32 CoordX, int32 CoordY)
		: X(CoordX), Y(CoordY)
	{
	}

	FGridIndex(const FGridIndex& Other)
		: X(Other.X), Y(Other.Y)
	{
	}

	friend bool operator==(const FGridIndex& First, const FGridIndex& Second)
	{
		return (First.X == Second.X) && (First.Y == Second.Y);
	}

	friend uint32 GetTypeHash(const FGridIndex& Other)
	{
		return GetTypeHash(Other.X) ^ GetTypeHash(Other.Y);
	}
};

UCLASS()
class PATHFINDINGMODULE_API USquareGrid : public UObject
{
	GENERATED_BODY()

private:
	UPROPERTY()
	TArray<int32> Grid;
	
	UPROPERTY()
	int32 Width;

	UPROPERTY()
	int32 Height;

public:
	int32 GetWidth();
	int32 GetHeight();
	int32 GetCell(int32 X, int32 Y);

public:
	USquareGrid(const FObjectInitializer& ObjectInitializer);

private:
	int32 CalcCellValue(TArray<int32>& RawGrid, int32 RawGridStride, int32 GridStep, int32 X, int32 Y);
	inline void AddCell(TArray<FGridIndex>& Array, int32 X, int32 Y);

public:
	void Init(TArray<int32>& RawGrid, int32 RawGridStride, int32 GridStep);
	TArray<FGridIndex> GetNeighbors(FGridIndex& Id);
	float GetCost(FGridIndex& From, FGridIndex& To);

	inline int32 FlattenIndex(FGridIndex& Index);
	inline bool IsInBounds(int32 X, int32 Y);
};

int32 USquareGrid::FlattenIndex(FGridIndex& Index)
{
	return Index.X + Index.Y * Width;
}

bool USquareGrid::IsInBounds(int32 X, int32 Y)
{
	return X >= 0 && X < Width && Y > 0 && Y < Height;
}

void USquareGrid::AddCell(TArray<FGridIndex>& Array, int32 X, int32 Y)
{
	if (!IsInBounds(X, Y) || Grid[X + Y * Width] == 0)
	{
		return;
	}

	FGridIndex Index(X, Y);
	Array.Add(Index);
}
