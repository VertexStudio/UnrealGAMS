// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;
using System.Collections.Generic;

public class UnrealGAMSTarget : TargetRules
{
	public UnrealGAMSTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;

		bUseUnityBuild = false;
		bUsePCHFiles = false;

		ExtraModuleNames.AddRange( new string[] { "UnrealGAMS" } );
	}
}
