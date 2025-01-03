// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class STEIKEMANN_UETarget : TargetRules
{
	public STEIKEMANN_UETarget( TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
        IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_1;
		DefaultBuildSettings = BuildSettingsVersion.V2;
        ExtraModuleNames.AddRange( new string[] { "STEIKEMANN_UE" } );
		ExtraModuleNames.AddRange( new string[] { "BaseClasses" } );
		ExtraModuleNames.AddRange( new string[] { "MechanicTestingClasses" } );
    }
}
