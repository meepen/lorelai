local function includelexer()
	includedirs {
		"src"
	}
	files {
		"src/lexer.hpp"
	}
end
local function includeast()
	includedirs {
		"src",
		"src/thirdparty"
	}
	files {
		"src/**.hpp"
	}
end
local function includeasmjit()
	includedirs {
		"asmjit/src/asmjit",
		"asmjit/src"
	}
	files {
		"asmjit/src/asmjit/**.h"
	}
end

local function includelorelai()
	includeasmjit()
	includeast()
	includedirs {
		"src/vm"
	}
	files {
		"src/vm/**.hpp"
	}
end


local function linklexer()
	links "lexer"
	includelexer()
end

local function linkast()
	links "parser"
	includeast()
	linklexer()
end

local function linkasmjit()
	filter "system:linux"
		links {
			"pthread",
			"rt",
			"asmjit"
		}
	includeasmjit()
end

local function linklorelai()
	includelorelai()
	linkasmjit()
	linkast()
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

	project "parser"
		kind "StaticLib"
		includeast()
		linklexer()

		includedirs "src/parser"
		files "src/parser/**.cpp"

	project "test-parser"
		kind "ConsoleApp"
		linkast()

		includedirs {
			"tests/parser",
		}
		files {
			"tests/parser/main.cpp",
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

	project "asmjit"
		kind "StaticLib"
		includeasmjit()
		defines {
			"ASMJIT_STATIC",
			"ASMJIT_TARGET_TYPE=\"STATIC\""
		}
		filter "platforms:x86 or x86-64"
			files {
				"asmjit/src/asmjit/x86/**.cpp",
				"asmjit/src/asmjit/core/**.cpp"
			}

	project "liblorelai"
		kind "SharedLib"
		includelorelai()
		prebuildcommands {
			"protoc --proto_path=../src/vm/proto/src --cpp_out=../src/vm/proto ../src/vm/proto/src/bytecode.proto"
		}

		files {
			"src/jit/vm/**.cc",
			"src/jit/vm/*.cpp"
		}

		filter "platforms:x86 or x86-64"
			files {
				"src/jit/vm/x86/**.cpp"
			}
		filter "platforms:not x86 and not x86-64"
			files {
				"src/jit/software/**.cpp"
			}
		
	project "lorelai"
		kind "ConsoleApp"
		linklorelai()
		files "tests/jit/main.cpp"

	filter "platforms:x86 or x86-64"
		project "test-asmjit-x86"
			kind "ConsoleApp"
			linkasmjit()
			files "tests/asmjit/main.cpp"

			