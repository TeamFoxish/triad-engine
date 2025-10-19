beginModule()

project("Engine")
  targetname("Engine")
  kind("ConsoleApp")  -- TODO: replace with SharedLib/StaticLib when scripting prototype is ready. 
                      -- Seekerses comment: maybe not. Engine actually .exe that starts and compiles scripts. Working like framework
  location("../build/Engine")
  language("C++")
  debugdir("")

  -- local libsPath = "vcpkg_installed/" .. archTable["%{cfg.architecture}"] .. "-%{cfg.system}/"
  -- local arch = "%{cfg.architecture}"
  local arch = "x64" -- %{cfg.architecture} doesn't get resolved during premake execution and has x86_64 as default for x64
  local sys = "%{cfg.system}"
  local libsPath = string.format("vcpkg_installed/%s-%s/", arch, sys)
  
  includedirs({ "source" })
    includedirs({ libsPath .. "include" })
  
  filter("configurations:Debug*")
    libdirs({ libsPath .. "debug/lib" })
    links({"fmtd"})
  
  filter("configurations:Release*")
    libdirs({ libsPath .. "lib" })
    links({"fmt"})
  
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

