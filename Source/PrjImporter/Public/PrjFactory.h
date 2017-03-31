#pragma once

#include "PrjImporter.h"
#include "Engine.h"
#include "PrjFile.h"
#include "LitFile.h"
#include "PrjFactory.generated.h"

class BtbFile;

UCLASS()
class UPrjFactory : public UFactory
{
	GENERATED_UCLASS_BODY()

private:
	enum class EMaterialType : uint8
	{
		Opaque,
		OpaqueLightmapped,
		Masked,
		Transparent
	};

public:
	virtual UObject* FactoryCreateBinary(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, const TCHAR* Type, const uint8*& Buffer, const uint8* BufferEnd, FFeedbackContext* Warn, bool& bOutOperationCanceled) override;
	virtual bool FactoryCanImport(const FString& Filename) override;

private:
	bool FindFile(FString& Mask, FString& OutFilePath);

	bool ImportMeshNonMerged(FString& MeshesPackagePath, FString& MeshFolderPath, FString& MeshFileName, TMap<FString, UMaterialInterface*>& MaterialMap, TArray<UStaticMesh*>& OutStaticMeshes);
	bool CreateDebugHeightmapMesh(FString AssetPath, bool bTerr, int32 HeightMapIndex, PrjFile& ProjectFile);
	
	bool CreateTerrainMap(UObject* InParent, PrjFile& ProjectFile, int32 MapIndex);
	bool ImportTerrainData(UObject* InParent, PrjFile& ProjectFile);

	bool ImportTextures(UObject* InParent, PrjFile& ProjectFile, TMap<FString, UTexture2D*>& OutTextureMap);
	bool CreateMaterial(FString& MaterialsPackagePath, EMaterialType MaterialType, UMaterial** OutMaterial);
	bool CreateMaterialInstance(FString& MaterialInstancesPackagePath, UMaterial* Material, UTexture2D* Texture, FString& TextureName, UMaterialInstance** OutMaterialInstance);
	bool CreateMaterialsAndInstances(UObject* InParent, TMap<FString, UTexture2D*>& TextureMap, TMap<FString, UMaterialInterface*>& OutMaterialMap);
	
	void FillRawMesh(class M3dModel* Model, uint32 ObjectIndex, uint32& VertexOffset, TMap<FString, int32>& ModelMaterialMap, TArray<UMaterialInterface*>& MaterialArray, TMap<FString, UMaterialInterface*>& MaterialMap, TMap<uint32, uint32> VertexMap, TArray<FVector> Vertices, TArray<FVector> Normals, FRawMesh& NewRawMesh);
	bool ImportMeshMerged(FString& MeshesPackagePath, FString& MeshFolderPath, FString& MeshFileName, float NormalDotProductThreshold, TMap<FString, UMaterialInterface*>& MaterialMap, UStaticMesh** OutStaticMesh);
	bool ImportMeshSeparated(FString& MeshesPackagePath, FString& MeshFolderPath, FString& MeshFileName, float NormalDotProductThreshold, TMap<FString, UMaterialInterface*>& MaterialMap, TArray<UStaticMesh*>& OutStaticMeshes);
	bool ImportMesh(uint32 StartIndex, uint32 EndIndex, M3dModel& Model, bool bAppendIndexToName, FString& MeshesPackagePath, FString& MeshFolderPath, FString& MeshFileName, float NormalDotProductThreshold, TMap<FString, UMaterialInterface*>& MaterialMap, UStaticMesh** OutStaticMesh);
	bool ImportMeshes(UObject* InParent, PrjFile& ProjectFile, TMap<FString, UMaterialInterface*>& MaterialMap, TArray<UStaticMesh*>& OutBaseMeshes, UStaticMesh** OutWaterMesh, TArray<UStaticMesh*>& OutFurnMeshes);

	bool ReadLights(TArray<FLitLight>& OutLights);
	bool ReadBtbFile(BtbFile& OutBtbFile);

	bool FillLevel(PrjFile& ProjectFile, UWorld* World, TArray<UStaticMesh*> BaseMeshes, UStaticMesh* WaterMesh, TArray<UStaticMesh*> FurnMeshes, TArray<FLitLight>& Lights);
};
