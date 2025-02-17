local scriptDir = path.getdirectory(_SCRIPT)  -- Gets the directory of the Premake script

project("Engine")
  targetname("Engine")
  kind("ConsoleApp")  -- TODO: replace with SharedLib/StaticLib when scripting prototype is ready. 
                      -- Seekerses comment: maybe not. Engine actually .exe that starts and compiles scripts. Working like framework
  location("../build/Engine")
  language("C++")
  debugdir("")
  
  includedirs({ "source" })
  includedirs({ "libs/**/include" })
  includedirs({ "source/imgui" })
  
  filter("configurations:Debug*")
    libdirs({ "libs/**/debug" })
	links({ "assimp-vc143-mtd", "freetyped", "argumentumd", "yaml-cppd", "angelscriptd", "spdlogd", "fmodL_vc", "fmodstudioL_vc", "DebugUtils-d", "Detour-d", "DetourCrowd-d", "DetourTileCache-d", "Recast-d" })
  
  filter("configurations:Release*")
    libdirs({ "libs/**/release" })
	links({ "assimp-vc143-mt", "freetype", "argumentum", "yaml-cpp", "angelscript", "spdlog", "fmod_vc", "fmodstudio_vc", "Detour", "DetourCrowd", "DetourTileCache", "Recast" })
    
  filter("configurations:*Editor")
    links({ "DebugUtils-d" })
  
  filter {  }
  
  links({ "d3d11", "dxgi", "d3dcompiler", "dxguid", "DirectXTK", "foonathan_string_id", "FrameGraph", "Jolt" })
  
  files({ "source/**.h", "source/**.cpp", "source/**.hpp" })

  postbuildcommands { 
    "{COPYDIR} " .. scriptDir .. "/assets %{cfg.targetdir}/assets",
    "{COPYDIR} " .. scriptDir .. "/config %{cfg.targetdir}/config",
    "{COPYDIR} " .. scriptDir .. "/fonts %{cfg.targetdir}/fonts",
    "{COPYDIR} " .. scriptDir .. "/shaders %{cfg.targetdir}/shaders",
    "{COPYFILE} " .. scriptDir .. "/DefaultImGuiSettings.ini %{cfg.targetdir}/DefaultImGuiSettings.ini"
   }

  filter("configurations:Debug*")
   postbuildcommands {
    "{COPYFILE} " .. scriptDir .. "/assimp-vc143-mtd.dll %{cfg.targetdir}/assimp-vc143-mtd.dll",
    "{COPYFILE} " .. scriptDir .. "/fmodstudioL.dll %{cfg.targetdir}/fmodstudioL.dll",
    "{COPYFILE} " .. scriptDir .. "/fmodL.dll %{cfg.targetdir}/fmodL.dll"
  }
  filter("configurations:Release*")
   postbuildcommands {
    "{COPYFILE} " .. scriptDir .. "/assimp-vc143-mt.dll %{cfg.targetdir}/assimp-vc143-mt.dll",
    "{COPYFILE} " .. scriptDir .. "/fmodstudio.dll %{cfg.targetdir}/fmodstudio.dll",
    "{COPYFILE} " .. scriptDir .. "/fmod.dll %{cfg.targetdir}/fmod.dll"
  }
  