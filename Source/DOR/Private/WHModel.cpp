#include "DOR.h"
#include "WHModel.h"
#include "LevelData.h"

FName AWHModel::WalkName("Walk");
FName AWHModel::ChargeName("Charge");
FName AWHModel::AttackName("Attack");
FName AWHModel::DeathName("Death");

AWHModel::AWHModel()
{
	PrimaryActorTick.bCanEverTick = true;

	_worldSize = 1.0f;
	_columns = 8;
	_rows = 1;
	_catchUpSpeedCoeff = 1.2f;
	_catchUpDistance = 1.0f;

	_root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = _root;

	_billboard = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Billboard"));
	_billboard->SetupAttachment(_root);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> UnitQuad(TEXT("StaticMesh'/Game/Common/Meshes/SM_UnitQuad.SM_UnitQuad'"));
	_billboard->SetStaticMesh(UnitQuad.Object);
}

void AWHModel::SetAnimating(bool value)
{
	if (_animating != value)
	{
		_animating = value;
		if (_animating)
		{
			_frameTimer = 0.0f;
		}
	}
}

void AWHModel::SetTarget(FVector position, FRotator rotation)
{
	_targetPosition = position;
	_targetRotation = rotation;
}

void AWHModel::SetGoal(EModelGoals goal)
{
	if (_currentGoal != goal)
	{
		switch (goal)
		{
			case EModelGoals::Move:
			{
				_reachedTarget = false;
			}
			break;
		}
		_currentGoal = goal;
	}
}

bool AWHModel::GetIsMoving()
{
	return _isMoving;
}

void AWHModel::SetParent(AWHUnit* parent)
{
	_parent = parent;
}

void AWHModel::ProjectToTerrain(FVector location)
{
	if (_levelData)
	{
		_levelData->ProjectToTerrainBase(location.X, location.Y, location);
	}
	SetActorLocation(location);
}

void AWHModel::FindLevelData()
{
	_levelData = nullptr;
	for (TActorIterator<ALevelData> iter(GetWorld()); iter; ++iter)
	{
		_levelData = *iter;
		break;
	}
}

void AWHModel::SetAnimationData()
{
	for (int index = 0, len = _actions.Num(); index < len; ++index)
	{
		FSpriteAction& action = _actions[index];
		_animationMap.Add(action.Name, index);
	}
}

void AWHModel::UpdateAction(float deltaTime)
{
	if (_animating && _actions.Num() != 0)
	{
		FSpriteAction& spriteAction = _actions[_actionIndex];
		if (_frameTimer > spriteAction.FrameTime && spriteAction.FrameTime != 0.0f)
		{
			_frameTimer = FMath::Fmod(_frameTimer, spriteAction.FrameTime);
			++_animatingRow;
			if (_animatingRow > spriteAction.RowTo)
			{
				_animatingRow = spriteAction.RowFrom;
			}
		}
		_frameTimer += deltaTime;
	}
}

void AWHModel::UpdateFrame(const FVector& cameraForward)
{
	FVector actorForward = GetActorForwardVector();

	FVector2D cameraForwardInPlane(cameraForward.X, cameraForward.Y);
	cameraForwardInPlane.Normalize();

	FVector2D actorForwardInPlane(actorForward.X, actorForward.Y);
	actorForwardInPlane.Normalize();

	float angle = FMath::RadiansToDegrees(FMath::Atan2(actorForwardInPlane.Y, actorForwardInPlane.X) - FMath::Atan2(cameraForwardInPlane.Y, cameraForwardInPlane.X));
	if (angle < 0.0f)
	{
		angle += 360.0f;
	}

	float frameIndex = angle / (360.0f / _columns) + 0.5f;
	frameIndex = FMath::TruncToFloat(frameIndex);
	frameIndex = FMath::Fmod(frameIndex, _columns);
	frameIndex /= _columns;

	static FName UvTransform(TEXT("UvTransform"));
	_materialInstance->SetVectorParameterValue(UvTransform, FLinearColor(1.0f / _columns, 1.0f / _rows, frameIndex, (float)_animatingRow / (float)_rows));
}

