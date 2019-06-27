// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;
using System.Collections.Generic;

public class UnrealGAMSEditorTarget : TargetRules
{
	public UnrealGAMSEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;

		bUseUnityBuild = false;
		bUsePCHFiles = false;

		ExtraModuleNames.AddRange( new string[] { "UnrealGAMS" } );
	}
}
