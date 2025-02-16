// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class STEIKEMANN_UETarget : TargetRules
{
	public STEIKEMANN_UETarget( TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
        IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_5;
		DefaultBuildSettings = BuildSettingsVersion.V2;
		CppStandard = CppStandardVersion.Cpp20;
        bLegacyParentIncludePaths = true;
        //BuildEnvironment = TargetBuildEnvironment.Unique;
        //bOverrideBuildEnvironment = true;
        WindowsPlatform.bStrictConformanceMode = true;
        bValidateFormatStrings = false;

        ExtraModuleNames.AddRange( new string[] { "STEIKEMANN_UE" } );
		ExtraModuleNames.AddRange( new string[] { "BaseClasses" } );
		ExtraModuleNames.AddRange( new string[] { "MechanicTestingClasses" } );
    }
}
