#include "PrjImporter.h"
#include "PrjFactory.h"

#include "ObjectTools.h"
#include "PackageTools.h"
#include "Engine/StaticMesh.h"
#include "RawMesh.h"
#include "Core.h"
#include "AssetRegistryModule.h"
#include "Materials/MaterialExpressionTextureSampleParameter2D.h"
#include "Materials/MaterialExpressionCustom.h"
#include "Materials/MaterialExpressionConstant.h"
#include "Materials/MaterialExpressionPanner.h"
#include "Materials/MaterialExpressionVertexColor.h"
#include "Materials/MaterialExpressionMultiply.h"
#include "PhysicsEngine/BodySetup.h"
#include "DarkOmenLight.h"

#include "PrjFile.h"
#include "M3dModel.h"
#include "BtbFile.h"

UPrjFactory::UPrjFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SupportedClass = UObject::StaticClass();
	Formats.Add(TEXT("prj;DO project file"));

	bCreateNew = false;
	bText = false;
	bEditorImport = true;
}


// Not used
bool UPrjFactory::ImportMeshNonMerged(FString& MeshesPackagePath, FString& MeshFolderPath, FString& MeshFileName, TMap<FString, UMaterialInterface*>& MaterialMap, TArray<UStaticMesh*>& OutStaticMeshes)
{
	/*FString FilePath = MeshFolderPath / MeshFileName;
	FString BaseMeshFileName = FPaths::GetBaseFilename(MeshFileName);
	FString SanitizedName = ObjectTools::SanitizeObjectName(BaseMeshFileName);

	M3dModel Model;
	if (!M3dModel::LoadFromM3d(FilePath, Model))
	{
		UE_LOG(PrjImporter, Warning, TEXT("Failed to load m3d model %s"), *FilePath);
		return false;
	}

	OutStaticMeshes.Empty();

	for (int32 ObjectIndex = 0; ObjectIndex < Model.Objects.Num(); ++ObjectIndex)
	{
		auto& Object = Model.Objects[ObjectIndex];

		FString AssetName = TEXT("SM_") + SanitizedName + TEXT("_") + Object.Name;
		FString AssetPath = MeshesPackagePath + AssetName;
		FString ObjectPath = AssetPath + TEXT(".") + AssetName;

		UStaticMesh* ExistingMesh = LoadObject<UStaticMesh>(nullptr, *ObjectPath);
		if (ExistingMesh != nullptr)
		{
			OutStaticMeshes.Add(ExistingMesh);
			UE_LOG(PrjImporter, Warning, TEXT("Sub static mesh %s already imported %s"), *AssetName, *FilePath);
			continue;
		}

		UPackage* Package = CreatePackage(nullptr, *AssetPath);

		UStaticMesh* StaticMesh = NewObject<UStaticMesh>(Package, FName(*AssetName), RF_Public | RF_Standalone);

		new(StaticMesh->SourceModels) FStaticMeshSourceModel();
		FStaticMeshSourceModel& SrcModel = StaticMesh->SourceModels[0];

		uint32 VertexOffset = 0;
		TMap<FString, int32> ModelMaterialMap;
		TArray<UMaterialInterface*> MaterialArray;

		FRawMesh NewRawMesh;
		FillRawMesh(&Model, ObjectIndex, VertexOffset, LightmapData, ModelMaterialMap, MaterialArray, MaterialMap, NewRawMesh);

		bool bValid = NewRawMesh.IsValidOrFixable();
		if (!bValid)
		{
			UE_LOG(PrjImporter, Error, TEXT("Sub RawMesh %s is not valid %s"), *AssetName, *MeshFileName);
			ObjectTools::DeleteSingleObject(StaticMesh);
			return false;
		}

		for (UMaterialInterface* MaterialInstance : MaterialArray)
		{
			StaticMesh->Materials.Add(MaterialInstance);
		}

		SrcModel.RawMeshBulkData->SaveRawMesh(NewRawMesh);
		SrcModel.BuildSettings.bGenerateLightmapUVs = false;
		SrcModel.BuildSettings.bRecomputeNormals = false;

		StaticMesh->Build();

		FAssetRegistryModule::AssetCreated(StaticMesh);

		Package->SetDirtyFlag(true);
		OutStaticMeshes.Add(StaticMesh);
	}*/

	return true;
}

