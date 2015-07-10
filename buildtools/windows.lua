function doOperatingSystem()
	buildoptions { "/wd4127" } -- conditional expression is constant
	defines { "_SCL_SECURE_NO_WARNINGS" }
	defines {"_WINDOWS", "WINDOWS", "WIN32", "_CRT_SECURE_NO_DEPRECATE", "_CRT_SECURE_NO_WARNINGS", "_MT", "_DLL" }
	-- temporary fix for variadic templates in vs2012
	--defines { "_VARIADIC_MAX=10" }
end
