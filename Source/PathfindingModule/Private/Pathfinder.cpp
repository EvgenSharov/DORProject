#include "PathfindingModule.h"
#include "Pathfinder.h"
#include "ProceduralMeshComponent.h"
#include "LevelData.h"
#include "SquareGrid.h"

APathfinder::APathfinder(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	ProceduralMesh = nullptr;
	Step = 1;

	Grid = CreateDefaultSubobject<USquareGrid>(TEXT("Grid"));

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;
	Root->SetWorldLocation(FVector::ZeroVector);
	Root->SetMobility(EComponentMobility::Static);

#if WITH_EDITOR
	static ConstructorHelpers::FObjectFinder<UMaterial> GridMaterial(TEXT("Material'/Game/Common/M_PathGrid.M_PathGrid'"));
	ProceduralMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("GridMesh"), true);
	ProceduralMesh->SetMaterial(0, GridMaterial.Object);
	ProceduralMesh->SetupAttachment(RootComponent);
#endif
}

bool APathfinder::IsValid()
{
	return RegionDataReader != nullptr && Step > 0;
}

void APathfinder::BuildGrid()
{
	if (!IsValid())
	{
		return;
	}

	Grid->Init(RegionDataReader->Data, RegionDataReader->Stride, Step);
}

ALevelData* APathfinder::FindLevelData()
{
	for (TActorIterator<ALevelData> ActorItr(GetWorld()); ActorItr; ++ActorItr)
	{
		return *ActorItr;
	}
	return nullptr;
}

#if WITH_EDITOR
void APathfinder::BuildGridDebug()
{
	ProceduralMesh->ClearAllMeshSections();

	if (!bDrawGrid)
	{
		return;
	}

	ALevelData* LevelData = FindLevelData();
	if (LevelData == nullptr)
	{
		return;
	}
	
	TArray<FVector> Vertices;
	TArray<int32> Indices;
	TArray<FVector> Normals;
	TArray<FVector2D> UV0;
	GridColors.Empty();
	TArray<FProcMeshTangent> Tangents;

	int32 GridWidth = Grid->GetWidth();
	int32 GridHeight = Grid->GetHeight();
	int32 GridSize = GridWidth * GridHeight;

	int32 VertexNum = GridSize * 4;
	int32 IndexNum = GridSize * 6;

	Vertices.SetNumUninitialized(VertexNum);
	Indices.SetNumUninitialized(IndexNum);
	Normals.Init(FVector::UpVector, VertexNum);
	UV0.SetNumUninitialized(VertexNum);
	GridColors.SetNumUninitialized(VertexNum);
	Tangents.Init(FProcMeshTangent(1, 1, 1), VertexNum);

	int32 RegionX, RegionY, Height0, Height1, Height2, Height3, V0Index, V1Index, V2Index, V3Index, Index, IndexOffset;

	auto GetHeight = [&](int32 X, int32 Y)
	{
		FVector Point;
		LevelData->ProjectToTerrainBase(-X, Y, Point);
		return Point.Z;
	};

	for (int32 IndexY = 0; IndexY < GridHeight; ++IndexY)
	{
		for (int32 IndexX = 0; IndexX < GridWidth; ++IndexX)
		{
			RegionX = IndexX * Step;
			RegionY = IndexY * Step;

			Height0 = GetHeight(RegionX       , RegionY       );
			Height1 = GetHeight(RegionX + Step, RegionY       );
			Height2 = GetHeight(RegionX + Step, RegionY + Step);
			Height3 = GetHeight(RegionX       , RegionY + Step);

			FVector Position(-RegionX, RegionY, Height0);
			uint8 Item = (uint8)Grid->GetCell(IndexX, IndexY);

			FColor Color = Item == 0 ? FColor::Black : FColor::White;

			Index = IndexX + IndexY * GridWidth;

			V0Index = Index * 4;
			V1Index = V0Index + 1;
			V2Index = V1Index + 1;
			V3Index = V2Index + 1;

			Vertices[V0Index] = Position;
			Vertices[V1Index] = FVector(Position.X - Step, Position.Y       , Height1);
			Vertices[V2Index] = FVector(Position.X - Step, Position.Y + Step, Height2);
			Vertices[V3Index] = FVector(Position.X       , Position.Y + Step, Height3);

			IndexOffset = Index * 6;
			Indices[IndexOffset    ] = V0Index;
			Indices[IndexOffset + 1] = V1Index;
			Indices[IndexOffset + 2] = V2Index;
			Indices[IndexOffset + 3] = V0Index;
			Indices[IndexOffset + 4] = V2Index;
			Indices[IndexOffset + 5] = V3Index;

			GridColors[V0Index] = Color;
			GridColors[V1Index] = Color;
			GridColors[V2Index] = Color;
			GridColors[V3Index] = Color;

			UV0[V0Index] = FVector2D(0, 0);
			UV0[V1Index] = FVector2D(0, 1);
			UV0[V2Index] = FVector2D(1, 1);
			UV0[V3Index] = FVector2D(1, 0);
		}
	}

	ProceduralMesh->CreateMeshSection(0, Vertices, Indices, Normals, UV0, GridColors, Tangents, false);
}

