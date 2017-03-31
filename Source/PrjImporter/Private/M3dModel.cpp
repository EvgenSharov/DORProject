#include "PrjImporter.h"
#include "M3dModel.h"

M3dModel::M3dModel()
{
}

bool M3dModel::LoadFromM3d(FString Filepath, M3dModel& OutModel)
{
	TArray<uint8> DataBinary;
	if (!FFileHelper::LoadFileToArray(DataBinary, *Filepath))
	{
		return false;
	}

	uint32 FilePos = 0;
	auto ReadFile = [](TArray<uint8> Data, uint32& FilePos, void* Dest, uint32 Size)
	{
		FMemory::Memcpy(Dest, &Data[FilePos], Size);
		FilePos += Size;
	};

	struct M3DFILEHEADER
	{
		long ID;
		long Magic;
		long Version;
		long CRC;
		long NOT_CRC;
	};

	M3DFILEHEADER FileHeader;
	ReadFile(DataBinary, FilePos, &FileHeader, sizeof(M3DFILEHEADER));

	OutModel.ID = FileHeader.ID;
	OutModel.Magic = FileHeader.Magic;
	OutModel.Version = FileHeader.Version;
	OutModel.CRC = FileHeader.CRC;
	OutModel.NOT_CRC = FileHeader.NOT_CRC;

	struct M3DHEADER
	{
		short NumTextures;
		short NumObjects;
	};

	M3DHEADER Header;
	ReadFile(DataBinary, FilePos, &Header, sizeof(M3DHEADER));

	uint32 NumTextures = Header.NumTextures;
	uint32 NumObjects = Header.NumObjects;

	OutModel.NumTextures = NumTextures;
	OutModel.NumObjects = NumObjects;
	OutModel.Textures.SetNum(NumTextures);

	struct M3DTEXTURE
	{
		char Path[64];	//  path to texture.
		char Name[32];	//  filename of texture.
	};

	TArray<M3DTEXTURE> Textures;
	Textures.SetNumUninitialized(NumTextures);
	ReadFile(DataBinary, FilePos, &Textures[0], sizeof(M3DTEXTURE) * NumTextures);

	for (uint32 i = 0; i < NumTextures; ++i)
	{
		FString Path = FString(Textures[i].Path);
		FString Name = FString(Textures[i].Name);
		OutModel.Textures[i].Path = Path;
		OutModel.Textures[i].Name = Name;
	}

	struct M3DOBJECT
	{
		char		Name[32];	// object name
		short		Parent;		// parent index
		float		Pivot[3];	// origin for heirarchy
		short		Vertices;	// number of verts.
		short		Faces;		// number of faces
		long		Flags;
		long		Spare1;
		long		Spare2;
	};

	for (uint32 i = 0; i < NumObjects; ++i)
	{
		M3DOBJECT NativeObject;
		ReadFile(DataBinary, FilePos, &NativeObject, sizeof(M3DOBJECT));

		uint32 NumFaces = NativeObject.Faces;
		uint32 NumVertices = NativeObject.Vertices;

		if (NumFaces == 0 || NumVertices == 0)
		{
			// Skipping empty object
			continue;
		}

		Object Obj;
		Obj.Name = FString(NativeObject.Name);
		Obj.Parent = NativeObject.Parent;
		Obj.Pivot = FVector(NativeObject.Pivot[0], NativeObject.Pivot[1], NativeObject.Pivot[2]);
		Obj.NumVertices = NumVertices;
		Obj.NumFaces = NumFaces;
		Obj.Flags = NativeObject.Flags;
		Obj.Spare1 = NativeObject.Spare1;
		Obj.Spare2 = NativeObject.Spare2;

		Obj.Faces.SetNumUninitialized(NumFaces);
		ReadFile(DataBinary, FilePos, &Obj.Faces[0], sizeof(Face) * NumFaces);
			
		Obj.Vertices.SetNumUninitialized(NumVertices);
		ReadFile(DataBinary, FilePos, &Obj.Vertices[0], sizeof(Vertex) * NumVertices);
			
		OutModel.Objects.Add(Obj);
	}
	OutModel.NumObjects = (int16)OutModel.Objects.Num();

	return true;
}