bool UPrjFactory::CreateDebugHeightmapMesh(FString AssetPath, bool bTerr, int32 HeightMapIndex, PrjFile& ProjectFile)
{
	if (LoadObject<UStaticMesh>(nullptr, *(AssetPath + TEXT(".") + FPaths::GetBaseFilename(AssetPath))) != nullptr)
	{
		return true;
	}

	UPackage* Package = CreatePackage(nullptr, *AssetPath);

	UStaticMesh* StaticMesh = NewObject<UStaticMesh>(Package, FName(*FPaths::GetBaseFilename(AssetPath)), RF_Public | RF_Standalone);

	new(StaticMesh->SourceModels) FStaticMeshSourceModel();
	FStaticMeshSourceModel& SrcModel = StaticMesh->SourceModels[0];

	FRawMesh NewRawMesh;

	int32 BlocksX = ProjectFile.TerrainData.Width / 8;
	int32 BlocksY = ProjectFile.TerrainData.Height / 8;
	FVector2D BlockSize(8, 8);

	auto& HeightMapData = bTerr ? ProjectFile.TerrainData : ProjectFile.ShadowData;
	auto& HeightMap = bTerr ? (HeightMapIndex == 1 ? HeightMapData.HeightMap1 : HeightMapData.HeightMap2) : HeightMapData.HeightMap1;
	int32 MapWidth = HeightMapData.Width;
	int32 MapHeight = HeightMapData.Height;

	TMap<FString, int32> ModelMaterialMap;
	TArray<UMaterialInterface*> MaterialArray;
	for (int32 BlockIndex = 0; BlockIndex < ProjectFile.TerrainData.DataPairCount; ++BlockIndex)
	{
		float BaseHeight = HeightMap[BlockIndex].Key / 1024;

		int32 BlockX = BlockIndex % BlocksX;
		int32 BlockY = BlockIndex / BlocksX;

		FVector V0(-BlockX * BlockSize.X, BlockY * BlockSize.Y, BaseHeight);
		FVector V1 = V0 + FVector(-BlockSize.X, 0, 0);
		FVector V2 = V0 + FVector(-BlockSize.X, BlockSize.Y, 0);
		FVector V3 = V0 + FVector(0, BlockSize.Y, 0);

		bool bFine = true;
		if (bFine)
		{
			int32 MacroOffset = HeightMap[BlockIndex].Value;
			for (int32 SubIndex = 0; SubIndex < 64; ++SubIndex)
			{
				float Height = HeightMapData.MacroBlockData[MacroOffset + SubIndex] / 8;

				int32 SubBlockX = SubIndex % 8;
				int32 SubBlockY = SubIndex / 8;

				FVector SubV0 = V0 + FVector(-SubBlockX, SubBlockY, Height);
				FVector SubV1 = SubV0 + FVector(-1, 0, 0);
				FVector SubV2 = SubV0 + FVector(-1, 1, 0);
				FVector SubV3 = SubV0 + FVector(0, 1, 0);

				FVector2D SubUv0(-SubV0.X / MapWidth, SubV0.Y / MapHeight);
				FVector2D SubUv1(-SubV1.X / MapWidth, SubV1.Y / MapHeight);
				FVector2D SubUv2(-SubV2.X / MapWidth, SubV2.Y / MapHeight);
				FVector2D SubUv3(-SubV3.X / MapWidth, SubV3.Y / MapHeight);

				NewRawMesh.VertexPositions.Add(SubV0);
				NewRawMesh.VertexPositions.Add(SubV1);
				NewRawMesh.VertexPositions.Add(SubV2);
				NewRawMesh.VertexPositions.Add(SubV3);

				int32 V0Id = NewRawMesh.VertexPositions.Num() - 4;
				int32 V1Id = NewRawMesh.VertexPositions.Num() - 3;
				int32 V2Id = NewRawMesh.VertexPositions.Num() - 2;
				int32 V3Id = NewRawMesh.VertexPositions.Num() - 1;

				NewRawMesh.WedgeIndices.Add(V0Id);
				NewRawMesh.WedgeIndices.Add(V1Id);
				NewRawMesh.WedgeIndices.Add(V2Id);

				NewRawMesh.WedgeIndices.Add(V0Id);
				NewRawMesh.WedgeIndices.Add(V2Id);
				NewRawMesh.WedgeIndices.Add(V3Id);

				NewRawMesh.WedgeTexCoords[0].Add(SubUv0);
				NewRawMesh.WedgeTexCoords[0].Add(SubUv1);
				NewRawMesh.WedgeTexCoords[0].Add(SubUv2);

				NewRawMesh.WedgeTexCoords[0].Add(SubUv0);
				NewRawMesh.WedgeTexCoords[0].Add(SubUv2);
				NewRawMesh.WedgeTexCoords[0].Add(SubUv3);

				NewRawMesh.WedgeColors.Add(FColor::Black);
				NewRawMesh.WedgeColors.Add(FColor::Black);
				NewRawMesh.WedgeColors.Add(FColor::Black);

				NewRawMesh.WedgeColors.Add(FColor::Black);
				NewRawMesh.WedgeColors.Add(FColor::Black);
				NewRawMesh.WedgeColors.Add(FColor::Black);

				NewRawMesh.FaceSmoothingMasks.Add(0);
				NewRawMesh.FaceSmoothingMasks.Add(0);

				NewRawMesh.FaceMaterialIndices.Add(0);
				NewRawMesh.FaceMaterialIndices.Add(0);
			}
		}
		else
		{
			NewRawMesh.VertexPositions.Add(V0);
			NewRawMesh.VertexPositions.Add(V1);
			NewRawMesh.VertexPositions.Add(V2);
			NewRawMesh.VertexPositions.Add(V3);

			int32 V0Id = NewRawMesh.VertexPositions.Num() - 4;
			int32 V1Id = NewRawMesh.VertexPositions.Num() - 3;
			int32 V2Id = NewRawMesh.VertexPositions.Num() - 2;
			int32 V3Id = NewRawMesh.VertexPositions.Num() - 1;

			NewRawMesh.WedgeIndices.Add(V0Id);
			NewRawMesh.WedgeIndices.Add(V1Id);
			NewRawMesh.WedgeIndices.Add(V2Id);

			NewRawMesh.WedgeIndices.Add(V0Id);
			NewRawMesh.WedgeIndices.Add(V2Id);
			NewRawMesh.WedgeIndices.Add(V3Id);

			for (int32 Idx = 0; Idx < 6; ++Idx)
			{
				NewRawMesh.WedgeTexCoords[0].Add(FVector2D::ZeroVector);
				NewRawMesh.WedgeColors.Add(FColor::Black);
			}

			NewRawMesh.FaceSmoothingMasks.Add(0);
			NewRawMesh.FaceSmoothingMasks.Add(0);

			NewRawMesh.FaceMaterialIndices.Add(0);
			NewRawMesh.FaceMaterialIndices.Add(0);
		}
	}

	SrcModel.RawMeshBulkData->SaveRawMesh(NewRawMesh);

	auto MaterialInterface = UMaterial::GetDefaultMaterial(MD_Surface);
	StaticMesh->StaticMaterials.Add(FStaticMaterial(MaterialInterface, MaterialInterface != nullptr ? MaterialInterface->GetFName() : NAME_None));

	StaticMesh->Build();

	FAssetRegistryModule::AssetCreated(StaticMesh);

	Package->SetDirtyFlag(true);

	return true;
}


bool UPrjFactory::FindFile(FString& Mask, FString& OutFilePath)
{
	FString FolderPath = FPaths::GetPath(CurrentFilename);

	IFileManager& FileManager = IFileManager::Get();
	TArray<FString> FoundFiles;
	FileManager.FindFiles(FoundFiles, *FolderPath, *Mask);

	if (FoundFiles.Num() == 0)
	{
		return false;
	}

	OutFilePath = FolderPath / FoundFiles[0];

	return true;
}


