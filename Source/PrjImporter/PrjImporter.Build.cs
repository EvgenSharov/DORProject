// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class PrjImporter : ModuleRules
{
	public PrjImporter(TargetInfo Target)
	{
		PublicIncludePaths.AddRange(new string[] { "PrjImporter/Public" });
        PrivateIncludePaths.AddRange(new string[] { "PrjImporter/Private" });

		PrivateIncludePathModuleNames.AddRange(
            new string[] {
                "AssetTools",
                "AssetRegistry"
            });

		PrivateDependencyModuleNames.AddRange(
            new string[] 
            {
                "RawMesh",
                "RenderCore",
				"DOR"
            });

        PublicDependencyModuleNames.AddRange(
            new string[] 
            {
                "Core",
                "CoreUObject",
                "Engine",
                "UnrealEd",
            });

        DynamicallyLoadedModuleNames.AddRange(new string[] { "AssetTools", "AssetRegistry" });
    }
}