//	bool Model::SaveToObj(std::string& filePath)
//	{
//		cout << "Started writing obj\n";
//
//		ofstream file(filePath, ofstream::out);
//
//		if (!file.is_open())
//		{
//			return false;
//		}
//
//		string mtlFilePath = filePath.substr(0, filePath.find_last_of(".")) + ".mtl";
//		string matlFileName = mtlFilePath.substr(mtlFilePath.find_last_of("\\/") + 1);
//
//		file << "mtllib " << matlFileName << endl;
//
//		map<uint16_t, map<uint32_t, vector<uint32_t>>> renderMap;
//		vector<uint32_t> vertexOffsets;
//		uint32_t vertexOffset = 0;
//		
//		for (uint32_t objectIndex = 0; objectIndex < Objects.size(); ++objectIndex)
//		{
//			Object& object = Objects[objectIndex];
//
//			//if (object.Name != "n1") continue;
//			//file << "g " << object.Name << endl;
//			
//			float pivotX = object.Pivot.X;
//			float pivotY = object.Pivot.Y;
//			float pivotZ = object.Pivot.Z;
//
//			for (auto& vertex : object.Vertices)
//			{
//				float invNormalLength = 1.0f / sqrtf(vertex.Normal.X * vertex.Normal.X + vertex.Normal.Y * vertex.Normal.Y + vertex.Normal.Z * vertex.Normal.Z);
//
//				float px = vertex.Point.X + pivotX;
//				float py = vertex.Point.Y + pivotY;
//				float pz = vertex.Point.Z + pivotZ;
//
//				float u = vertex.Uv.X;
//				float v = 1.0f - vertex.Uv.Y;
//
//				float nx = vertex.Normal.X * invNormalLength;
//				float ny = vertex.Normal.Y * invNormalLength;
//				float nz = vertex.Normal.Z * invNormalLength;
//
//				file << "v " << px << " " << pz << " " << py << endl;
//				file << "vt " << u << " " << v << endl;
//				file << "vn " << nx << " " << nz << " " << ny  << endl;
//			}
//
//			//map<uint16_t, vector<uint32_t>> textureToFaceMap;
//			for (uint32_t faceIndex = 0; faceIndex < object.Faces.size(); ++faceIndex)
//			{
//				uint16_t textureIndex = object.Faces[faceIndex].Texture;
//				//textureToFaceMap[textureIndex].push_back(faceIndex);
//				renderMap[textureIndex][objectIndex].push_back(faceIndex);
//			}
//
//			//for (auto& pair : textureToFaceMap)
//			//{
//			//	//++group;
//			//	//file << "g faces" << group << endl;
//
//			//	string originalTextureName = Textures[pair.first].Name;
//			//	string textureName = originalTextureName.substr(0, originalTextureName.find_last_of("."));
//			//	file << "usemtl " << "M_" << textureName << endl;
//			//	
//			//	for (auto& faceIndex : pair.second)
//			//	{
//			//		Face& face = object.Faces[faceIndex];
//			//		
//			//		int v0 = face.Vertex[0] + 1 + vertexOffset;
//			//		int v1 = face.Vertex[2] + 1 + vertexOffset;
//			//		int v2 = face.Vertex[1] + 1 + vertexOffset;
//
//			//		file << "f ";
//			//		file << v0 << "/" << v0 << "/" << v0 << " ";
//			//		file << v1 << "/" << v1 << "/" << v1 << " ";
//			//		file << v2 << "/" << v2 << "/" << v2 << endl;
//			//	}
//			//}
//
//			vertexOffsets.push_back(vertexOffset);
//			vertexOffset += object.Vertices.size();
//		}
//
//		uint32_t objectIndex = 0;
//		for (auto& pair : renderMap)
//		{
//			Texture& texture = Textures[pair.first];
//			string textureName = texture.Name.substr(0, texture.Name.find_last_of("."));
//
//			file << "usemtl " << "M_" << textureName << endl;
//			file << "g " << textureName << endl;
//
//			for (auto& objectPair : pair.second)
//			{
//				uint32_t objectIndex = objectPair.first;
//				Object& object = Objects[objectIndex];
//				uint32_t vertexOffset = vertexOffsets[objectIndex];
//
//				for (auto& faceIndex : objectPair.second)
//				{
//					Face& face = object.Faces[faceIndex];
//
//					int v0 = face.Vertex[0] + 1 + vertexOffset;
//					int v1 = face.Vertex[2] + 1 + vertexOffset;
//					int v2 = face.Vertex[1] + 1 + vertexOffset;
//
//					file << "f ";
//					file << v0 << "/" << v0 << "/" << v0 << " ";
//					file << v1 << "/" << v1 << "/" << v1 << " ";
//					file << v2 << "/" << v2 << "/" << v2 << endl;
//				}
//			}
//		}
//
//		file.close();
//				
//		ofstream mtlFile(mtlFilePath, ofstream::out);
//
//		if (!mtlFile.is_open())
//		{
//			return false;
//		}
//
//		for (auto& texture : Textures)
//		{
//			string textureName = texture.Name.substr(0, texture.Name.find_last_of("."));
//			mtlFile << "newmtl " << "M_" << textureName << endl;
//			mtlFile << "map_Kd " << "./TEXTURE/" << texture.Name << endl;
//		}
//
//		mtlFile.close();
//
//		cout << "Finished writing obj\n";
//
//		return true;
//	}
//}