bool UPrjFactory::ImportTextures(UObject* InParent, PrjFile& ProjectFile, TMap<FString, UTexture2D*>& OutTextureMap)
{
	FString TexturesFolderPath = FPaths::GetPath(CurrentFilename) + TEXT("/TEXTURE");

	if (!FPaths::DirectoryExists(TexturesFolderPath))
	{
		UE_LOG(PrjImporter, Warning, TEXT("TEXTURE folder does not exist"));
		return false;
	}

	IFileManager& FileManager = IFileManager::Get();
	TArray<FString> FoundFiles;
	FileManager.FindFiles(FoundFiles, *TexturesFolderPath);

	FString BasePackageName = FPackageName::GetLongPackagePath(InParent->GetOutermost()->GetName());
	BasePackageName = PackageTools::SanitizePackageName(BasePackageName);
	FString AssetFolderPath = BasePackageName / TEXT("Textures");

	FString TaskString = TEXT("Importing Texture: ");
	FScopedSlowTask SlowTask(FoundFiles.Num(), FText::FromString(TaskString));
	SlowTask.MakeDialog();

	for (int32 Index = 0; Index < FoundFiles.Num(); ++Index)
	{
		FString TextureFileName = FoundFiles[Index];
		FString Name = FPaths::GetBaseFilename(TextureFileName);

		SlowTask.EnterProgressFrame(1.0f, FText::FromString(TaskString + Name));

		FString TextureFilePath = TexturesFolderPath / TextureFileName;

		TArray<uint8> DataBinary;
		if (!FFileHelper::LoadFileToArray(DataBinary, *TextureFilePath))
		{
			UE_LOG(PrjImporter, Warning, TEXT("%s texture failed to load"), *TextureFilePath);
			continue;
		}
		const uint8* PtrTexture = DataBinary.GetData();
				
		FString Extension = FPaths::GetExtension(TextureFilePath).ToLower();
		FString SanitizedName = ObjectTools::SanitizeObjectName(Name);
		FString AssetName = "T_" + SanitizedName;
		FString PackageName = AssetFolderPath / AssetName;

		UTexture2D* ExistingTexture = nullptr;
		// First check if the asset already exists.
		{
			FString ObjectPath = PackageName + TEXT(".") + AssetName;
			ExistingTexture = LoadObject<UTexture2D>(NULL, *ObjectPath);
		}

		UTexture2D* UnrealTexture = nullptr;
		if (ExistingTexture == nullptr)
		{
			UPackage* TexturePackage = CreatePackage(NULL, *PackageName);

			UTextureFactory* TextureFact = NewObject<UTextureFactory>();
			TextureFact->AddToRoot();

			UnrealTexture = (UTexture2D*)TextureFact->FactoryCreateBinary(
				UTexture2D::StaticClass(), TexturePackage, *AssetName,
				RF_Standalone | RF_Public, NULL, *Extension,
				PtrTexture, PtrTexture + DataBinary.Num(), GWarn);

			if (UnrealTexture != nullptr)
			{
				//Make sure the AssetImportData point on the texture file and not on the fbx files since the factory point on the fbx file
				UnrealTexture->AssetImportData->Update(FileManager.ConvertToAbsolutePathForExternalAppForRead(*TextureFilePath));
				UnrealTexture->SRGB = 0;

				// Notify the asset registry
				FAssetRegistryModule::AssetCreated(UnrealTexture);

				// Set the dirty flag so this package will get saved later
				TexturePackage->SetDirtyFlag(true);
			}
			else
			{
				UE_LOG(PrjImporter, Warning, TEXT("Failed to import the texture %s"), *TextureFilePath);
			}

			TextureFact->RemoveFromRoot();
		}
		else
		{
			UnrealTexture = ExistingTexture;

			UE_LOG(PrjImporter, Warning, TEXT("Texture already imported %s"), *TextureFilePath);
		}

		if (UnrealTexture != nullptr)
		{
			OutTextureMap.Add(Name, UnrealTexture);
		}
	}

	return true;
}

bool UPrjFactory::CreateMaterial(FString& MaterialsPackagePath, EMaterialType MaterialType, UMaterial** OutMaterial)
{
	FString AssetName;

	switch (MaterialType)
	{
		default:
		case UPrjFactory::EMaterialType::Opaque:
			AssetName = "M_Opaque";
			break;

		case UPrjFactory::EMaterialType::Masked:
			AssetName = "M_Masked";
			break;

		case UPrjFactory::EMaterialType::Transparent:
			AssetName = "M_Transparent";
			break;
	}

	FString MaterialAssetPath = MaterialsPackagePath / AssetName;
	
	FString MaterialObjectPath = MaterialAssetPath + TEXT(".") + AssetName;
	UMaterial* ExistingObject = LoadObject<UMaterial>(NULL, *MaterialObjectPath);
	if (ExistingObject != nullptr)
	{
		*OutMaterial = ExistingObject;
		UE_LOG(PrjImporter, Warning, TEXT("Material %s already created"), *AssetName);
		return true;
	}

	// Material does not exists, create it

	UPackage* Package = CreatePackage(nullptr, *MaterialAssetPath);

	UMaterialFactoryNew* MaterialFactory = NewObject<UMaterialFactoryNew>();
	MaterialFactory->AddToRoot();

	UMaterial* UnrealMaterial = (UMaterial*)MaterialFactory->FactoryCreateNew(
		UMaterial::StaticClass(), Package, *AssetName, RF_Standalone | RF_Public, nullptr, GWarn);

	MaterialFactory->RemoveFromRoot();

	if (UnrealMaterial == nullptr)
	{
		UE_LOG(PrjImporter, Warning, TEXT("Failed to create material %s"), *AssetName);
		return false;
	}
	
	UnrealMaterial->SetShadingModel(EMaterialShadingModel::MSM_Unlit);

	UMaterialExpressionTextureSampleParameter2D* TextureExpression = NewObject<UMaterialExpressionTextureSampleParameter2D>(UnrealMaterial);
	TextureExpression->ParameterName = TEXT("Texture");
	TextureExpression->MaterialExpressionEditorX = -400;
	TextureExpression->MaterialExpressionEditorY = 0;
	UnrealMaterial->Expressions.Add(TextureExpression);
	
	UMaterialExpressionVertexColor* VertexColorExpression = NewObject<UMaterialExpressionVertexColor>(UnrealMaterial);
	VertexColorExpression->MaterialExpressionEditorX = -400;
	VertexColorExpression->MaterialExpressionEditorY = -200;
	UnrealMaterial->Expressions.Add(VertexColorExpression);

	UMaterialExpressionMultiply* MulExpression = NewObject<UMaterialExpressionMultiply>(UnrealMaterial);
	MulExpression->B.Expression = TextureExpression;
	MulExpression->A.Expression = VertexColorExpression;
	MulExpression->MaterialExpressionEditorX = -200;
	MulExpression->MaterialExpressionEditorY = 0;
	UnrealMaterial->Expressions.Add(MulExpression);

	UnrealMaterial->EmissiveColor.Expression = MulExpression;

	//if (MaterialType == EMaterialType::Masked)
	//{
	//	TextureExpression->SamplerSource = ESamplerSourceMode::SSM_Clamp_WorldGroupSettings;
	//}
		
	switch (MaterialType)
	{
		default:
		case UPrjFactory::EMaterialType::Opaque:
			{
				UnrealMaterial->BlendMode = EBlendMode::BLEND_Opaque;
			}
			break;

		case UPrjFactory::EMaterialType::Masked:
			{
				UnrealMaterial->BlendMode = EBlendMode::BLEND_Masked;

				UMaterialExpressionConstant* ConstantExpression = NewObject<UMaterialExpressionConstant>(UnrealMaterial);
				ConstantExpression->R = 0.1f;
				ConstantExpression->MaterialExpressionEditorX = -400;
				ConstantExpression->MaterialExpressionEditorY = 200;
				UnrealMaterial->Expressions.Add(ConstantExpression);

				UMaterialExpressionCustom* CustomExpression = NewObject<UMaterialExpressionCustom>(UnrealMaterial);
				CustomExpression->Inputs.Empty();
				FCustomInput ColorCustomInput;
				ColorCustomInput.InputName = FString(TEXT("color"));
				ColorCustomInput.Input.Expression = TextureExpression;
				FCustomInput ThresholdCustomInput;
				ThresholdCustomInput.InputName = FString(TEXT("threshold"));
				ThresholdCustomInput.Input.Expression = ConstantExpression;
				CustomExpression->Inputs.Add(ColorCustomInput);
				CustomExpression->Inputs.Add(ThresholdCustomInput);
				CustomExpression->Code = FString(TEXT("return (color.x+color.y+color.z)>threshold?1:0;"));
				CustomExpression->OutputType = ECustomMaterialOutputType::CMOT_Float1;
				CustomExpression->MaterialExpressionEditorX = -200;
				CustomExpression->MaterialExpressionEditorY = 200;
				UnrealMaterial->Expressions.Add(CustomExpression);
				UnrealMaterial->OpacityMask.Expression = CustomExpression;
			}
			break;

		case UPrjFactory::EMaterialType::Transparent:
			{
				UnrealMaterial->BlendMode = EBlendMode::BLEND_Translucent;

				UMaterialExpressionConstant* ConstantExpression = NewObject<UMaterialExpressionConstant>(UnrealMaterial);
				ConstantExpression->R = 0.5f;
				ConstantExpression->MaterialExpressionEditorX = -200;
				ConstantExpression->MaterialExpressionEditorY = 200;
				UnrealMaterial->Expressions.Add(ConstantExpression);
				UnrealMaterial->Opacity.Expression = ConstantExpression;

				UMaterialExpressionPanner* PannerExpression = NewObject<UMaterialExpressionPanner>(UnrealMaterial);
				PannerExpression->SpeedY = 1.0f;
				PannerExpression->MaterialExpressionEditorX = -600;
				PannerExpression->MaterialExpressionEditorY = 0;
				UnrealMaterial->Expressions.Add(PannerExpression);
				TextureExpression->Coordinates.Expression = PannerExpression;
			}
			break;
	}

	// let the material update itself if necessary
	UnrealMaterial->PreEditChange(nullptr);
	UnrealMaterial->PostEditChange();
	UnrealMaterial->MarkPackageDirty();

	*OutMaterial = UnrealMaterial;

	// Notify the asset registry
	FAssetRegistryModule::AssetCreated(UnrealMaterial);	

	// Set the dirty flag so this package will get saved later
	Package->SetDirtyFlag(true);

	return true;
}

