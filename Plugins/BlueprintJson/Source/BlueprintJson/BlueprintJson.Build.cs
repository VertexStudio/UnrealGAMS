// Copyright 2018 Maksim Shestakov. All Rights Reserved.

namespace UnrealBuildTool.Rules
{
	public class BlueprintJson : ModuleRules
	{
		public BlueprintJson(ReadOnlyTargetRules Target) : base(Target)
		{
			PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

            //MinFilesUsingPrecompiledHeaderOverride = 1; 
            //bFasterWithoutUnity = true;
			
			PrivateDependencyModuleNames.AddRange(
                new string[] { 
                    "Core", 
                    "CoreUObject",
                    "Engine",
                    "Json"
                }
            );
		}
	}
}