void APathfinder::ApplyGridColors()
{
	TArray<FVector> Vertices;
	TArray<FVector> Normals;
	TArray<FVector2D> UV0;
	TArray<FProcMeshTangent> Tangents;

	TArray<FColor> Colors(GridColors);
	for (auto& Pair : PathfindingColors)
	{
		FColor& Color = Pair.Value;
		int32 ArrayIndex = Pair.Key * 4;

		// Update quad colors
		Colors[ArrayIndex    ] = Color;
		Colors[ArrayIndex + 1] = Color;
		Colors[ArrayIndex + 2] = Color;
		Colors[ArrayIndex + 3] = Color;
	}

	ProceduralMesh->UpdateMeshSection(0, Vertices, Normals, UV0, Colors, Tangents);
}
#endif

void APathfinder::OnConstruction(const FTransform& Transform)
{
#if WITH_EDITOR
	ProceduralMesh->bVisible = bDrawGrid;
	ProceduralMesh->SetRelativeLocation(DrawOffset);
#endif
}

void APathfinder::Tick(float DeltaSeconds)
{
	//TODO
}

#if WITH_EDITOR
void APathfinder::PostEditChangeProperty(struct FPropertyChangedEvent& E)
{
	FName PropertyName = (E.Property != NULL) ? E.MemberProperty->GetFName() : NAME_None;

	if (PropertyName == GET_MEMBER_NAME_CHECKED(APathfinder, bDrawGrid) || 
		PropertyName == GET_MEMBER_NAME_CHECKED(APathfinder, bClickToRebuild))
	{
		BuildGridDebug();
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(APathfinder, RegionDataReader) ||
		PropertyName == GET_MEMBER_NAME_CHECKED(APathfinder, Step))
	{
		BuildGrid();
		BuildGridDebug();
	}

	Super::PostEditChangeProperty(E);
}
#endif

void APathfinder::WorldToGrid(FVector& Point, int32& Out_X, int32& Out_Y)
{
	Out_X = (int32)(-Point.X / Step);
	Out_Y = (int32)( Point.Y / Step);
}

inline float Heuristic(FGridIndex& A, FGridIndex& B)
{
	return FMath::Abs(A.X - B.X) + FMath::Abs(A.Y - B.Y);
}