bool UPrjFactory::CreateMaterialInstance(FString& MaterialInstancesPackagePath, UMaterial* Material, UTexture2D* Texture, FString& TextureName, UMaterialInstance** OutMaterialInstance)
{
	FString AssetName = TEXT("MI_") + TextureName;
	FString InstanceAssetPath = MaterialInstancesPackagePath / AssetName;

	FString InstanceObjectPath = InstanceAssetPath + TEXT(".") + AssetName;
	UMaterialInstance* ExistingObject = LoadObject<UMaterialInstance>(NULL, *InstanceObjectPath);
	if (ExistingObject != nullptr)
	{
		*OutMaterialInstance = ExistingObject;
		UE_LOG(PrjImporter, Warning, TEXT("MaterialInstance %s already created"), *AssetName);
		return true;
	}

	// MaterialInstance does not exists, create it

	UPackage* Package = CreatePackage(nullptr, *InstanceAssetPath);

	UMaterialInstanceConstantFactoryNew* InstanceFactory = NewObject<UMaterialInstanceConstantFactoryNew>();
	InstanceFactory->InitialParent = Material;
	InstanceFactory->AddToRoot();

	UMaterialInstance* MaterialInstance = (UMaterialInstance*)InstanceFactory->FactoryCreateNew(
		UMaterialInstanceConstant::StaticClass(), Package, *AssetName, RF_Standalone | RF_Public, nullptr, GWarn);

	InstanceFactory->RemoveFromRoot();

	if (MaterialInstance == nullptr)
	{
		UE_LOG(PrjImporter, Warning, TEXT("Failed to create material instance %s"), *AssetName);
		return false;
	}

	FTextureParameterValue TextureParameter;
	TextureParameter.ParameterName = TEXT("Texture");
	TextureParameter.ParameterValue = Texture;
	MaterialInstance->TextureParameterValues.Add(TextureParameter);

	*OutMaterialInstance = MaterialInstance;

	// Notify the asset registry
	FAssetRegistryModule::AssetCreated(MaterialInstance);

	// Set the dirty flag so this package will get saved later
	Package->SetDirtyFlag(true);

	return true;
}

bool UPrjFactory::CreateMaterialsAndInstances(UObject* InParent, TMap<FString, UTexture2D*>& TextureMap, TMap<FString, UMaterialInterface*>& OutMaterialMap)
{
	FString BasePackageName = FPackageName::GetLongPackagePath(InParent->GetOutermost()->GetName());
	BasePackageName = PackageTools::SanitizePackageName(BasePackageName);

	FString MaterialsPackagePath = BasePackageName + TEXT("/Materials/");
	FString MaterialInstancesPackagePath = BasePackageName + TEXT("/MaterialInstances/");

	UMaterial* OpaqueMaterial = nullptr;
	UMaterial* MaskedMaterial = nullptr;
	UMaterial* TransparentMaterial = nullptr;
	UMaterial* CurrentMaterial = nullptr;

	FString TaskString = TEXT("Creating Materials: ");
	FScopedSlowTask SlowTask(TextureMap.Num(), FText::FromString(TaskString));
	SlowTask.MakeDialog();

	for (auto& Pair : TextureMap)
	{
		FString TextureName = Pair.Key;
		UTexture2D* Texture = Pair.Value;

		SlowTask.EnterProgressFrame(1.0f, FText::FromString(TaskString + TextureName));

		if (TextureName.StartsWith(TEXT("_1")))
		{
			// Masked
			if (MaskedMaterial == nullptr)
			{
				if (!CreateMaterial(MaterialsPackagePath, EMaterialType::Masked, &MaskedMaterial))
				{
					return false;
				}
			}
			CurrentMaterial = MaskedMaterial;
		}
		else if (TextureName.StartsWith(TEXT("_2")))
		{
			// Transparent
			if (TransparentMaterial == nullptr)
			{
				if (!CreateMaterial(MaterialsPackagePath, EMaterialType::Transparent, &TransparentMaterial))
				{
					return false;
				}
			}
			CurrentMaterial = TransparentMaterial;
		}
		else
		{
			// Opaque
			if (OpaqueMaterial == nullptr)
			{
				if (!CreateMaterial(MaterialsPackagePath, EMaterialType::Opaque, &OpaqueMaterial))
				{
					return false;
				}
			}
			CurrentMaterial = OpaqueMaterial;
		}

		UMaterialInstance* MaterialInstance = nullptr;
		if (!CreateMaterialInstance(MaterialInstancesPackagePath, CurrentMaterial, Texture, TextureName, &MaterialInstance))
		{
			return false;
		}

		MaterialInstance->ForceRecompileForRendering();

		OutMaterialMap.Add(TextureName, MaterialInstance);
	}

	return true;
}


