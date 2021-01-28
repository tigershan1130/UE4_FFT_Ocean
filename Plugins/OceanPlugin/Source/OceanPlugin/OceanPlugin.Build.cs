// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class OceanPlugin : ModuleRules
{
	public OceanPlugin(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"RenderCore",
				"RHI",
				"InputCore",
				"ShaderDeclaration",
				"Projects",
				"Slate",
				// ... add other public dependencies that you statically link with here ...
			}
			);
	}
}
