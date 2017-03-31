#pragma once

#include "Engine.h"
#include "SteeringBehaviors.generated.h"

USTRUCT(BlueprintType)
struct FSteeringAgentData
{
	GENERATED_BODY()

public:
	UPROPERTY(Category = SteeringAgentData, EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "0"))
	float MaxForce;

	UPROPERTY(Category = SteeringAgentData, EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "0"))
	float MaxSpeed;

	UPROPERTY(Category = SteeringAgentData, EditAnywhere, BlueprintReadOnly)
	bool ConstrainRotation;

	UPROPERTY(Category = SteeringAgentData, EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "0"))
	float MaxRotation;

	UPROPERTY(Category = SteeringAgent, EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "0.001"))
	float Mass;
};

UCLASS(BlueprintType)
class USteeringAgent : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(Category = SteeringAgent, EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "0"))
	float _maxForce;
	
	UPROPERTY(Category = SteeringAgent, EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "0"))
	float _maxSpeed;

	UPROPERTY(Category = SteeringAgentData, EditAnywhere, BlueprintReadOnly)
	bool _constrainRotation;

	UPROPERTY(Category = SteeringAgentData, EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "0"))
	float _maxRotation;
	
	UPROPERTY(Category = SteeringAgent, EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "0.001"))
	float _mass;

	UPROPERTY(Transient)
	FVector _velocity;

public:
	USteeringAgent();

private:
	FVector LimitDeviationAngle(bool insideOrOutside, FVector& source, float cosineOfConeAngle, FVector& basis);

public:
	UFUNCTION(Category = SteeringAgent, BlueprintCallable)
	void Init(FSteeringAgentData data);

	UFUNCTION(Category = SteeringAgent, BlueprintCallable)
	void UpdateVelocity(FVector steeringForce, FVector forward, float deltaTime, FVector& out_velocity);

	UFUNCTION(Category = SteeringAgent, BlueprintCallable)
	void UpdateActor(AActor* actor, float deltaTime);

	UFUNCTION(Category = SteeringAgent, BlueprintCallable)
	void SteerForSeek(FVector position, FVector target, FVector& out_force);
};