void UPrjFactory::FillRawMesh(
	M3dModel* Model, 
	uint32 ObjectIndex, 
	uint32& VertexOffset, 
	TMap<FString, int32>& ModelMaterialMap, 
	TArray<UMaterialInterface*>& MaterialArray, 
	TMap<FString, UMaterialInterface*>& MaterialMap, 
	TMap<uint32, uint32> VertexMap, 
	TArray<FVector> Vertices,
	TArray<FVector> Normals, 
	FRawMesh& NewRawMesh)
{
	M3dModel::Object& Obj = Model->Objects[ObjectIndex];
		
	for (int32 VertexIndex = 0; VertexIndex < Vertices.Num(); ++VertexIndex)
	{
		NewRawMesh.VertexPositions.Add(Vertices[VertexIndex]);
	}

	for (int32 FaceIndex = 0; FaceIndex < Obj.Faces.Num(); ++FaceIndex)
	{
		auto& Face = Obj.Faces[FaceIndex];

		int32 VertexIndex0 = Face.Vertex[0];
		int32 VertexIndex1 = Face.Vertex[2];
		int32 VertexIndex2 = Face.Vertex[1];

		NewRawMesh.WedgeTexCoords[0].Add(Obj.Vertices[VertexIndex0].Uv);
		NewRawMesh.WedgeTexCoords[0].Add(Obj.Vertices[VertexIndex1].Uv);
		NewRawMesh.WedgeTexCoords[0].Add(Obj.Vertices[VertexIndex2].Uv);
		
		VertexIndex0 += VertexOffset;
		VertexIndex1 += VertexOffset;
		VertexIndex2 += VertexOffset;

		int32 RemapIndex0 = VertexMap[VertexIndex0];
		int32 RemapIndex1 = VertexMap[VertexIndex1];
		int32 RemapIndex2 = VertexMap[VertexIndex2];
		int32 RemapIndices[3] = { RemapIndex0, RemapIndex1, RemapIndex2 };

		NewRawMesh.WedgeTangentZ.Add(Normals[RemapIndex0]);
		NewRawMesh.WedgeTangentZ.Add(Normals[RemapIndex1]);
		NewRawMesh.WedgeTangentZ.Add(Normals[RemapIndex2]);
		
		NewRawMesh.WedgeIndices.Add(RemapIndex0);
		NewRawMesh.WedgeIndices.Add(RemapIndex1);
		NewRawMesh.WedgeIndices.Add(RemapIndex2);

		NewRawMesh.WedgeColors.Add(FColor::White);
		NewRawMesh.WedgeColors.Add(FColor::White);
		NewRawMesh.WedgeColors.Add(FColor::White);
		//for (int32 I = 0; I < 3; ++I)
		//{
		//	FColor VertexColor = CalcLighting(Vertices[RemapIndices[I]], Normals[RemapIndices[I]]);
		//	NewRawMesh.WedgeColors.Add(VertexColor);
		//}

		NewRawMesh.FaceSmoothingMasks.Add(1);

		auto& ModelTexture = Model->Textures[Face.Texture];
		FString ModelTextureName = FPaths::GetBaseFilename(ModelTexture.Name).ToUpper();

		int32* TextureIndexPtr = ModelMaterialMap.Find(ModelTextureName);
		int32 TextureIndex;
		if (TextureIndexPtr == nullptr)
		{
			TextureIndex = MaterialArray.Num();
			ModelMaterialMap.Add(ModelTextureName, TextureIndex);

			UMaterialInterface** MaterialInstancePtr = MaterialMap.Find(ModelTextureName);
			UMaterialInterface* MaterialInstance = MaterialInstancePtr == nullptr ? nullptr : *MaterialInstancePtr;
			MaterialArray.Add(MaterialInstance);
		}
		else
		{
			TextureIndex = *TextureIndexPtr;
		}

		NewRawMesh.FaceMaterialIndices.Add(TextureIndex);
	}

	VertexOffset += Obj.Vertices.Num();
}

bool LoadM3d(FString& MeshFolderPath, FString& MeshFileName, M3dModel& OutModel)
{
	FString FilePath = MeshFolderPath / MeshFileName;

	if (!M3dModel::LoadFromM3d(FilePath, OutModel))
	{
		UE_LOG(PrjImporter, Warning, TEXT("Failed to load m3d model %s"), *FilePath);
		return false;
	}

	return true;
}

bool UPrjFactory::ImportMeshMerged(FString& MeshesPackagePath, FString& MeshFolderPath, FString& MeshFileName, float NormalDotProductThreshold, TMap<FString, UMaterialInterface*>& MaterialMap, UStaticMesh** OutStaticMesh)
{
	M3dModel Model;
	if (!LoadM3d(MeshFolderPath, MeshFileName, Model))
	{
		return false;
	}
		
	bool bResult = ImportMesh(0, (uint32)Model.NumObjects, Model, false, MeshesPackagePath, MeshFolderPath, MeshFileName, NormalDotProductThreshold, MaterialMap, OutStaticMesh);
	return bResult;
}

bool UPrjFactory::ImportMeshSeparated(FString& MeshesPackagePath, FString& MeshFolderPath, FString& MeshFileName, float NormalDotProductThreshold, TMap<FString, UMaterialInterface*>& MaterialMap, TArray<UStaticMesh*>& OutStaticMeshes)
{
	M3dModel Model;
	if (!LoadM3d(MeshFolderPath, MeshFileName, Model))
	{
		return false;
	}

	OutStaticMeshes.Empty();
	uint32 NumObjects = (uint32)Model.NumObjects;

	for (uint32 Index = 0; Index < NumObjects; ++Index)
	{
		UStaticMesh* SubMesh;
		bool bSuccess = ImportMesh(Index, Index + 1, Model, true, MeshesPackagePath, MeshFolderPath, MeshFileName, NormalDotProductThreshold, MaterialMap, &SubMesh);
		if (bSuccess)
		{
			OutStaticMeshes.Add(SubMesh);
		}
		else
		{
			return false;
		}
	}

	return true;
}

