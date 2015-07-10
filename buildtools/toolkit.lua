function addToolkitIncludes(baseFolder)
	baseFolder = baseFolder or ''
	includedirs { baseFolder .. "src/include" }
	if os.is("linux") then
		buildoptions { "-include " .. baseFolder .. "toolkit/common.hh" }
	elseif os.is("windows") then
		buildoptions { "/FI" .. baseFolder .. "toolkit/common.hh" }
	end
end

function useToolkit(baseFolder)
	baseFolder = baseFolder or ''
	addToolkitIncludes(baseFolder)
	links { "lptoolkit" }
	useNetwork()
end

function declareToolkit(baseFolder)
	baseFolder = baseFolder or ''

	project "lptoolkit"
	kind "StaticLib"
	language "C++"
	
	flags { "NoExceptions", "NoRTTI" }
	defines { "NO_EXCEPTIONS", "_HAS_EXCEPTIONS=0" }
	
	addToolkitIncludes(baseFolder)

	files { 
		"src/*.cpp",
		"src/include/**.hh", 
		"src/include/**.inl" 
	}
end

