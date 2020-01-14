// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;
using System.Collections.Generic;

public class ADSimulatorEditorTarget : TargetRules
{
	public ADSimulatorEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		//DefaultBuildSettings = BuildSettingsVersion.V2; //Esconde o "upgrade" log nao acha Runnable

		ExtraModuleNames.AddRange( new string[] { "ADSimulator" } );
	}
}
