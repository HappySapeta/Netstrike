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
removefiles { "Source/Client.cpp" }

filter "configurations:Debug-Client or Release-Client"
removefiles { "Source/Server.cpp" }

filter {}

includedirs 
{
    "Include/",
	"../../ThirdParty/SFML-3.0.0/include"
}

libdirs
{
	"../../ThirdParty/SFML-3.0.0/lib/"
}

links { "opengl32.lib", "freetype.lib", "winmm.lib", "gdi32.lib", "flac.lib", "vorbisenc.lib", "vorbisfile.lib", "vorbis.lib", "ogg.lib", "ws2_32.lib" }

targetdir("../../Binaries/" .. OutputDir .. "/%{prj.name}")
objdir("../../Binaries/Intermediates/" .. OutputDir .. "/%{prj.name}")

filter "system:windows"
systemversion "latest"
defines { "WINDOWS", "WIN32" }

filter "configurations:Debug-Server or Debug-Client"
defines { "DEBUG" }
runtime "Debug"
symbols "On"
links 
{ 
    "sfml-audio-s-d.lib", 
    "sfml-graphics-s-d.lib", 
    "sfml-network-s-d.lib", 
    "sfml-system-s-d.lib", 
    "sfml-window-s-d.lib" 
}
--postbuildcommands
--{
--    "xcopy /Y /D \"..\\..\\ThirdParty\\SFML-3.0.0\\bin\\*.dll\" \"$(OutDir)\""
--}

filter "configurations:Release-Server or Release-Client"
defines { "RELEASE" }
runtime "Release"
optimize "On"
symbols "Off"
links 
{ 
    "sfml-audio-s.lib", 
    "sfml-graphics-s.lib", 
    "sfml-network-s.lib", 
    "sfml-system-s.lib", 
    "sfml-window-s.lib" 
}
--postbuildcommands
--{
--    "xcopy /Y /D \"..\\..\\ThirdParty\\SFML-3.0.0\\bin\\*.dll\" \"$(OutDir)\""
--}
