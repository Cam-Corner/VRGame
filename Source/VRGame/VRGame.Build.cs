// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class VRGame : ModuleRules
{
	public VRGame(ReadOnlyTargetRules Target) : base(Target)
	{
		//PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		PCHUsage = PCHUsageMode.UseSharedPCHs;

		//PrivatePCHHeaderFile = "Public/Networking/NetworkingHelpers.h";

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "UMG", "PhysicsCore" });

		PrivateDependencyModuleNames.AddRange(new string[] { "HeadMountedDisplay" });

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