void APathfinder::FindPath(FVector From, FVector To, TArray<FVector2D>& Out_Path, bool& Out_bResult)
{
	// http://www.redblobgames.com/pathfinding/a-star/implementation.html

	struct IndexWithPriority
	{
		FGridIndex Index;
		float Priority;

		IndexWithPriority()
			: Index(), Priority(0.0f)
		{
		}

		IndexWithPriority(FGridIndex& InIndex, float InPriority)
			: Index(InIndex), Priority(InPriority)
		{
		}

		IndexWithPriority(const IndexWithPriority& Other)
			: Index(Other.Index), Priority(Other.Priority)
		{
		}
	};
	struct QueuePredicate
	{
		bool operator()(const IndexWithPriority& A, const IndexWithPriority& B) const
		{
			return A.Priority < B.Priority;
		}
	};
	QueuePredicate Predicate;

	Out_bResult = false;

	FGridIndex FromIndex;
	WorldToGrid(From, FromIndex.X, FromIndex.Y);
	
	FGridIndex ToIndex;
	WorldToGrid(To, ToIndex.X, ToIndex.Y);

#if WITH_EDITOR
	if (bVisualizePathfinding)
	{
		PathfindingColors.Empty();
	}
#endif

	if (!Grid->IsInBounds(FromIndex.X, FromIndex.Y) || !Grid->IsInBounds(ToIndex.X, ToIndex.Y))
	{
		UE_LOG(LogTemp, Warning, TEXT("APathfinder::FindPath() From or To are not in the map bounds"));
		return;
	}

	if (Grid->GetCell(FromIndex.X, FromIndex.Y) == 0 || Grid->GetCell(ToIndex.X, ToIndex.Y) == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("APathfinder::FindPath() From or To are in the unwalkable area"));
		return;
	}

	FGridIndex& Start = ToIndex;
	FGridIndex& Goal = FromIndex;

	TArray<IndexWithPriority> Frontier;
	Frontier.HeapPush(IndexWithPriority(Start, 0), Predicate);

	TMap<FGridIndex, FGridIndex> CameFrom;
	CameFrom.Add(Start, Start);

	TMap<FGridIndex, float> CostSoFar;
	CostSoFar.Add(Start, 0);
	
	IndexWithPriority Current;
	float CurrentCost, NewCost, Priority;

#if WITH_EDITOR
	if (bVisualizePathfinding)
	{
		//PathfindingColors.Add(Grid->FlattenIndex(FromIndex), FColor::Yellow);
	}
#endif
	
	while (Frontier.Num() != 0)
	{
		Frontier.HeapPop(Current, Predicate, false);
		FGridIndex& CurrentIndex = Current.Index;

		if (CurrentIndex == Goal)
		{
			break;
		}

		CurrentCost = CostSoFar[CurrentIndex];
		TArray<FGridIndex> Neighbors = Grid->GetNeighbors(CurrentIndex);

		for (int32 NeighborIndex = 0, Length = Neighbors.Num(); NeighborIndex < Length; ++NeighborIndex)
		{
			FGridIndex& NextIndex = Neighbors[NeighborIndex];
			NewCost = CurrentCost + Grid->GetCost(CurrentIndex, NextIndex);

			if (!CostSoFar.Contains(NextIndex))// || NewCost < CostSoFar[NextIndex])
			{
				CostSoFar.Add(NextIndex, NewCost);
				CameFrom.Add(NextIndex, CurrentIndex);

				Priority = NewCost + Heuristic(NextIndex, Goal);
				Frontier.HeapPush(IndexWithPriority(NextIndex, Priority), Predicate);

#if WITH_EDITOR
				if (bVisualizePathfinding)
				{
					PathfindingColors.Add(Grid->FlattenIndex(NextIndex), FColor::Yellow);
				}
#endif
			}
		}
	}

	FGridIndex Curr = Goal;
	while (!(Curr == Start))
	{
		Curr = CameFrom[Curr];

		//TODO construct the actual path

#if WITH_EDITOR
		if (bVisualizePathfinding)
		{
			//PathfindingColors[Grid->FlattenIndex(Curr)] = FColor::Green;
			PathfindingColors.Add(Grid->FlattenIndex(Curr), FColor::Purple);
		}
#endif
	}

#if WITH_EDITOR
	if (bVisualizePathfinding)
	{
		//PathfindingColors[Grid->FlattenIndex(FromIndex)] = FColor::Blue;
		//PathfindingColors[Grid->FlattenIndex(ToIndex)] = FColor::Red;
		PathfindingColors.Add(Grid->FlattenIndex(FromIndex), FColor::Blue);
		PathfindingColors.Add(Grid->FlattenIndex(ToIndex), FColor::Red);
		ApplyGridColors();
	}
#endif

	Out_bResult = true;
}