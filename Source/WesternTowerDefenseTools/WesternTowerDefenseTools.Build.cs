using UnrealBuildTool;

public class WesternTowerDefenseTools : ModuleRules
{
    public WesternTowerDefenseTools(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PublicIncludePaths.Add(ModuleDirectory + "/../WesternTowerDefense");

        PublicDependencyModuleNames.AddRange(new string[]
            { "Core", "CoreUObject", "Engine", "WesternTowerDefense" });

        PrivateDependencyModuleNames.AddRange(new string[]
            { "UnrealEd", "Blutility", "UMG", "UMGEditor", "Slate", "SlateCore", "EditorScriptingUtilities" });
    }
}
