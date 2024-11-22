// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Week01 : ModuleRules
{
	public Week01(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput" });
	}
}
