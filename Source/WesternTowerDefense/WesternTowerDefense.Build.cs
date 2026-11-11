// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class WesternTowerDefense : ModuleRules
{
	public WesternTowerDefense(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore",
                                                                "UMG", "Slate", "SlateCore",
                                                                "AIModule", "NavigationSystem", "GameplayTasks" });

		PrivateDependencyModuleNames.AddRange(new string[] {  });

	}
}
