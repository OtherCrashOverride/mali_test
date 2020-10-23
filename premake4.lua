#!lua
local output = "./build/" .. _ACTION

solution "mali_test"
   configurations { "Debug", "Release" }


project "test_vsync"
   location (output)
   kind "ConsoleApp"
   language "C++"
   includedirs { "." }
   files { "test_vsync.cpp", "X11Window.cpp", "Egl.cpp" }
   buildoptions { "" }
   linkoptions { "" }
   links { "Mali" }

   configuration "Debug"
      flags { "Symbols" }
      defines { "DEBUG" }

   configuration "Release"
      flags { "Optimize" }
      defines { "NDEBUG" }


project "eglinfo"
    location (output)
    kind "ConsoleApp"
    language "C"
    includedirs { "." }
    files { "eglinfo.c" }
    buildoptions { "" }
    linkoptions { "" }
    links { "Mali" }

    configuration "Debug"
        flags { "Symbols" }
        defines { "DEBUG" }

    configuration "Release"
        flags { "Optimize" }
        defines { "NDEBUG" }

project "test_pan"
    location (output)
    kind "ConsoleApp"
    language "C++"
    includedirs { "." }
    files { "test_pan.cpp" }
    buildoptions { "" }
    linkoptions { "" }
    -- links { "" }

    configuration "Debug"
        flags { "Symbols" }
        defines { "DEBUG" }

    configuration "Release"
        flags { "Optimize" }
        defines { "NDEBUG" }
