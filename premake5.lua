

workspace("Quantizer")
	-- Configuration.
	configurations({ "Release", "Dev"})
	location("build")
	targetdir ("build/%{prj.name}/%{cfg.longname}")
	debugdir ("build/%{prj.name}/%{cfg.longname}")
	architecture("x64")

	systemversion("latest")

	-- Configuration specific settings.
	filter("configurations:Release")
		defines({ "NDEBUG" })
		optimize("On")

	filter("configurations:Dev")
		defines({ "DEBUG" })
		symbols("On")

	filter({})
	startproject("Quantizer")

	project("imagequant")
		kind("StaticLib")
		
		-- common files
		files({"libs/imagequant/*.*" })
		language("C")
		
		-- visual studio filters
		filter("action:vs*")
			defines({ "_CRT_SECURE_NO_WARNINGS" })    

		filter({})

	project("posterizer")
		kind("StaticLib")
		
		-- common files
		files({"libs/posterizer/*.*" })
		language("C")
		
		-- visual studio filters
		filter("action:vs*")
			defines({ "_CRT_SECURE_NO_WARNINGS" })    

		filter({})

	project("pngnq")
		kind("StaticLib")
		
		-- common files
		files({"libs/pngnq/*.*" })
		language("C")
		
		-- visual studio filters
		filter("action:vs*")
			defines({ "_CRT_SECURE_NO_WARNINGS" })    

		filter({})


	project("Quantizer")
		kind("ConsoleApp")

		language("C++")
		cppdialect("C++17")
		
		-- Compiler flags
		filter("toolset:not msc*")
			buildoptions({ "-Wall", "-Wextra" })
		filter("toolset:msc*")
			buildoptions({ "-W3"})
		filter({})
		-- Common include dirs
		-- System headers are used to support angled brackets in Xcode.
		includedirs({"src/"})
		sysincludedirs({ "libs/" })

		-- common files
		files({"src/**", "premake5.lua"})
		removefiles({"**.DS_STORE", "**.thumbs"})

		links({"imagequant", "posterizer", "pngnq"})
		
		-- visual studio filters
		filter("action:vs*")
			defines({ "_CRT_SECURE_NO_WARNINGS" })  
		filter({})



newaction {
   trigger     = "clean",
   description = "Clean the build directory",
   execute     = function ()
      print("Cleaning...")
      os.rmdir("./build")
      print("Done.")
   end
}
