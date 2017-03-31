// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class PathfindingModule : ModuleRules
{
	public PathfindingModule(TargetInfo Target)
	{
		PublicIncludePaths.AddRange(new string[] { "PathfindingModule/Public" });
		PrivateIncludePaths.AddRange(new string[] { "PathfindingModule/Private", } );
					
		PublicDependencyModuleNames.AddRange(new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
			});
			
		
		PrivateDependencyModuleNames.AddRange(new string[]
			{
				"ProceduralMeshComponent",
				"DOR"
			});
	}
}
