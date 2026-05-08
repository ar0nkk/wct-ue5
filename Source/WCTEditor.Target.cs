using UnrealBuildTool;
using System.Collections.Generic;

public class WCTEditorTarget : TargetRules
{
    public WCTEditorTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Editor;
        DefaultBuildSettings = BuildSettingsVersion.V6;
        IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
        ExtraModuleNames.Add("WCT");
    }
}
