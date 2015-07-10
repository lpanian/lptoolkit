function useNetwork()
	if os.is("windows") then
		links { "ws2_32.lib" }
	end
end