bool UPrjFactory::ImportMesh(uint32 StartIndex, uint32 EndIndex, M3dModel& Model, bool bAppendIndexToName, FString& MeshesPackagePath, FString& MeshFolderPath, FString& MeshFileName, float NormalDotProductThreshold, TMap<FString, UMaterialInterface*>& MaterialMap, UStaticMesh** OutStaticMesh)
{
	FString FilePath = MeshFolderPath / MeshFileName;
	FString BaseMeshFileName = FPaths::GetBaseFilename(MeshFileName);
	FString SanitizedName = ObjectTools::SanitizeObjectName(BaseMeshFileName);
	FString AssetName = TEXT("SM_") + SanitizedName;
	if (bAppendIndexToName)
	{
		AssetName += FString::Printf(TEXT("%d"), StartIndex);
	}
	FString AssetPath = MeshesPackagePath + AssetName;
	FString ObjectPath = AssetPath + TEXT(".") + AssetName;

	UStaticMesh* ExistingMesh = LoadObject<UStaticMesh>(nullptr, *ObjectPath);
	if (ExistingMesh != nullptr)
	{
		*OutStaticMesh = ExistingMesh;
		UE_LOG(PrjImporter, Warning, TEXT("Static mesh already imported %s"), *FilePath);
		return true;
	}

	//M3dModel Model;
	//if (!M3dModel::LoadFromM3d(FilePath, Model))
	//{
	//	UE_LOG(PrjImporter, Warning, TEXT("Failed to load m3d model %s"), *FilePath);
	//	return false;
	//}

	TMap<uint32, uint32> VertexMap;
	TArray<FVector> Verts;
	TArray<TArray<FVector>> Normals;
	uint32 VertexOffset = 0;

	for (uint32 ObjectIndex = StartIndex; ObjectIndex < EndIndex; ++ObjectIndex)
	{
		auto& Obj = Model.Objects[ObjectIndex];

		// If flags has 2 (i.e. 10 in binary) then pivot is used otherwise not
		FVector Pivot = (Obj.Flags & 2) ? Obj.Pivot : FVector::ZeroVector;

		for (int32 VertexIndex = 0; VertexIndex < Obj.Vertices.Num(); ++VertexIndex)
		{
			auto& Vertex = Obj.Vertices[VertexIndex];

			float X = (Vertex.Point.X + Pivot.X) * -1.0f;
			float Y = Vertex.Point.Z + Pivot.Z;
			float Z = Vertex.Point.Y + Pivot.Y;
			FVector Point(X, Y, Z);

			int32 OriginIndex = VertexIndex + VertexOffset;
			int32 RemapIndex = OriginIndex;
			
			FVector Normal(-Vertex.Normal.X, Vertex.Normal.Z, Vertex.Normal.Y);
			Normal = Normal.GetSafeNormal();
			TArray<FVector> VertexNormals;
			VertexNormals.Add(Normal);
			Normals.Add(VertexNormals);

			for (int32 Index = 0; Index < Verts.Num(); ++Index)
			{
				FVector& V = Verts[Index];
				float DistSqr = FVector::DistSquared(Point, V);
				if (DistSqr < 1e-1f)
				{
					TArray<FVector>& RemapNormals = Normals[Index];
					float DotProd = FVector::DotProduct(Normal, RemapNormals[0]);
					if (DotProd >= NormalDotProductThreshold)
					{
						RemapIndex = Index;
						break;
					}
				}
			}
			Verts.Add(Point);
			
			VertexMap.Add(OriginIndex, RemapIndex);

			if (OriginIndex != RemapIndex)
			{
				bool bFoundNormal = false;
				TArray<FVector>& RemapNormals = Normals[RemapIndex];

				for (int32 Index = 0; Index < RemapNormals.Num(); ++Index)
				{
					FVector& Item = RemapNormals[Index];
					float DistSqr = FVector::DistSquared(Item, Normal);
					if (DistSqr < 1e-4f)
					{
						bFoundNormal = true;
						break;
					}
				}
				if (!bFoundNormal)
				{
					RemapNormals.Add(Normal);
				}
			}
		}
		VertexOffset += Obj.NumVertices;
	}
	TArray<FVector> AvgNormals;
	AvgNormals.SetNumUninitialized(Normals.Num());
	for (int32 Index = 0; Index < Normals.Num(); ++Index)
	{
		TArray<FVector>& BucketNormals = Normals[Index];
		FVector AvgNormal = BucketNormals[0];
		for (int32 ArrayIndex = 1; ArrayIndex < BucketNormals.Num(); ++ArrayIndex)
		{
			AvgNormal += BucketNormals[ArrayIndex];
		}
		AvgNormal = AvgNormal.GetSafeNormal();
		AvgNormals[Index] = AvgNormal;
	}

	UPackage* Package = CreatePackage(nullptr, *AssetPath);

	UStaticMesh* StaticMesh = NewObject<UStaticMesh>(Package, FName(*AssetName), RF_Public | RF_Standalone);

	new(StaticMesh->SourceModels) FStaticMeshSourceModel();
	FStaticMeshSourceModel& SrcModel = StaticMesh->SourceModels[0];

	FRawMesh NewRawMesh;
	VertexOffset = 0;
	TMap<FString, int32> ModelMaterialMap;
	TArray<UMaterialInterface*> MaterialArray;
	for (uint32 ObjectIndex = StartIndex; ObjectIndex < EndIndex; ++ObjectIndex)
	{
		FillRawMesh(&Model, ObjectIndex, VertexOffset, ModelMaterialMap, MaterialArray, MaterialMap, VertexMap, Verts, AvgNormals, NewRawMesh);
	}

	bool bValid = NewRawMesh.IsValidOrFixable();
	if (!bValid)
	{
		UE_LOG(PrjImporter, Error, TEXT("RawMesh is not valid %s"), *MeshFileName);
		ObjectTools::DeleteSingleObject(StaticMesh);
		return false;
	}

	for (UMaterialInterface* MaterialInstance : MaterialArray)
	{
		StaticMesh->StaticMaterials.Add(FStaticMaterial(MaterialInstance, MaterialInstance != nullptr ? MaterialInstance->GetFName() : NAME_None));
	}

	SrcModel.RawMeshBulkData->SaveRawMesh(NewRawMesh);
	SrcModel.BuildSettings.bRecomputeNormals = false;

	//StaticMesh->bCustomizedCollision = true;
	StaticMesh->CreateBodySetup();
	StaticMesh->BodySetup->CollisionTraceFlag = ECollisionTraceFlag::CTF_UseComplexAsSimple;
	StaticMesh->BodySetup->bMeshCollideAll = true;

	for (int32 SectionIndex = 0; SectionIndex < StaticMesh->StaticMaterials.Num(); ++SectionIndex)
	{
		FMeshSectionInfo Info = StaticMesh->SectionInfoMap.Get(0, SectionIndex);
		Info.bEnableCollision = true;
		StaticMesh->SectionInfoMap.Set(0, SectionIndex, Info);
	}

	StaticMesh->Build();

	FAssetRegistryModule::AssetCreated(StaticMesh);

	Package->SetDirtyFlag(true);
	*OutStaticMesh = StaticMesh;

	return true;
}

