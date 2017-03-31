#include "DOR.h"
#include "DarkOmenLight.h"

#if WITH_EDITOR
#include "LightingBuilder.h"
#endif

UDarkOmenLightComponent::UDarkOmenLightComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	LightColor = FLinearColor::White;
	bCastLight = true;
	bCastShadow = true;
	bEnabled = true;
	bAffectsBase = true;
	bAffectsFurn = true;
#if WITH_EDITORONLY_DATA
	bVisualizeComponent = true;
#endif
}

#if WITH_EDITOR
void UDarkOmenLightComponent::OnRegister()
{
	Super::OnRegister();

	if (SpriteComponent)
	{
		SpriteComponent->SpriteInfo.Category = TEXT("Lighting");
		SpriteComponent->SpriteInfo.DisplayName = NSLOCTEXT("SpriteCategory", "Lighting", "Lighting");
		SpriteComponent->SetSprite(EditorTexture);
		SpriteComponent->SetRelativeScale3D(FVector(0.25f));
	}
}
#endif

EDarkOmenLightType UDarkOmenLightComponent::GetLightType()
{
	return EDarkOmenLightType::None;
}

#if WITH_EDITOR
void UDarkOmenLightComponent::PostEditChangeProperty(struct FPropertyChangedEvent& E)
{
	//TODO this is hacky way to call function, should be a button probably
	// https://docs.unrealengine.com/latest/INT/Programming/Slate/DetailsCustomization/index.html

	FName PropertyName = (E.Property != NULL) ? E.MemberProperty->GetFName() : NAME_None;
	if (PropertyName == GET_MEMBER_NAME_CHECKED(UDarkOmenLightComponent, bRebuildLight))
	{
		UE_LOG(LogTemp, Warning, TEXT("Build DO Lighting"));
		ULightingBuilder::BuildDOLighting(GetWorld());
	}
	Super::PostEditChangeProperty(E);
}
#endif

// ------------------------------------------------------------------------------------------

UDarkOmenPointLightComponent::UDarkOmenPointLightComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
#if WITH_EDITORONLY_DATA
	if (!IsRunningCommandlet())
	{
		static ConstructorHelpers::FObjectFinder<UTexture2D> StaticTexture(TEXT("/Engine/EditorResources/LightIcons/S_LightPoint"));
		EditorTexture = StaticTexture.Object;
	}
#endif
	AttenuationCoeff = 0.0f;
}

EDarkOmenLightType UDarkOmenPointLightComponent::GetLightType()
{
	return EDarkOmenLightType::Point;
}

// ------------------------------------------------------------------------------------------

UDarkOmenDirectionalLightComponent::UDarkOmenDirectionalLightComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
#if WITH_EDITORONLY_DATA
	if (!IsRunningCommandlet())
	{
		static ConstructorHelpers::FObjectFinder<UTexture2D> StaticTexture(TEXT("/Engine/EditorResources/LightIcons/S_LightDirectional"));
		EditorTexture = StaticTexture.Object;
	}
#endif
}

EDarkOmenLightType UDarkOmenDirectionalLightComponent::GetLightType()
{
	return EDarkOmenLightType::Directional;
}

// ==========================================================================================

ADarkOmenLight::ADarkOmenLight(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	LightComponent = CreateAbstractDefaultSubobject<UDarkOmenLightComponent>(TEXT("LightComponent0"));

	bHidden = true;
	bCollideWhenPlacing = true;
}

UDarkOmenLightComponent* ADarkOmenLight::GetLightComponent()
{
	return LightComponent;
}

EDarkOmenLightType ADarkOmenLight::GetLightType()
{
	return LightComponent->GetLightType();
}

// ------------------------------------------------------------------------------------------

ADarkOmenPointLight::ADarkOmenPointLight(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UDarkOmenPointLightComponent>(TEXT("LightComponent0")))
{
	PointLightComponent = CastChecked<UDarkOmenPointLightComponent>(GetLightComponent());
	RootComponent = PointLightComponent;
}

// ------------------------------------------------------------------------------------------

ADarkOmenDirectionalLight::ADarkOmenDirectionalLight(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UDarkOmenDirectionalLightComponent>(TEXT("LightComponent0")))
{
	DirectionalLightComponent = CastChecked<UDarkOmenDirectionalLightComponent>(GetLightComponent());
	RootComponent = DirectionalLightComponent;

#if WITH_EDITORONLY_DATA
	struct FConstructorStatics
	{
		FName ID_Lighting;
		FText NAME_Lighting;
		FConstructorStatics()
			: ID_Lighting(TEXT("Lighting"))
			, NAME_Lighting(NSLOCTEXT("SpriteCategory", "Lighting", "Lighting"))
		{
		}
	};
	static FConstructorStatics ConstructorStatics;

	ArrowComponent = CreateEditorOnlyDefaultSubobject<UArrowComponent>(TEXT("ArrowComponent0"));
	if (ArrowComponent)
	{
		ArrowComponent->ArrowColor = FColor(150, 200, 255);

		ArrowComponent->bTreatAsASprite = true;
		ArrowComponent->SpriteInfo.Category = ConstructorStatics.ID_Lighting;
		ArrowComponent->SpriteInfo.DisplayName = ConstructorStatics.NAME_Lighting;
		ArrowComponent->SetupAttachment(DirectionalLightComponent);
		ArrowComponent->bLightAttachment = true;
		ArrowComponent->bIsScreenSizeScaled = true;
	}
#endif // WITH_EDITORONLY_DATA
}