void AWHModel::UpdateMovement(float deltaTime)
{
	_isMoving = false;

	switch (_currentGoal)
	{
		default:
		case EModelGoals::Idle:
		{
			// Doing nothing
		}
		break;

		case EModelGoals::Move:
		{
			if (!_reachedTarget)
			{
				FVector location = GetActorLocation();
				FVector direction = _targetPosition - location;
				float directionLength = direction.Size();

				_reachedTarget = directionLength < 1e-3f;
				if (!_reachedTarget)
				{
					// Still not reached target this update

					_currentSpeed = FMath::Min(_moveSpeed, _currentSpeed + deltaTime * _acceleration);
					float speedCoeff = FMath::Clamp(directionLength / _catchUpDistance, 1.0f, _catchUpSpeedCoeff);
					float speed = _currentSpeed * speedCoeff;

					float targetDistance = deltaTime * speed;
					targetDistance = FMath::Min(targetDistance, directionLength);
					FVector deltaMovement = direction * (targetDistance / directionLength);
					
					location += deltaMovement;
					ProjectToTerrain(location);
				}
				// else - finally reached target
			}
			else
			{
				if (_currentSpeed > 0.0f)
				{
					_currentSpeed -= deltaTime * _deceleration;
					_currentSpeed = FMath::Max(_currentSpeed, 0.0f);

					FVector location = GetActorLocation();
					FVector direction = GetActorForwardVector();
					FVector deltaMovement = direction * _currentSpeed * deltaTime;
					
					location += deltaMovement;
					ProjectToTerrain(location);
				}
			}

			FRotator currentRotation = GetActorRotation();
			FRotator rotation = FMath::RInterpTo(GetActorRotation(), _targetRotation, deltaTime, _rotationSpeed);
			bool rotating = currentRotation != rotation;
			if (rotating)
			{
				SetActorRotation(rotation);
			}
			
			_isMoving = _currentSpeed != 0.0f && rotating;
		}
		break;

		case EModelGoals::Charge:
		{
			//TODO
		}
		break;
		
		case EModelGoals::Attack:
		{
			//TODO
		}
		break;
	}
}

void AWHModel::UpdateAnimation()
{
	switch (_currentGoal)
	{
		case EModelGoals::Idle:
			SetActionByNameIfDifferent(WalkName);
			SetAnimating(false);
			break;

		case EModelGoals::Move:
			SetActionByNameIfDifferent(WalkName);
			SetAnimating(_isMoving);
			break;

		case EModelGoals::Charge:
			SetActionByNameIfDifferent(ChargeName);
			SetAnimating(_isMoving);
			break;

		case EModelGoals::Attack:
			if (_isMoving)
			{
				SetActionByNameIfDifferent(WalkName);
			}
			else
			{
				SetActionByNameIfDifferent(AttackName);
			}
			SetAnimating(true);
			break;
	}
}

void AWHModel::OnConstruction(const FTransform& transform)
{
	Super::OnConstruction(transform);

	_materialInstance = _material != nullptr ? UMaterialInstanceDynamic::Create(_material, this) : nullptr;
	_billboard->SetMaterial(0, _materialInstance);
	_billboard->SetRelativeScale3D(FVector(_worldSize));

	if (_materialInstance != nullptr)
	{
		SetActionByIndex(0);
		UpdateFrame(FVector::ForwardVector);
	}
}

void AWHModel::BeginPlay()
{
	Super::BeginPlay();

	FindLevelData();
	SetAnimationData();

	SetGoal(EModelGoals::Idle);
}

void AWHModel::Tick(float deltaTime)
{
	Super::Tick(deltaTime);

	UpdateMovement(deltaTime);
	UpdateAnimation();

	APlayerCameraManager* pcm = UGameplayStatics::GetPlayerCameraManager(this, 0);
	if (pcm != nullptr)
	{
		FVector cameraForward = pcm->GetActorForwardVector();
		FVector cameraBackward = -cameraForward;

		_billboard->SetWorldRotation(cameraBackward.Rotation());

		if (_materialInstance != nullptr)
		{
			UpdateAction(deltaTime);
			UpdateFrame(cameraForward);
		}
	}
}

bool AWHModel::ShouldTickIfViewportsOnly() const
{
	return false;
}

void AWHModel::SetActionByIndex(int32 actionIndex)
{
	_frameTimer = 0.0f;
	_actionIndex = actionIndex;
	_animatingRow = _actions[actionIndex].RowFrom;
}

void AWHModel::SetActionByName(FName actionName)
{
	int32* valuePtr = _animationMap.Find(actionName);
	if (valuePtr)
	{
		SetActionByIndex(*valuePtr);
	}
}

void AWHModel::SetActionByNameIfDifferent(FName actionName)
{
	int32* valuePtr = _animationMap.Find(actionName);
	if (valuePtr)
	{
		int32 value = *valuePtr;
		if (value != _actionIndex)
		{
			SetActionByIndex(value);
		}
	}
}
