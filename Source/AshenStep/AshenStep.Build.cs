// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class AshenStep : ModuleRules
{
	public AshenStep(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"Slate"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"AshenStep",
			"AshenStep/Variant_Platforming",
			"AshenStep/Variant_Platforming/Animation",
			"AshenStep/Variant_Combat",
			"AshenStep/Variant_Combat/AI",
			"AshenStep/Variant_Combat/Animation",
			"AshenStep/Variant_Combat/Gameplay",
			"AshenStep/Variant_Combat/Interfaces",
			"AshenStep/Variant_Combat/UI",
			"AshenStep/Variant_SideScrolling",
			"AshenStep/Variant_SideScrolling/AI",
			"AshenStep/Variant_SideScrolling/Gameplay",
			"AshenStep/Variant_SideScrolling/Interfaces",
			"AshenStep/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
