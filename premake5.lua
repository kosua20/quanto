-- On Linux We have to query the dependencies of gtk+3 for sr_gui, we do this on the host for now.
if os.ishost("linux") then
	listing, code = os.outputof("pkg-config --libs libnotify gtk+-3.0")
	liballLibs = string.explode(string.gsub(listing, "-l", ""), " ")
end


workspace("Quanto")
	
	-- Configuration.
	configurations({ "Release", "Dev"})
	location("build")
	targetdir ("build/%{prj.name}/%{cfg.longname}")
	debugdir ("build/%{prj.name}/%{cfg.longname}")
	architecture("x64")
	systemversion("latest")

	filter("system:macosx")
		systemversion("10.12:latest")
	filter({})

	-- Configuration specific settings.
	filter("configurations:Release")
		defines({ "NDEBUG" })
		optimize("On")

	filter("configurations:Dev")
		defines({ "DEBUG" })
		symbols("On")

	filter({})
	startproject("Quanto")

-- Projects

function CLibrarySetup(path)
	
	kind("StaticLib")
	language("C")

	files({ path })

	filter("action:vs*")
			defines({ "_CRT_SECURE_NO_WARNINGS" }) 
	filter({}) 

end	

function CommonFlags()
	language("C++")
	cppdialect("C++17")
	
	-- Compiler flags
	filter("toolset:not msc*")
		buildoptions({ "-Wall", "-Wextra" })
	filter("toolset:msc*")
		buildoptions({ "-W3"})
	filter({})

	-- visual studio filters
	filter("action:vs*")
		defines({ "_CRT_SECURE_NO_WARNINGS" })  
	filter({})

end


group("Libs")
	
project("imagequant")
	CLibrarySetup("libs/imagequant/*.*")

project("posterizer")
	CLibrarySetup("libs/posterizer/*.*")

project("pngnq")
	CLibrarySetup("libs/pngnq/*.*")

-- Include sr_gui and GLFW premake files.
include("libs/sr_gui/premake5.lua")
include("libs/glfw/premake5.lua")

group("Quanto")

project("QuantoTool")
	
	kind("ConsoleApp")
	CommonFlags()

	includedirs({"src/"})
	externalincludedirs({ "libs/" })

	-- common files
	files({"src/core/**", "src/libs/stb_image/**", "src/libs/lodepng/**", "src/tool/**", "premake5.lua"})
	removefiles({"**.DS_STORE", "**.thumbs"})

	links({"imagequant", "posterizer", "pngnq"})
	


project("Quanto")
	
	kind("WindowedApp")
	CommonFlags()

	includedirs({"src/"})
	externalincludedirs({ "libs/", "libs/glfw/include/" })

	-- common files
	files({"src/core/**", "src/libs/**", "src/app/**", "premake5.lua"})
	removefiles({"**.DS_STORE", "**.thumbs"})

	-- per platform files
	filter("action:vs*")
		files({"resources/windows/*"})

	filter("action:xcode*")
		files({"resources/macos/*"})
		
   filter({})
	
	links({"imagequant", "posterizer", "pngnq", "sr_gui", "glfw3"})

	-- Libraries for each platform.
	filter("system:macosx")
		links({"OpenGL.framework", "Cocoa.framework", "IOKit.framework", "CoreVideo.framework", "AppKit.framework"})

	filter("system:windows")
		links({"opengl32", "User32", "Comdlg32", "Comctl32", "Shell32", "runtimeobject"})

	filter("system:linux")
		links({"GL", "X11", "Xi", "Xrandr", "Xxf86vm", "Xinerama", "Xcursor", "Xext", "Xrender", "Xfixes", "xcb", "Xau", "Xdmcp", "rt", "m", "pthread", "dl", liballLibs})
	
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
