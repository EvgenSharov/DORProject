#pragma once

#include "Engine.h"
#include "DarkOmenLight.generated.h"

enum class EDarkOmenLightType : uint8
{
	None,
	Directional,
	Point
};

UCLASS(Abstract, HideCategories=(Mobility))
class DOR_API UDarkOmenLightComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category=Light, meta=(HideAlphaChannel))
	FLinearColor LightColor;

	UPROPERTY(EditAnywhere, Category = Light)
	uint8 bEnabled : 1;

	UPROPERTY(EditAnywhere, Category=Light)
	uint8 bCastLight : 1;

	UPROPERTY(EditAnywhere, Category=Light)
	uint8 bCastShadow : 1;

	UPROPERTY(EditAnywhere, Category=Light)
	uint8 bAffectsBase : 1;

	UPROPERTY(EditAnywhere, Category=Light)
	uint8 bAffectsFurn : 1;

	UPROPERTY(EditAnywhere, Category = RebuildLight)
	uint8 bRebuildLight : 1;

#if WITH_EDITORONLY_DATA
	UPROPERTY(transient)
	UTexture2D* EditorTexture;
#endif

public:
	UDarkOmenLightComponent(const FObjectInitializer& ObjectInitializer);

public:
#if WITH_EDITOR
	virtual void OnRegister() override;
#endif

#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& E) override;
#endif

	virtual EDarkOmenLightType GetLightType();
};

// ------------------------------------------------------------------------------------------

UCLASS(ClassGroup=(Lights))
class DOR_API UDarkOmenPointLightComponent : public UDarkOmenLightComponent
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category=Light, meta=(UIMin="0.0", UIMax="10.0", SliderExponent = "0.01"))
	float AttenuationCoeff;

public:
	UDarkOmenPointLightComponent(const FObjectInitializer& ObjectInitializer);

public:
	virtual EDarkOmenLightType GetLightType() override;
};

// ------------------------------------------------------------------------------------------

UCLASS(ClassGroup=(Lights))
class DOR_API UDarkOmenDirectionalLightComponent : public UDarkOmenLightComponent
{
	GENERATED_BODY()

public:
	UDarkOmenDirectionalLightComponent(const FObjectInitializer& ObjectInitializer);

public:
	virtual EDarkOmenLightType GetLightType() override;
};

// ==========================================================================================

UCLASS(Abstract, ClassGroup=Lights)
class DOR_API ADarkOmenLight : public AActor
{
	GENERATED_BODY()

private:
	UPROPERTY(VisibleAnywhere, Category=Light)
	UDarkOmenLightComponent* LightComponent;

public:
	ADarkOmenLight(const FObjectInitializer& ObjectInitializer);

public:
	UDarkOmenLightComponent* GetLightComponent();
	EDarkOmenLightType GetLightType();
};

// ------------------------------------------------------------------------------------------

UCLASS(ClassGroup=(Lights, PointLights))
class DOR_API ADarkOmenPointLight : public ADarkOmenLight
{
	GENERATED_BODY()

public:
	UPROPERTY()
	UDarkOmenPointLightComponent* PointLightComponent;

public:
	ADarkOmenPointLight(const FObjectInitializer& ObjectInitializer);
};

// ------------------------------------------------------------------------------------------

UCLASS(ClassGroup=(Lights, DirectionalLights))
class DOR_API ADarkOmenDirectionalLight : public ADarkOmenLight
{
	GENERATED_BODY()

private:
#if WITH_EDITORONLY_DATA
	UPROPERTY()
	UArrowComponent* ArrowComponent;
#endif

public:
	UPROPERTY()
	UDarkOmenDirectionalLightComponent* DirectionalLightComponent;

public:
	ADarkOmenDirectionalLight(const FObjectInitializer& ObjectInitializer);
};
