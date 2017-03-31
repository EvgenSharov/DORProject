#include "PrjImporter.h"
#include "BtbFile.h"

class FileReader
{
private:
	TArray<uint8> DataBinary;
	uint32 FilePos;

public:
	FileReader()
	{
	}

public:
	bool Init(FString& FilePath)
	{
		if (!FFileHelper::LoadFileToArray(DataBinary, *FilePath))
		{
			return false;
		}

		FilePos = 0;

		return true;
	}

	void ShiftPosition(uint32 Delta)
	{
		FilePos += Delta;
	}

	void ReadFile(void* Dest, uint32 Size)
	{
		FMemory::Memcpy(Dest, &DataBinary[FilePos], Size);
		FilePos += Size;
	}

	uint32 ReadUInt32()
	{
		uint32 Result;
		ReadFile(&Result, sizeof(Result));
		return Result;
	}

	int32 ReadInt32()
	{
		int32 Result;
		ReadFile(&Result, sizeof(Result));
		return Result;
	}

	void ReadBytes(void* Dest, uint32 Count)
	{
		ReadFile(Dest, Count);
	}
};

class BTBReader
{
private:
	FileReader& reader;

public:
	BTBReader(FileReader& Reader)
		: reader(Reader)
	{
	}

public:
	uint32 ReadObjectHeader(uint32 checkTypecode)
	{
		uint32 typecode = reader.ReadUInt32();
		uint32 length = reader.ReadUInt32();

		//if (typecode != checkTypecode)
		//	throw new UnexpectedObjectFound();

		return length;
	}

	void ReadPropertyHeader(uint32 checkTypecode, uint32 checkLength)
	{
		uint32 typecode = reader.ReadUInt32();
		uint32 length = reader.ReadUInt32();

		//if (typecode != checkTypecode)
		//	throw new UnexpectedPropertyFound();
		//if (length != (checkLength + 8))
		//	throw new UnexpectedPropertyLength();
	}

	TArray<int32> ReadIntTupleProperty(uint32 expectedTypeCode, uint32 arity)
	{
		ReadPropertyHeader(expectedTypeCode, arity * sizeof(int32));
		TArray<int32> tuple;
		tuple.SetNumUninitialized(arity);
		for (uint32 i = 0; i < arity; i++)
		{
			tuple[i] = reader.ReadInt32();
		}
		return tuple;
	}

	FString ReadStringProperty(uint32 expectedTypeCode)
	{
		ReadPropertyHeader(expectedTypeCode, 32);

		char Buffer[32];
		reader.ReadBytes(Buffer, 32);

		FString Result(Buffer);

		return Result;
	}
	
	uint32 PeekNextTypecode()
	{
		uint32 typecode = reader.ReadUInt32();
		reader.ShiftPosition(-4);
		return typecode;
	}
};

void BtbFile::CheckBTBFileType(BTBReader& reader)
{
	//try
	{
		reader.ReadObjectHeader(0xBEAFEED0);
	}
	//catch (UnexpectedObjectFound)
	{
	//	throw new NotABTBFileException();
	}
}

void BtbFile::ReadMapHeaderObject(BTBReader& reader, Battle& battle)
{
	reader.ReadObjectHeader(0x1);
	int32 width = reader.ReadIntTupleProperty(1, 1)[0];
	int32 height = reader.ReadIntTupleProperty(2, 1)[0];
	FString player = reader.ReadStringProperty(1001);
	FString enemy = reader.ReadStringProperty(1002);
	FString ctl = reader.ReadStringProperty(1003);
	reader.ReadStringProperty(1004);
	reader.ReadStringProperty(1005);
	reader.ReadIntTupleProperty(9, 2);
	battle.Init(width, height, player, enemy, ctl);
}

void BtbFile::ReadObjectivesObject(BTBReader& reader, Battle& battle)
{
	uint32 length = reader.ReadObjectHeader(0x2);
	for (uint32 i = 0; i < length; i += 20)
	{
		TArray<int32> tuple = reader.ReadIntTupleProperty(3, 3);
		Objective newObjective(tuple[0], tuple[1], tuple[2]);
		battle.Objectives.Add(newObjective);
	}
}

