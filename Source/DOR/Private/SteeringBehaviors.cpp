#include "DOR.h"
#include "SteeringBehaviors.h"

// http://www.red3d.com/cwr/steer/gdc99/
// http://opensteer.sourceforge.net/doc.html
// http://natureofcode.com/book/chapter-6-autonomous-agents/
// https://github.com/libgdx/gdx-ai/wiki/Steering-Behaviors

USteeringAgent::USteeringAgent()
{
}

inline FVector ParallelComponent(FVector& thisVector, FVector& unitBasis)
{
	const float projection = FVector::DotProduct(thisVector, unitBasis);
	return unitBasis * projection;
}

inline FVector PerpendicularComponent(FVector& thisVector, FVector& unitBasis)
{
	return thisVector - ParallelComponent(thisVector, unitBasis);
}

FVector USteeringAgent::LimitDeviationAngle(bool insideOrOutside, FVector& source, float cosineOfConeAngle, FVector& basis)
{
	// This method is taken from OpenSteer

	// immediately return zero length input vectors
	float sourceLength = source.Size();
	if (sourceLength == 0) return source;

	// measure the angular diviation of "source" from "basis"
	FVector direction = source / sourceLength;
	float cosineOfSourceAngle = FVector::DotProduct(direction, basis);

	// Simply return "source" if it already meets the angle criteria.
	// (note: we hope this top "if" gets compiled out since the flag
	// is a constant when the function is inlined into its caller)
	if (insideOrOutside)
	{
		// source vector is already inside the cone, just return it
		if (cosineOfSourceAngle >= cosineOfConeAngle) return source;
	}
	else
	{
		// source vector is already outside the cone, just return it
		if (cosineOfSourceAngle <= cosineOfConeAngle) return source;
	}

	// find the portion of "source" that is perpendicular to "basis"
	FVector perp = PerpendicularComponent(source, basis);

	// normalize that perpendicular
	FVector unitPerp = perp.GetSafeNormal();

	// construct a new vector whose length equals the source vector,
	// and lies on the intersection of a plane (formed the source and
	// basis vectors) and a cone (whose axis is "basis" and whose
	// angle corresponds to cosineOfConeAngle)
	float perpDist = FMath::Sqrt(1 - (cosineOfConeAngle * cosineOfConeAngle));
	FVector c0 = basis * cosineOfConeAngle;
	FVector c1 = unitPerp * perpDist;
	FVector result = (c0 + c1) * sourceLength;
	
	return result;
}

void USteeringAgent::Init(FSteeringAgentData data)
{
	_maxForce = data.MaxForce;
	_maxSpeed = data.MaxSpeed;
	_constrainRotation = data.ConstrainRotation;
	_maxRotation = data.MaxRotation;
	_mass = data.Mass;
}

void USteeringAgent::UpdateVelocity(FVector steeringForce, FVector forward, float deltaTime, FVector& out_velocity)
{
	FVector clippedForce = steeringForce.GetClampedToMaxSize(_maxForce);
	FVector acceleration = clippedForce / _mass;

	FVector newVelocity = _velocity + acceleration * deltaTime;
	newVelocity = newVelocity.GetClampedToMaxSize(_maxSpeed);

	if (_constrainRotation)
	{
		float cosAngle = FMath::Cos(FMath::DegreesToRadians(_maxRotation * deltaTime));
		newVelocity = LimitDeviationAngle(true, newVelocity, cosAngle, forward);
	}
	newVelocity.Z = 0.0f;

	_velocity = newVelocity;
	out_velocity = newVelocity;
}

void USteeringAgent::UpdateActor(AActor* actor, float deltaTime)
{
	float speed = _velocity.Size();
	if (speed > 1e-5f)
	{
		FVector location = actor->GetActorLocation();
		location += _velocity * deltaTime;
		actor->SetActorLocation(location);

		FVector direction = _velocity / speed;
		actor->SetActorRotation(direction.Rotation());
	}
}

void USteeringAgent::SteerForSeek(FVector position, FVector target, FVector& out_force)
{
	FVector disiredVelocity = target - position;
	disiredVelocity = disiredVelocity.GetClampedToMaxSize(_maxSpeed);
	out_force = disiredVelocity - _velocity;
}
