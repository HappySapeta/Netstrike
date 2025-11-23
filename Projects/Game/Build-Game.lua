project "Game"
kind "ConsoleApp"
language "C++"
cppdialect "C++20"
targetdir "Binaries/%{cfg.buildcfg}"
staticruntime "off"

defines { "GAME_PLATFORM_WINDOWS", "SFML_STATIC" }

files 
{
    "Include/**.h",
    "Source/**.cpp"
}

filter "configurations:Debug-Server or Release-Server"
defines { "NS_SERVER" }
removefiles 
{ 
    "Source/Client.cpp", 
    "Include/Input.h",
    "Source/Input.cpp"
}

filter "configurations:Debug-Client or Release-Client"
defines { "NS_CLIENT" }
removefiles { "Source/Server.cpp" }

filter {}

includedirs 
{
    "Include/",
	"../../ThirdParty/SFML/include"
}

targetdir("../../Binaries/" .. OutputDir .. "/%{prj.name}")
objdir("../../Binaries/Intermediates/" .. OutputDir .. "/%{prj.name}")

filter "system:windows"
systemversion "latest"
defines { "WINDOWS", "WIN32" }

filter "configurations:Debug-Server or Debug-Client"
defines { "DEBUG" }
runtime "Debug"
symbols "On"
libdirs
{
    "../../ThirdParty/SFML/lib/Debug"
}
links
{
    "opengl32",
    "winmm",
    "gdi32",
    "ws2_32",
    "FLACd.lib",
    "freetyped.lib",
    "harfbuzzd.lib",
    "oggd.lib",
    "vorbisd.lib",
    "vorbisencd.lib",
    "vorbisfiled.lib",
    "sfml-main-d.lib",
    "sfml-system-s-d.lib",
    "sfml-audio-s-d.lib", 
    "sfml-window-s-d.lib",
    "sfml-graphics-s-d.lib",
    "sfml-network-s-d.lib",
}
postbuildcommands { "{COPYDIR} \"%{wks.location}Resources\" \"%{!cfg.targetdir}/Resources\"" }

filter {}

filter "configurations:Release-Server or Release-Client"
defines { "RELEASE" }
runtime "Release"
optimize "On"
symbols "Off"
libdirs
{
    "../../ThirdParty/SFML/lib/Release"
}
links
{
    "opengl32",
    "winmm",
    "gdi32",
    "ws2_32",
    "FLAC.lib",
    "freetype.lib",
    "harfbuzz.lib",
    "ogg.lib",
    "vorbis.lib",
    "vorbisenc.lib",
    "vorbisfile.lib",
    "sfml-main.lib",
    "sfml-system-s.lib",
    "sfml-audio-s.lib",
    "sfml-window-s.lib",
    "sfml-graphics-s.lib",
    "sfml-network-s.lib",
}
postbuildcommands { "{COPYDIR} \"%{wks.location}Resources\" \"%{!cfg.targetdir}/Resources\"" }
