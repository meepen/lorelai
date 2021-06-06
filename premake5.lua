local function includeast()
	includedirs {
		"src",
		"src/thirdparty"
	}
	files {
		"src/**.hpp"
	}
end
local function includelexer()
	includedirs {
		"src"
	}
	files {
		"src/lexer.hpp"
	}
end

local function linklexer()
	links "lexer"
	includelexer()
end

local function linkast()
	links "astgen"
	includeast()
	linklexer()
end

workspace "lorelai"
	configurations { "debug", "release" }
	platforms { "x86", "x86-64" }
	cppdialect "C++17"

	targetdir "bin/%{cfg.buildcfg}/%{cfg.platform}"
	location "proj"
	characterset "MBCS"

	filter "configurations:debug"
		defines "DEBUG"
		symbols "On"
		optimize "Debug"
	filter "configurations:release"
		defines "NDEBUG"
		optimize "Full"

	filter "platforms:x86"
		architecture "x86"
	filter "platforms:x86-64"
		architecture "x86_64"

	project "lexer"
		kind "StaticLib"
		includelexer()

		files "src/lexer/**.cpp"

	project "astgen"
		kind "StaticLib"
		includeast()
		linklexer()

		includedirs "src/astgen"
		files "src/astgen/**.cpp"

	project "test-astgen"
		kind "ConsoleApp"
		linkast()

		includedirs {
			"tests/astgen",
		}
		files {
			"tests/astgen/main.cpp",
		}

	project "test-visitor"
		kind "ConsoleApp"
		linkast()

		includedirs {
			"tests/visitor",
		}
		files {
			"tests/visitor/main.cpp",
		}