bool UPrjFactory::ImportMeshes(UObject* InParent, PrjFile& ProjectFile, TMap<FString, UMaterialInterface*>& MaterialMap, TArray<UStaticMesh*>& OutBaseMeshes, UStaticMesh** OutWaterMesh, TArray<UStaticMesh*>& OutFurnMeshes)
{
	FString BasePackageName = FPackageName::GetLongPackagePath(InParent->GetOutermost()->GetName());
	BasePackageName = PackageTools::SanitizePackageName(BasePackageName);
	FString MeshesPackagePath = BasePackageName + TEXT("/Meshes/");
	FString MeshesFolderPath = FPaths::GetPath(CurrentFilename) + TEXT("/");
	
	if (!ImportMeshSeparated(MeshesPackagePath, MeshesFolderPath, ProjectFile.BaseFileName, 0.1f, MaterialMap, OutBaseMeshes))
	{
		UE_LOG(PrjImporter, Warning, TEXT("Failed to import BASE mesh"));
		return false;
	}

	if (!ProjectFile.WaterFileName.IsEmpty())
	{
		UStaticMesh* WaterMesh = nullptr;
		if (!ImportMeshMerged(MeshesPackagePath, MeshesFolderPath, ProjectFile.WaterFileName, 0.1f, MaterialMap, &WaterMesh))
		{
			UE_LOG(PrjImporter, Warning, TEXT("Failed to import WATR mesh"));
			return false;
		}
		*OutWaterMesh = WaterMesh;
	}
	else
	{
		*OutWaterMesh = nullptr;
	}

	TArray<FString> FurnFileNames;
	for (auto& FurnInst : ProjectFile.FurnitureInstances)
	{
		if (FurnInst.MeshSlot > 0)
		{
			FurnFileNames.Add(ProjectFile.FurnFileNames[FurnInst.MeshSlot - 1]);
		}
	}

	OutFurnMeshes.Empty();
	//for (FString& FurnFileName : ProjectFile.FurnFileNames)
	for (FString& FurnFileName : FurnFileNames)
	{
		UStaticMesh* FurnMesh = nullptr;
		if (!ImportMeshMerged(MeshesPackagePath, MeshesFolderPath, FurnFileName, -0.9f, MaterialMap, &FurnMesh))
		{
			UE_LOG(PrjImporter, Warning, TEXT("Failed to import Furn mesh %s"), *FurnFileName);
			return false;
		}
		OutFurnMeshes.Add(FurnMesh);
	}

	//TODO
	//if (0)
	//{
	//	CreateDebugHeightmapMesh(BasePackageName + TEXT("/TERR_Map1"), true, 1, ProjectFile);
	//	CreateDebugHeightmapMesh(BasePackageName + TEXT("/TERR_Map2"), true, 2, ProjectFile);
	//	CreateDebugHeightmapMesh(BasePackageName + TEXT("/SHD_Map"), false, 1, ProjectFile);
	//}

	return true;
}


bool UPrjFactory::CreateTerrainMap(UObject* InParent, PrjFile& ProjectFile, int32 MapIndex)
{
	FString AssetFolderPath = FPackageName::GetLongPackagePath(InParent->GetOutermost()->GetName()) + TEXT("/Terrain");
	FString AssetFileName = FString::Printf(TEXT("T_TerrainMap%d"), MapIndex);
	FString AssetFilePath = AssetFolderPath / AssetFileName;
	FString AssetObjectFilePath = AssetFilePath + TEXT(".") + AssetFileName;

	UTexture2D* ExistingAsset = LoadObject<UTexture2D>(nullptr, *AssetObjectFilePath);
	if (ExistingAsset != nullptr)
	{
		UE_LOG(PrjImporter, Warning, TEXT("Terrain light map is already created %s"), *AssetFilePath);
		return true;
	}

	//PrjFile::HeightMapDataFlattened ShdHeightMap;
	//PrjFile::FlattenHeightMapData(ProjectFile.ShadowData, 1, ShdHeightMap);

	PrjFile::HeightMapDataFlattened TerrHeightMap;
	PrjFile::FlattenHeightMapData(ProjectFile.TerrainData, MapIndex, TerrHeightMap);

	UPackage* Package = CreatePackage(nullptr, *AssetFilePath);

	//check((ShdHeightMap.Width == TerrHeightMap.Width) && (ShdHeightMap.Height == TerrHeightMap.Height));

	int32 Width = ProjectFile.TerrainData.Width;
	int32 Height = ProjectFile.TerrainData.Height;
	//int32 Width = ProjectFile.AttrBlockData.Width;
	//int32 Height = ProjectFile.AttrBlockData.Height;

	UTexture2D* Tex2D;
	{
		Tex2D = NewObject<UTexture2D>(Package, FName(*AssetFileName), RF_Standalone | RF_Public);
		Tex2D->Source.Init(Width, Height, 1, 1, TSF_G8);

		// Create base mip for the texture we created.
		uint8* MipData = Tex2D->Source.LockMip(0);
		for (int32 y = 0; y < Height; ++y)
		{
			uint8* DestPtr = &MipData[(Height - 1 - y) * Width];
			for (int32 x = 0; x < Width; ++x)
			{
				//int32 HeightMapIndex = x + (Height - 1 - y) * Width;
				int32 HeightMapIndex = x + y * Width;
				//int32 ShdValue = ShdHeightMap.Heightmap[HeightMapIndex];
				int32 TerrValue = TerrHeightMap.Heightmap[HeightMapIndex];
				//int32 LightMapValue = FMath::Max(0, ShdValue - TerrValue);

				//uint8 Col = (uint8)(((float)ShdHeightMap.Heightmap[x + (Height - 1 - y) * ShdHeightMap.Width] / 80.f) * 255);
				//*DestPtr++ = (uint8)LightMapValue;
				//uint8 Value = ProjectFile.AttrBlockData.Map[x + y * Width];

				*DestPtr++ = (uint8)TerrValue;
			}
		}
		Tex2D->Source.UnlockMip(0);

		// Set compression options.
		Tex2D->SRGB = 0;
		Tex2D->CompressionSettings = TextureCompressionSettings::TC_Grayscale;
		Tex2D->CompressionNoAlpha = true;
		Tex2D->DeferCompression = true;
		Tex2D->PostEditChange();
	}

	FAssetRegistryModule::AssetCreated(Tex2D);

	Package->SetDirtyFlag(true);

	return true;
}

bool UPrjFactory::ImportTerrainData(UObject* InParent, PrjFile& ProjectFile)
{
	if (!CreateTerrainMap(InParent, ProjectFile, 1))
	{
		return false;
	}

	if (!CreateTerrainMap(InParent, ProjectFile, 2))
	{
		return false;
	}

	return true;
}


bool UPrjFactory::ReadLights(TArray<FLitLight>& OutLights)
{
	FString LitFilePath;
	FString Mask(TEXT("LIT"));
	if (FindFile(Mask, LitFilePath))
	{
		bool bResult = LitFile::LoadFromLit(LitFilePath, OutLights);
		return bResult;
	}
	return false;
}

bool UPrjFactory::ReadBtbFile(BtbFile& OutBtbFile)
{
	FString FilePath = (FPaths::GetPath(CurrentFilename) / FPaths::GetBaseFilename(CurrentFilename)) + FString(TEXT(".BTB"));
	return BtbFile::LoadFromFile(FilePath, OutBtbFile);
}

