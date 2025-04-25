// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class ShooterEditorTarget : TargetRules
{
	public ShooterEditorTarget( TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
        IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_5;
        DefaultBuildSettings = BuildSettingsVersion.V5;
        CppStandard = CppStandardVersion.Cpp20;
        bLegacyParentIncludePaths = false;
        bValidateFormatStrings = true;
        WindowsPlatform.bStrictConformanceMode = true;
        ExtraModuleNames.Add("Shooter");
	}
}
