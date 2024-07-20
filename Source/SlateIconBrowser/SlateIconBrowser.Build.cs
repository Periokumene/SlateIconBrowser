// Copyright 2022 sirjofri. Licensed under MIT license. See License.txt for full license text.

using UnrealBuildTool;

public class SlateIconBrowser : ModuleRules
{
	public SlateIconBrowser(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Projects",
				"InputCore",
				
#if UE_5_0_OR_LATER
				"EditorFramework",
#endif
				
				"UnrealEd",
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"EditorStyle",
				"ApplicationCore",
				"WorkspaceMenuStructure",
				"EditorWidgets",
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
			}
			);
	}
}
