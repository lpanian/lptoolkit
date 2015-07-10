
if os.is("linux") then
	dofile("buildtools/linux.lua")
elseif os.is("windows") then
	dofile("buildtools/windows.lua")
end

dofile("buildtools/toolkit.lua")
dofile("buildtools/network.lua")
dofile("extern/googletest.lua")

function declareSimpleTest(name, projFiles)
	project(name)
	flags { "NoExceptions", "NoRTTI" }
	defines { "NO_EXCEPTIONS", "_HAS_EXCEPTIONS=0" }
	defines { "CONSOLE" }
	kind "ConsoleApp"
	language "C++"
	useToolkit()
	files(projFiles)
end

solution "Toolkit"
	configurations { "Debug", "Release" }
	platforms { "x64", "x32" }
	
	flags { "ExtraWarnings" }
	warnings "extra"
        targetdir "bin"

	doOperatingSystem()

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

	declareToolkit()
	
	declareGtest()

	declareSimpleTest("kdtree_test", 
	{ "tests/kdtree/**.hh", "tests/kdtree/**.cpp", })
	
	declareSimpleTest("timer_test",  
	{ "tests/timer/**.hh", "tests/timer/**.cpp", })
	
	declareSimpleTest("statistics_test",  
	{ "tests/statistics/**.hh", "tests/statistics/**.cpp", })
	
	declareSimpleTest("msg_client",  
	{ "tests/network/**.hh", "tests/network/msg_client.cpp", })
	
	declareSimpleTest("msg_server",  
	{ "tests/network/**.hh", "tests/network/msg_server.cpp", })
	
	declareSimpleTest("test_client",  
	{ "tests/network/**.hh", "tests/network/test_client.cpp", })
	
	declareSimpleTest("test_server",  
	{ "tests/network/**.hh", "tests/network/test_server.cpp", })

	declareSimpleTest("unit_tests", 
	{ "tests/unit/**.hh", "tests/unit/**.cpp", })
	useGtest()

