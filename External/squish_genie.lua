project ( EXTERNAL_PROJECT_PREFIX .. "squish" )
	kind ( EXTERNAL_PROJECT_KIND )
	language "C++"
	files { "./squish/*.h", "./squish/*.cpp" }
	includedirs { "./squish" }
	-- defines{ "SQUISH_USE_SSE=1" } -- need to check for later version for 64-bit.
