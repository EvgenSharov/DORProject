// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class DOR : ModuleRules
{
	public DOR(TargetInfo Target)
	{
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"InputCore"
			});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		//if (Target.Type == TargetRules.TargetType.Editor)
		//{
		//	PublicDependencyModuleNames.Add("UnreadEd");
		//}

		// https://answers.unrealengine.com/questions/420623/cannot-find-hcadlib-when-using-unrealed.html
		if (UEBuildConfiguration.bBuildEditor)
		{
			PublicDependencyModuleNames.AddRange(new string[] { "UnrealEd", "RawMesh", "RenderCore" });
		}
	}
}
