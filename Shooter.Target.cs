// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class ShooterTarget : TargetRules
{
	public ShooterTarget( TargetInfo Target) : base(Target)
	{
        DefaultBuildSettings = BuildSettingsVersion.V5;
        CppStandard = CppStandardVersion.Cpp20;
        IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_5;
        bLegacyParentIncludePaths = false;
        bValidateFormatStrings = true;
        WindowsPlatform.bStrictConformanceMode = true;
        ExtraModuleNames.Add("Shooter");
	}
}
