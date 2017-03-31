#include "PathfindingModule.h"
#include "SquareGrid.h"

USquareGrid::USquareGrid(const FObjectInitializer& ObjectInitializer)
{
	Width = 0;
	Height = 0;
}

int32 USquareGrid::GetWidth()
{
	return Width;
}

int32 USquareGrid::GetHeight()
{
	return Height;
}

int32 USquareGrid::GetCell(int32 X, int32 Y)
{
	return Grid[X + Y * Width];
}

int32 USquareGrid::CalcCellValue(TArray<int32>& RawGrid, int32 RawGridStride, int32 GridStep, int32 X, int32 Y)
{
	int32 RawX = X * GridStep;
	int32 RawY = Y * GridStep;

	int32 Result = 1;
	for (int32 IndexY = RawY, MaxY = RawY + GridStep; IndexY < MaxY; ++IndexY)
	{
		int32 Offset = IndexY * RawGridStride;
		for (int32 IndexX = RawX, MaxX = RawX + GridStep; IndexX < MaxX; ++IndexX)
		{
			int32 RawGridValue = RawGrid[IndexX + Offset];
			if (RawGridValue == 0)
			{
				// Found unwalkable node, so whole block is marked unwalkable
				Result = 0;

				// No need to check further
				IndexX = MaxX;
				IndexY = MaxY;
			}
		}
	}
	return Result;
}

void USquareGrid::Init(TArray<int32>& RawGrid, int32 RawGridStride, int32 GridStep)
{
	int32 RawGridWidth = RawGridStride;
	int32 RawGridHeight = RawGrid.Num() / RawGridWidth;

	int32 GridWidth = RawGridWidth / GridStep;
	int32 GridHeight = RawGridHeight / GridStep;
	int32 GridSize = GridWidth * GridHeight;

	Grid.Empty(GridSize);
	Grid.SetNumUninitialized(GridSize);

	Width = GridWidth;
	Height = GridHeight;

	for (int32 Y = 0; Y < GridHeight; ++Y)
	{
		int32 Offset = Y * GridWidth;
		for (int32 X = 0; X < GridWidth; ++X)
		{
			int32 Cell = CalcCellValue(RawGrid, RawGridStride, GridStep, X, Y);
			Grid[X + Offset] = Cell;
		}
	}
}

#define PATHFINDING_USE_DIAG

TArray<FGridIndex> USquareGrid::GetNeighbors(FGridIndex& Id)
{
	TArray<FGridIndex> Result;

	AddCell(Result, Id.X - 1, Id.Y    );
#ifdef PATHFINDING_USE_DIAG
	AddCell(Result, Id.X - 1, Id.Y + 1);
#endif
	AddCell(Result, Id.X    , Id.Y + 1);
#ifdef PATHFINDING_USE_DIAG
	AddCell(Result, Id.X + 1, Id.Y + 1);
#endif
	AddCell(Result, Id.X + 1, Id.Y    );
#ifdef PATHFINDING_USE_DIAG
	AddCell(Result, Id.X + 1, Id.Y - 1);
#endif
	AddCell(Result, Id.X    , Id.Y - 1);
#ifdef PATHFINDING_USE_DIAG
	AddCell(Result, Id.X - 1, Id.Y - 1);
#endif

	return Result;
}

float USquareGrid::GetCost(FGridIndex& From, FGridIndex& To)
{
	// TODO: use 10 or 14 (~sqrt(2)) as cost?
	float X = To.X - From.X;
	float Y = To.Y - From.Y;
	float Result = FMath::Sqrt(X * X + Y * Y);
	return Result;

	//TODO
	// add method for getting cost for neighbors? replace it with if check rather than math
}
