using UnrealBuildTool;
using System.IO;

public class GStreamer : ModuleRules
{
    public GStreamer(ReadOnlyTargetRules Target) : base(Target)
    {
        const string GStreamerRoot = @"/usr";

        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        bEnableUndefinedIdentifierWarnings = false;

        PublicDependencyModuleNames.AddRange(
            new string[] {
                "Core",
                "CoreUObject",
                "Engine",
                "InputCore",
                "RHI",
                "RenderCore",
                "Slate",
                "SlateCore",
                "GStreamerLoader"
            }
        );

        PublicIncludePaths.Add(Path.Combine(GStreamerRoot, "/usr/include/gstreamer-1.0"));
        PublicIncludePaths.Add(Path.Combine(GStreamerRoot, "/usr/include/glib-2.0"));
        PublicIncludePaths.Add(Path.Combine(GStreamerRoot, "/usr/lib/x86_64-linux-gnu/glib-2.0/include"));
        PublicLibraryPaths.Add(Path.Combine(GStreamerRoot, "/usr/lib/x86_64-linux-gnu"));

        PublicAdditionalLibraries.Add("glib-2.0");
        PublicAdditionalLibraries.Add("gobject-2.0");
        PublicAdditionalLibraries.Add("gstreamer-1.0");
        PublicAdditionalLibraries.Add("gstapp-1.0");
        PublicAdditionalLibraries.Add("gstvideo-1.0");
    }
}