bool UPrjFactory::FillLevel(PrjFile& ProjectFile, UWorld* World, TArray<UStaticMesh*> BaseMeshes, UStaticMesh* WaterMesh, TArray<UStaticMesh*> FurnMeshes, TArray<FLitLight>& Lights)
{
	ULevel* Level = World->GetLevel(0);

	// ===== Add geometry =====

	auto GetActorName = [](UStaticMesh* Value)
	{
		return Value->GetName().Mid(3);
	};
	auto AddStaticMesh = [&](UStaticMesh* Value)
	{
		AStaticMeshActor* StaticMeshActor = (AStaticMeshActor*)GEditor->AddActor(Level, AStaticMeshActor::StaticClass(), FTransform::Identity);
		StaticMeshActor->SetActorLabel(GetActorName(Value));
		StaticMeshActor->GetStaticMeshComponent()->StaticMesh = Value;
		StaticMeshActor->SetFolderPath("/Geometry");
		return StaticMeshActor;
	};

	for (int32 BaseIndex = 0; BaseIndex < BaseMeshes.Num(); ++BaseIndex)
	{
		FString BaseName = FString::Printf(TEXT("base%d"), BaseIndex);
		AddStaticMesh(BaseMeshes[BaseIndex])->SetActorLabel(BaseName);
	}
	
	if (WaterMesh != nullptr)
	{
		AddStaticMesh(WaterMesh)->SetActorLabel(TEXT("water"));
	}

	int32 Index = 0;
	for (int32 FurnIndex = 0; FurnIndex < ProjectFile.FurnitureInstances.Num(); ++FurnIndex)
	{
		auto& FurnInst = ProjectFile.FurnitureInstances[FurnIndex];
		//TODO create furn actor instead of just static mesh actor
		if (FurnInst.MeshSlot > 0)
		{
			//UStaticMesh* StaticMesh = FurnMeshes[FurnInst.MeshSlot - 1];
			UStaticMesh* StaticMesh = FurnMeshes[Index];
			++Index;
			AStaticMeshActor* Actor = AddStaticMesh(StaticMesh);
			Actor->SetActorLocation(FurnInst.Position);
			Actor->SetActorRotation(FurnInst.Rotation);
		}
	}

	// ===== Add lights =====

	float DirectionalZ = 50.0f;
	auto AddLight = [&](FLitLight& Light, int32 Index)
	{
		FString ActorName = FString::Printf(TEXT("Light%d_"), ((Index + 1) * 10));

		ADarkOmenLight* LightActor;
		if (Light.bDirectional)
		{
			LightActor = (ADarkOmenLight*)GEditor->AddActor(Level, ADarkOmenDirectionalLight::StaticClass(), FTransform::Identity);
			LightActor->SetActorLocation(FVector(0, 0, DirectionalZ += 10.0f));
			FVector Direction = -Light.Position;
			Direction = Direction.GetSafeNormal();
			LightActor->SetActorRotation(Direction.ToOrientationRotator());
			ActorName += "Directional";
		}
		else
		{
			LightActor = (ADarkOmenLight*)GEditor->AddActor(Level, ADarkOmenPointLight::StaticClass(), FTransform::Identity);
			LightActor->SetActorLocation(Light.Position);
			Cast<UDarkOmenPointLightComponent>(LightActor->GetLightComponent())->AttenuationCoeff = Light.Attenuation;
			ActorName += "Point";
		}
		UDarkOmenLightComponent* LightComponent = LightActor->GetLightComponent();
		LightComponent->LightColor = Light.Color;
		LightComponent->bCastLight = Light.bLight;
		LightComponent->bCastShadow = Light.bShadow;
		LightComponent->bAffectsBase = Light.bBase;
		LightComponent->bAffectsFurn = Light.bFurn;
		LightActor->SetFolderPath("/Lights");
		LightActor->SetActorLabel(ActorName);
	};

	for (int32 LightIndex = 0; LightIndex < Lights.Num(); ++LightIndex)
	{
		AddLight(Lights[LightIndex], LightIndex);
	}

	// ===== Add postprocessing blueprint =====

	FStringAssetReference ItemRef(TEXT("Blueprint'/Game/Common/BP_PostProcessing.BP_PostProcessing'"));
	ItemRef.TryLoad();
	UObject* ItemObj = ItemRef.ResolveObject();
	UBlueprint* Gen = Cast<UBlueprint>(ItemObj);
	AActor* PostProcessingActor = GEditor->AddActor(Level, Gen->GeneratedClass, FTransform::Identity);
	PostProcessingActor->SetFolderPath("/FX");

	Level->MarkPackageDirty();

	return true;
}


UObject* UPrjFactory::FactoryCreateBinary
(
	UClass*				Class,
	UObject*			InParent,
	FName				Name,
	EObjectFlags		Flags,
	UObject*			Context,
	const TCHAR*		Type,
	const uint8*&		Buffer,
	const uint8*		BufferEnd,
	FFeedbackContext*	Warn,
	bool&				bOutOperationCanceled
)
{
	//FEditorDelegates::OnAssetPreImport.Broadcast(this, NULL);

	PrjFile ProjectFile;
	if (!PrjFile::LoadFromPrj(CurrentFilename, ProjectFile))
	{
		UE_LOG(PrjImporter, Warning, TEXT("Failed to parse project file"));
		return nullptr;
	}

	TMap<FString, UTexture2D*> TextureMap;
	if (!ImportTextures(InParent, ProjectFile, TextureMap))
	{
		UE_LOG(PrjImporter, Warning, TEXT("Failed to import textures"));
		return nullptr;
	}

	TMap<FString, UMaterialInterface*> MaterialMap;
	if (!CreateMaterialsAndInstances(InParent, TextureMap, MaterialMap))
	{
		UE_LOG(PrjImporter, Warning, TEXT("Failed to create materials"));
		return nullptr;
	}

	TArray<UStaticMesh*> BaseMeshes;
	UStaticMesh* WaterMesh = nullptr;
	TArray<UStaticMesh*> FurnMeshes;
	TMap<FString, UStaticMesh*> MeshMap;

	if (!ImportMeshes(InParent, ProjectFile, MaterialMap, BaseMeshes, &WaterMesh, FurnMeshes))
	{
		UE_LOG(PrjImporter, Warning, TEXT("Failed to import meshes"));
		return nullptr;
	}

	//if (!ImportTerrainData(InParent, ProjectFile))
	//{
	//	UE_LOG(PrjImporter, Warning, TEXT("Failed to import terrain data"));
	//	return nullptr;
	//}

	TArray<FLitLight> Lights;
	if (!ReadLights(Lights))
	{
		UE_LOG(PrjImporter, Warning, TEXT("Failed to read lights"));
		return nullptr;
	}
	
	UWorldFactory* WorldFactory = NewObject<UWorldFactory>();
	WorldFactory->AddToRoot();

	UWorld* World = (UWorld*)WorldFactory->FactoryCreateNew(UWorld::StaticClass(), InParent, Name, Flags, nullptr, GWarn);

	WorldFactory->RemoveFromRoot();

	BtbFile Btb;
	if (!ReadBtbFile(Btb))
	{
		UE_LOG(PrjImporter, Error, TEXT("Failed reading BTB file"));
		return nullptr;
	}

	if (!FillLevel(ProjectFile, World, BaseMeshes, WaterMesh, FurnMeshes, Lights))
	{
		UE_LOG(PrjImporter, Error, TEXT("Level creation failed"));
	}

	World->MarkPackageDirty();	
	
	//FEditorDelegates::OnAssetPostImport.Broadcast(this, World);
	return World;
}

bool UPrjFactory::FactoryCanImport(const FString& Filename)
{
	const FString Extension = FPaths::GetExtension(Filename);
	if (Extension == TEXT("prj"))
	{
		return true;
	}
	return false;
}