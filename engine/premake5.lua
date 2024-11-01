project("Engine")
  targetname("Engine")
  kind("ConsoleApp")  -- TODO: replace with SharedLib/StaticLib when scripting prototype is ready
  location("../build/Engine")
  language("C++")
  debugdir("")
  
  includedirs({ "source" })
  includedirs({ "libs/**/include" })
  includedirs({ "source/imgui" })
  
  filter("configurations:Debug*")
    libdirs({ "libs/**/debug" })
	links({ "assimp-vc143-mtd", "freetyped", "argumentumd" })
  
  filter("configurations:Release*")
    libdirs({ "libs/**/release" })
	links({ "assimp-vc143-mt", "freetype", "argumentum" })
  
  filter {  }
  
  links({ "d3d11", "dxgi", "d3dcompiler", "dxguid", "DirectXTK", "foonathan_string_id" })
  
  files({ "source/**.h", "source/**.cpp", "source/**.hpp" })
  