// Some copyright should be here...


namespace UnrealBuildTool.Rules
{
    public class ShaderDeclaration : ModuleRules
    {
        public ShaderDeclaration(ReadOnlyTargetRules Target) : base(Target)
        {
            PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

            PrivateIncludePaths.AddRange(new string[]
            {
                "ShaderDeclaration/Private"
            });

            PrivateDependencyModuleNames.AddRange(new string[]
            {
                "Core",
                "CoreUObject",
                "Engine",
                "Renderer",
                "RenderCore",
                "RHI",
                "Projects"
            });
        }
    }
}
