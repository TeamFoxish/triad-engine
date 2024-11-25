workspace("Triad")
  configurations({ "Debug", "DebugEditor", "Release", "ReleaseEditor" })
  platforms({ "Win32", "Win64" })
  filter { "platforms:Win32" }
    system "Windows"
    architecture "x86"
  
  filter { "platforms:Win64" }
    system "Windows"
    architecture "x86_64"

  filter("configurations:Debug*")
    defines({ "DEBUG", "JSON_DIAGNOSTICS" })
    symbols("On")
  
  filter("configurations:Release*")
    defines({ "NDEBUG" })
    optimize("On")
	
  filter("configurations:*Editor")
    defines({ "EDITOR" })
	
  filter("system:windows")
    cppdialect("C++20")
	--staticruntime("On") -- set if cpp runtime should be linked statically (/MT, /MTd)
	flags { 
	  "MultiProcessorCompile" -- enable multi-processor compilation
	}
	
  filter {  }
  
  defines({"_CRT_SECURE_NO_WARNINGS"}) -- ignore strncpy unsafety warnings caused by Strid
  
  location("build")
  targetdir("bin/%{cfg.buildcfg}")
  objdir("intermediate/%{cfg.buildcfg}")
  
  include("engine/premake5.lua")