project("SampleProject")
  targetname("SampleProjectGameExe")
  kind("WindowedApp")
  location("build/SampleProject")
  language("C++")
  targetdir("bin/%{cfg.buildcfg}")
  objdir("intermediate/%{cfg.buildcfg}")
  debugdir("")
  
  links({ "d3d11", "dxgi", "d3dcompiler", "dxguid", "assimp-vc143-mtd", "DirectXTK", "gtest", "gtest_main" })
  
  files({ "source/**.h", "source/**.cpp", "source/**.hpp" })