-- dofile("qthelper.lua")

function addNoExceptions ()
	flags { "NoExceptions", "NoRTTI" }
	defines { "NO_EXCEPTIONS", "_HAS_EXCEPTIONS=0" }
end

function addIncludes ()
	includedirs { "src/include" }
	if os.is("linux") then
		buildoptions { "-include toolkit/common.hh" }
	elseif os.is("windows") then
		buildoptions { "/FItoolkit/common.hh" }
	end
end

function addCpp11Flag()
	if os.is("linux") then
		buildoptions { "-std=c++0x" }
	end
end

solution "Toolkit"
	configurations { "Debug", "Release" }
	
	if os.is("linux") then
		defines {"LINUX"}
	elseif os.is("windows") then 
		buildoptions { "/wd4127" } -- conditional expression is constant
		defines { "_SCL_SECURE_NO_WARNINGS" }
		defines {"_WINDOWS", "WINDOWS", "WIN32", "_CRT_SECURE_NO_DEPRECATE", "_CRT_SECURE_NO_WARNINGS", "_MT", "_DLL" }
		platformtoolset "v110" -- this is a custom thing in my own premake4 build
	end

	platforms { "x64", "x32" }

	configuration "Debug"
		defines { "DEBUG", "_DEBUG", "PROFILE", "GPU_PROFILE" }
		flags { "Symbols" }
		targetsuffix "-d"
	configuration "Profile"
		defines { "NDEBUG", "RELEASE", "PROFILE", "GPU_PROFILE" }
		flags { "Optimize" }
		targetsuffix "-p"
	configuration "Release"
		defines { "NDEBUG", "RELEASE" }
		flags { "Optimize" }
		targetsuffix "-z"
	configuration "Final"
		defines { "NDEBUG", "RELEASE", "FINAL" }
		flags { "Optimize" }
		targetsuffix "-f"

	--
	project "lptoolkit"
		flags { "ExtraWarnings" }
		addNoExceptions()
		addCpp11Flag()
		addIncludes()
		kind "StaticLib"
		language "C++"
		files { 
			"src/*.cpp",
			"src/include/**.hh", 
			"src/include/**.inl" 
		}

	--
	project "kdtree_test"
		flags { "ExtraWarnings" }
		addNoExceptions()
		addCpp11Flag()
		addIncludes()
		defines { "CONSOLE" }
		kind "ConsoleApp"
		language "C++"
		links { "lptoolkit" }
		files { "tests/kdtree/**.hh", "tests/kdtree/**.cpp" }
		files { "src/include/**.hh", "src/include/**.inl" }

	--
	project "timer_test"
		flags { "ExtraWarnings" }
		addNoExceptions()
		addCpp11Flag()
		addIncludes()
		defines { "CONSOLE" }
		kind "ConsoleApp"
		language "C++"
		links { "lptoolkit" }
		files { "tests/timer/**.hh", "tests/timer/**.cpp" }
		files { "src/include/**.hh", "src/include/**.inl" }
		
