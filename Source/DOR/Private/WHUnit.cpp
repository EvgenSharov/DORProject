#include "DOR.h"
#include "WHUnit.h"
#include "WHModel.h"
#include "LevelData.h"

AWHUnit::AWHUnit()
{
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	PrimaryActorTick.bCanEverTick = true;

	_modelCount = 1;
	_lineCount = 1;
}

void AWHUnit::SetGoal(EUnitGoals goal)
{
	if (_currentGoal != goal)
	{
		_currentGoal = goal;

		switch (goal)
		{
			case EUnitGoals::Idle:
			{
				for (auto model : _models)
				{
					model->SetGoal(EModelGoals::Idle);
				}
			}
			break;

			case EUnitGoals::Moving:
			{
				for (auto model : _models)
				{
					model->SetGoal(EModelGoals::Move);
				}
			}
			break;
		}
	}
}

void AWHUnit::ProjectUnitToTerrain()
{
	if (_levelData)
	{
		FVector location = GetActorLocation();
		_levelData->ProjectToTerrainBase(location.X, location.Y, location);
		SetActorLocation(location);

		FTransform transform = GetTransform();
		for (int32 index = 0, len = _models.Num(); index < len; ++index)
		{
			AWHModel* model = _models[index];
			
			FVector2D& offset = _offsets[index];
			location = FVector(offset.X, offset.Y, 0.0f);
			location = transform.TransformPosition(location);

			_levelData->ProjectToTerrainBase(location.X, location.Y, location);

			model->SetActorLocation(location);
			model->SetActorRotation(transform.Rotator());
		}
	}
}

void AWHUnit::OnConstruction(const FTransform& transform)
{
	Super::OnConstruction(transform);

#if WITH_EDITOR
	TArray<FVector2D> offsets;
	offsets.SetNum(_modelCount);
	CalcFormation(offsets, _modelCount);

	if (_debugFormation)
	{
		for (USphereComponent* sphere : _formationSpheres)
		{
			if (sphere)
			{
				sphere->DestroyComponent();
			}
		}
		_formationSpheres.Empty();

		_formationSpheres.SetNum(_modelCount);
		for (int32 index = 0; index < _modelCount; ++index)
		{
			USphereComponent* sphere = NewObject<USphereComponent>(this);
			sphere->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
			sphere->RegisterComponent();
			sphere->CreationMethod = EComponentCreationMethod::UserConstructionScript;
			sphere->SetSimulatePhysics(false);
			sphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			sphere->InitSphereRadius(_debugSphereSize);
			sphere->bHiddenInGame = false;
			
			_formationSpheres[index] = sphere;
		}

		ShowDebugFormation(offsets, _modelCount);
	}
#endif
}

void AWHUnit::BeginPlay()
{
	Super::BeginPlay();

	// Find level data
	_levelData = nullptr;
	for (TActorIterator<ALevelData> iter(GetWorld()); iter; ++iter)
	{
		_levelData = *iter;
		break;
	}
	if (_levelData == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("AWHUnit::BeginPlay() Level data is null"));
	}

	// Spawn unit models
	UWorld* world = GetWorld();
	_models.SetNum(_modelCount);
	for (int32 index = 0; index < _modelCount; ++index)
	{
		AWHModel* model = world->SpawnActor<AWHModel>(_modelClass);
		model->SetParent(this);
		_models[index] = model;
	}

	// Get unit characteristics using its model (we always have at least 1 model in the unit)
	_moveSpeed = _models[0]->_moveSpeed;
	_chargeSpeed = _models[0]->_chargeSpeed;
	_rotationSpeed = _models[0]->_rotationSpeed;

	// Starting formation
	_offsets.SetNum(_modelCount);
	CalcFormation(_offsets, _modelCount);

	ProjectUnitToTerrain();
	SetGoal(EUnitGoals::Idle);
}

void AWHUnit::Tick(float deltaTime)
{
	Super::Tick(deltaTime);

	switch (_currentGoal)
	{
		default:
		case EUnitGoals::None:
		case EUnitGoals::Idle:
		{
			// Doing nothing
		}
		break;
		
		case EUnitGoals::Moving:
		{
			FVector location = GetActorLocation();
			FVector direction = _targetPoint - location;
			
		  	float directionLength = direction.Size();
			if (directionLength < 1e-4f)
			{
				// Finished moving
				bool isMoving = false;
				for (auto model : _models)
				{
					if (model->GetIsMoving())
					{
						isMoving = true;
						break;
					}
				}
				if (!isMoving)
				{
					SetGoal(EUnitGoals::Idle);
				}
			}
			else
			{
				float targetDistance = deltaTime * _moveSpeed;
				targetDistance = FMath::Min(targetDistance, directionLength);
				FVector deltaMovement = direction * (targetDistance / directionLength);

				location += deltaMovement;

				SetActorLocation(location);
				
				//ProjectUnitToTerrain();
				if (_levelData)
				{
					location = GetActorLocation();
					_levelData->ProjectToTerrainBase(location.X, location.Y, location);
					SetActorLocation(location);

					FTransform transform = GetTransform();
					FRotator rotation = transform.Rotator();
					for (int32 index = 0, len = _models.Num(); index < len; ++index)
					{
						AWHModel* model = _models[index];

						FVector2D& offset = _offsets[index];
						location = FVector(offset.X, offset.Y, 0.0f);
						location = transform.TransformPosition(location);

						_levelData->ProjectToTerrainBase(location.X, location.Y, location);

						model->SetTarget(location, rotation);
					}
				}
			}
		}
		break;
	}
}

void AWHUnit::CalcFormation(TArray<FVector2D>& offsets, int32 modelCount)
{
	//TODO add proper formation code
	float lineShift = _lineCount / 2;
	for (int32 index = 0; index < modelCount; ++index)
	{
		int32 rank = index / _lineCount;
		int32 line = index % _lineCount;
		FVector2D offset(-rank * _modelPadding.Y, line * _modelPadding.X - lineShift);
		offsets[index] = offset;
	}
}

#if WITH_EDITOR
void AWHUnit::ShowDebugFormation(TArray<FVector2D>& offsets, int32 modelCount)
{
	int32 index;
	for (index = 0; index < modelCount; ++index)
	{
		USphereComponent* sphere = _formationSpheres[index];
		FVector2D& offset = offsets[index];
		sphere->SetRelativeLocation(FVector(offset.X, offset.Y, 0));
		sphere->bVisible = true;
	}
	for (int32 len = _formationSpheres.Num(); index < len; ++index)
	{
		_formationSpheres[index]->bVisible = false;
	}
}
#endif

void AWHUnit::MoveTo(FVector location)
{
	//TODO simple code for testing purposes, later pathfinding should be invoked here

	_targetPoint = location;
	FVector direction = location - GetActorLocation();
	direction.Z = 0;
	FRotator rotation = direction.Rotation();
	SetActorRotation(rotation);

	SetGoal(EUnitGoals::Moving);
}
