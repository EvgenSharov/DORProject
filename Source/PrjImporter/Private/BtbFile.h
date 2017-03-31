#pragma once

#include "Core.h"

class BTBReader;

class BtbFile
{
public:
	class Objective
	{
	public:
		int32 TypeCode;
		int32 Val1;
		int32 Val2;

		Objective(int32 typecode, int32 val1, int32 val2)
		{
			TypeCode = typecode;
			Val1 = val1;
			Val2 = val2;
		}
	};

	class Obstacle
	{
	public:
		enum class PROP : uint8
		{
			ENABLED = 1,
			MOVE_BLOCK = 2,
			PROJ_BLOCK = 4
		};

		PROP Properties;
		int32 X;
		int32 Y;
		int32 Z;
		uint32 Radius;

		Obstacle(PROP properties, int32 x, int32 y, int32 z, uint32 radius)
		{
			Properties = properties;
			X = x;
			Y = y;
			Z = z;
			Radius = radius;
		}
	};

	class Region
	{
	public:
		class LineSegment
		{
		public:
			int32 StartX;
			int32 StartY;
			int32 EndX;
			int32 EndY;

			LineSegment(int32 startx, int32 starty, int32 endx, int32 endy)
			{
				StartX = startx;
				StartY = starty;
				EndX = endx;
				EndY = endy;
			}
		};

		enum class USAGE : uint8
		{
			SIGHT_EDGE,
			BOUNDARY,
			INV_BOUNDARY,
			PLAYER_DEPLOY,
			ENEMY_DEPLOY,
			PATH,
			BATTLE_EDGE
		};

		bool IsClosed;
		FString Name;
		TArray<LineSegment> Lines;
		USAGE Usage;

		Region(FString name, bool isClosed)
		{
			Name = name;
			IsClosed = isClosed;
		}

		//Region(int[][] pointList, string name, bool isClosed)
		//{
		//	Name = name;
		//	IsClosed = isClosed;
		//	int[] prev = pointList[0];
		//	for (int i = 1; i < pointList.Length; i++)
		//	{
		//		int[] cur = pointList[i];
		//		this.Lines.Add(new LineSegment(prev[0], prev[1],
		//			cur[0], cur[1]));
		//		prev = cur;
		//	}
		//}
	};

	class Node
	{
	public:
		enum class USAGE
		{
			UNKNOWN4,
			DEPLOYMENT,
			UNKNOWN2,
			NONE
		};

		int32 X;
		int32 Y;
		uint32 Radius;
		uint32 Direction;
		uint32 NodeID;
		uint32 UUID;
		uint32 ScriptFunc;
		USAGE Usage;

		Node()
		{
		}

		void Init(int32 x, int32 y, uint32 radius, uint32 direction, uint32 nodeID, uint32 uUID, uint32 scriptFunc, USAGE usage)
		{
			X = x;
			Y = y;
			Radius = radius;
			Direction = direction;
			NodeID = nodeID;
			UUID = uUID;
			ScriptFunc = scriptFunc;
			Usage = usage;
		}
	};

	class Battle
	{
	public:
		int32 Width;
		int32 Height;
		FString PlayerArmy;
		FString EnemyArmy;
		FString CTL;

		TArray<Objective> Objectives;
		TArray<Obstacle> Obstacles;
		TArray<Region> Regions;
		TArray<Node> Nodes;

		Battle()
		{
		}

		void Init(int32 width, int32 height, FString playerArmy, FString enemyArmy, FString ctl)
		{
			Width = width;
			Height = height;
			PlayerArmy = playerArmy;
			EnemyArmy = enemyArmy;
			CTL = ctl;
		}
	};

public:
	Battle BattleData;

public:
	static bool LoadFromFile(FString& FilePath, BtbFile& OutBtb);

private:
	static void CheckBTBFileType(BTBReader& reader);
	static void ReadMapHeaderObject(BTBReader& reader, Battle& battle);
	static void ReadObjectivesObject(BTBReader& reader, Battle& battle);
	static void ReadObstaclesObject(BTBReader& reader, Battle& battle);
	static void ReadRegions(BTBReader& reader, Battle& battle);
	static void ReadNodes(BTBReader& reader, Battle& battle);
};
