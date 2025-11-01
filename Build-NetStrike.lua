--premake5.lua

workspace "NetStrike"
architecture "x64"
configurations 
{ 
    "Debug-Server",
    "Debug-Client",
    "Release-Server",
    "Release-Client"
}

startproject "Game"

resourcedir = "%{wks.location}/Resources"
defines { 'RESOURCE_DIR="' .. resourcedir .. '"' }

--Workspace-wide build options for MSVC
filter "system:windows"
filter "configurations:Debug-Server or Debug-Client"
buildoptions 
{
    "/EHsc",
    "/Zc:preprocessor",
    "/Zc:__cplusplus",
    "/D_ITERATOR_DEBUG_LEVEL=2"
}

filter "configurations:Release-Server or Release-Client"
buildoptions
{
    "/EHsc",
    "/Zc:preprocessor",
    "/Zc:__cplusplus",
    "/D_ITERATOR_DEBUG_LEVEL=0"
}

filter "configurations:Debug"
linkoptions
{
    "/NODEFAULTLIB:msvcrt"
}

OutputDir = "%{cfg.system}-%{cfg.architecture}/%{cfg.buildcfg}"

include "Projects/Game/Build-Game.lua"