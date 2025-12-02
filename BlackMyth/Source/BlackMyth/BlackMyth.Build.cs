// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class BlackMyth : ModuleRules
{
	public BlackMyth(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput" });
	}
}
