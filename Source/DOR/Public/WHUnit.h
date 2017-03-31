#pragma once

#include "GameFramework/Actor.h"
#include "WHUnit.generated.h"

class AWHModel;
class ALevelData;

UENUM(BlueprintType)
enum class EUnitGoals : uint8
{
	None,
	Idle,
	Moving
};

UCLASS(Blueprintable)
class DOR_API AWHUnit : public AActor
{
	GENERATED_BODY()
	
public:
	UPROPERTY(Category = Unit, EditAnywhere, BlueprintReadOnly)
	FVector2D _modelPadding;

	UPROPERTY(Category = Unit, EditAnywhere, BlueprintReadOnly)
	TSubclassOf<AWHModel> _modelClass;

	UPROPERTY(Category = Unit, EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "1"))
	int32 _modelCount;

	UPROPERTY(Category = Unit, EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "1"))
	int32 _lineCount;

#if WITH_EDITOR
	UPROPERTY(Category = Debug, EditAnywhere, BlueprintReadOnly)
	bool _debugFormation;

	UPROPERTY(Category = Debug, EditAnywhere, BlueprintReadOnly)
	float _debugSphereSize;
	
	UPROPERTY()
	TArray<USphereComponent*> _formationSpheres;
#endif

	ALevelData* _levelData;
	TArray<AWHModel*> _models;
	TArray<FVector2D> _offsets;
	float _moveSpeed;
	float _chargeSpeed;
	float _rotationSpeed;
	EUnitGoals _currentGoal;
	FVector _targetPoint;

public:
	AWHUnit();

private:
	void SetGoal(EUnitGoals goal);
	void ProjectUnitToTerrain();
	void CalcFormation(TArray<FVector2D>& offsets, int32 modelCount);

#if WITH_EDITOR
	void ShowDebugFormation(TArray<FVector2D>& offsets, int32 modelCount);
#endif

public:
	virtual void OnConstruction(const FTransform& transform) override;
	virtual void BeginPlay() override;
	virtual void Tick(float deltaSeconds) override;	

	UFUNCTION(Category = Model, BlueprintCallable)
	void MoveTo(FVector location);
};
