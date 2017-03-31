#pragma once

#include "Engine.h"
#include "GameFramework/Actor.h"
#include "WHModel.generated.h"

class ALevelData;
class AWHUnit;

USTRUCT(BlueprintType)
struct FSpriteAction
{
	GENERATED_BODY()

	UPROPERTY(Category = "Sprite Action", EditAnywhere, BlueprintReadOnly)
	FName Name;

	UPROPERTY(Category = "Sprite Action", EditAnywhere, BlueprintReadOnly)
	int32 RowFrom;

	UPROPERTY(Category = "Sprite Action", EditAnywhere, BlueprintReadOnly)
	int32 RowTo;

	UPROPERTY(Category = "Sprite Action", EditAnywhere, BlueprintReadOnly)
	float FrameTime;
};

UENUM(BlueprintType)
enum class EModelGoals : uint8
{
	Idle,
	Move,
	Charge,
	Attack
};

UCLASS(Blueprintable)
class DOR_API AWHModel : public AActor
{
	GENERATED_BODY()

private:
	static FName WalkName;
	static FName ChargeName;
	static FName AttackName;
	static FName DeathName;

public:
	UPROPERTY()
	UMaterialInstanceDynamic* _materialInstance;
	
	UPROPERTY()
	ALevelData* _levelData;

	TMap<FName, int32> _animationMap;

	UPROPERTY(Transient)
	bool _animating;

	UPROPERTY(Transient)
	float _frameTimer;

	UPROPERTY(Transient)
	int32 _actionIndex;

	UPROPERTY(Transient)
	int32 _animatingRow;

	UPROPERTY(Transient)
	bool _isMoving;

	UPROPERTY(Transient)
	EModelGoals _currentGoal;

	UPROPERTY(Transient)
	FVector _targetPosition;

	UPROPERTY(Transient)
	FRotator _targetRotation;

	UPROPERTY(Category = Components, EditDefaultsOnly, BlueprintReadOnly)
	USceneComponent* _root;

	UPROPERTY(Category = Components, EditDefaultsOnly, BlueprintReadOnly)
	UStaticMeshComponent* _billboard;

	UPROPERTY(Category = Sprite, EditAnywhere, BlueprintReadOnly)
	UMaterialInterface* _material;

	UPROPERTY(Category = Sprite, EditAnywhere, BlueprintReadOnly)
	float _worldSize;

	UPROPERTY(Category = Sprite, EditAnywhere, BlueprintReadOnly)
	int32 _columns;

	UPROPERTY(Category = Sprite, EditAnywhere, BlueprintReadOnly)
	int32 _rows;

	UPROPERTY(Category = Sprite, EditAnywhere, BlueprintReadOnly)
	TArray<FSpriteAction> _actions;

	UPROPERTY(Category = Model, EditAnywhere, BlueprintReadOnly)
	float _moveSpeed;

	UPROPERTY(Category = Model, EditAnywhere, BlueprintReadOnly)
	float _chargeSpeed;

	UPROPERTY(Category = Model, EditAnywhere, BlueprintReadOnly)
	float _acceleration;

	UPROPERTY(Category = Model, EditAnywhere, BlueprintReadOnly)
	float _deceleration;

	UPROPERTY(Category = Model, EditAnywhere, BlueprintReadOnly)
	float _rotationSpeed;

	UPROPERTY(Category = Model, EditAnywhere, BlueprintReadOnly)
	float _catchUpSpeedCoeff;

	UPROPERTY(Category = Model, EditAnywhere, BlueprintReadOnly)
	float _catchUpDistance;

	AWHUnit* _parent;

	UPROPERTY(Transient)
	float _currentSpeed;

	bool _reachedTarget;

public:	
	AWHModel();

public:
	UFUNCTION(Category = AWHModel, BlueprintCallable)
	void SetAnimating(bool value);

	UFUNCTION(Category = AWHModel, BlueprintCallable)
	void SetTarget(FVector position, FRotator rotation);

	UFUNCTION(Category = AWHModel, BlueprintCallable)
	void SetGoal(EModelGoals goal);
	
	bool GetIsMoving();
	void SetParent(AWHUnit* parent);
	void ProjectToTerrain(FVector location);

private:
	void FindLevelData();
	void SetAnimationData();
	void UpdateAction(float deltaTime);
	void UpdateFrame(const FVector& cameraForward);
	void UpdateMovement(float deltaTime);
	void UpdateAnimation();

public:
	virtual void OnConstruction(const FTransform& transform) override;
	virtual void BeginPlay() override;
	virtual void Tick(float deltaSeconds) override;
	virtual bool ShouldTickIfViewportsOnly() const override;

	UFUNCTION(Category = AWHModel, BlueprintCallable)
	void SetActionByIndex(int32 actionIndex);

	UFUNCTION(Category = AWHModel, BlueprintCallable)
	void SetActionByName(FName actionName);

	UFUNCTION(Category = AWHModel, BlueprintCallable)
	void SetActionByNameIfDifferent(FName actionName);
};