void BtbFile::ReadObstaclesObject(BTBReader& reader, Battle& battle)
{
	uint32 length = reader.ReadObjectHeader(0x3);

	int32 unknownCount = reader.ReadIntTupleProperty(8, 1)[0];
	uint32 obstacleCount = (length - 12u) / 80u;
	for (uint32 obstacleNum = 0; obstacleNum < obstacleCount; obstacleNum++)
	{
		reader.ReadPropertyHeader(501, 72);
		int32 flags = reader.ReadIntTupleProperty(5, 1)[0];
		int32 x = reader.ReadIntTupleProperty(1, 1)[0];
		int32 y = reader.ReadIntTupleProperty(2, 1)[0];
		int32 z = reader.ReadIntTupleProperty(4, 1)[0];
		int32 rad = reader.ReadIntTupleProperty(6, 1)[0];
		int32 dir = reader.ReadIntTupleProperty(7, 1)[0];
		Obstacle::PROP P = (Obstacle::PROP)flags;
		battle.Obstacles.Add(Obstacle(P, x, y, z, (uint32)rad));
	}
}

void BtbFile::ReadRegions(BTBReader& reader, Battle& battle)
{
	while (reader.PeekNextTypecode() == 0x4)
	{
		reader.ReadObjectHeader(0x4);
		FString regionName = reader.ReadStringProperty(1006);
		int32 flags = reader.ReadIntTupleProperty(5, 1)[0];
		Region newRegion(regionName, (flags & 2) != 0);
		TArray<int32> pos = reader.ReadIntTupleProperty(10, 2);
		while (reader.PeekNextTypecode() == 502)
		{
			TArray<int32> line = reader.ReadIntTupleProperty(502, 4);
			newRegion.Lines.Add(
				Region::LineSegment(line[0], line[1], line[2], line[3])
			);
		}
		battle.Regions.Add(newRegion);
	}
}

void BtbFile::ReadNodes(BTBReader& reader, Battle& battle)
{
	reader.ReadObjectHeader(0x5);
	uint32 nodeCount = (uint32)reader.ReadIntTupleProperty(8, 1)[0];
	for (uint32 nodeNum = 0; nodeNum < nodeCount; nodeNum++)
	{
		reader.ReadPropertyHeader(503, 96);
		int32 flags = reader.ReadIntTupleProperty(5, 1)[0];
		int32 x = reader.ReadIntTupleProperty(1, 1)[0];
		int32 y = reader.ReadIntTupleProperty(2, 1)[0];
		uint32 rad = (uint32)reader.ReadIntTupleProperty(6, 1)[0];
		uint32 dir = (uint32)reader.ReadIntTupleProperty(7, 1)[0];
		uint32 nodeid = (uint32)reader.ReadIntTupleProperty(11, 1)[0];
		uint32 uuid = (uint32)reader.ReadIntTupleProperty(12, 1)[0];
		uint32 scriptid = (uint32)reader.ReadIntTupleProperty(13, 1)[0];

		Node N;
		switch (flags)
		{
		case 3:
			N.Init(x, y, rad, dir, nodeid, uuid, scriptid, Node::USAGE::UNKNOWN2);
			break;

		case 5:
			N.Init(x, y, rad, dir, nodeid, uuid, scriptid, Node::USAGE::UNKNOWN4);
			break;

		case 16387:
			N.Init(x, y, rad, dir, nodeid, uuid, scriptid, Node::USAGE::DEPLOYMENT);
			break;

		case 1:
			N.Init(x, y, rad, dir, nodeid, uuid, scriptid, Node::USAGE::NONE);
			break;

		//default:
		//	throw new UnknownNodeTypeException();
		}
		battle.Nodes.Add(N);
	}
}

bool BtbFile::LoadFromFile(FString& FilePath, BtbFile& OutBtb)
{
	FileReader FReader;
	if (!FReader.Init(FilePath))
	{
		return false;
	}
	BTBReader reader(FReader);

	Battle& newBattle = OutBtb.BattleData;

	CheckBTBFileType(reader);
	ReadMapHeaderObject(reader, newBattle);
	ReadObjectivesObject(reader, newBattle);
	ReadObstaclesObject(reader, newBattle);
	ReadRegions(reader, newBattle);
	ReadNodes(reader, newBattle);
	CheckBTBFileType(reader);
	
	return true;
}