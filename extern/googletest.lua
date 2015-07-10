function declareGtest()
	project "googletest"
	kind "StaticLib"
	language "C++"
	files { 
		"extern/googletest/src/gtest-all.cc", 
		"extern/googletest/include/gtest/**.h", 
		"extern/googletest/include/gtest/**.pump" 
	}
	includedirs { 
		"extern/googletest/include", 
		"extern/googletest" 
	}
	if os.is("windows") then
		-- disable stupid MS warnings about exceptions...
		buildoptions { "/wd4275" }
	end
end

function useGtest()
	includedirs { 
		"extern/googletest/include", 
		"extern/googletest" 
	}
	links { "googletest" }
end
