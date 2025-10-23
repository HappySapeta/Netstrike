--premake5.lua

workspace "NetStrike"
architecture "x64"
configurations 
{ 
    "Debug", 
    "Release" 
}

startproject "Game"

--Workspace-wide build options for MSVC
filter "system:windows"
filter "configurations:Debug"
buildoptions 
{
    "/EHsc",
    "/Zc:preprocessor",
    "/Zc:__cplusplus",
    "/D_ITERATOR_DEBUG_LEVEL=2"
}

filter "configurations:Release"
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