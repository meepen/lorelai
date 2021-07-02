newoption {
	trigger     = "with-jit",
	description = "Enables JIT state runtime (if available)"
}

local libs = {
	lexer = {
		includes = {
			dirs = {
				"src"
			},
			files = {
				"src/lexer.hpp"
			}
		}
	},
	parser = {
		includes = {
			dirs = {
				"src",
				"src/parser"
			},
			files = {
				"src/parser/**.hpp"
			}
		},
		links = {
			"lexer",
		}
	},
	asmjit = {
		includes = {
			dirs = {
				"asmjit/src/asmjit",
				"asmjit/src"
			},
			files = {
				"asmjit/src/asmjit/**.h"
			}
		},
		links = {
			["not windows"] = {
				"rt"
			}
		}
	},
	liblorelai = {
		includes = {
			dirs = {
				"src",
				"src/vm"
			},
			files = {
				"src/vm/**.hpp"
			}
		},
		links = {
			"asmjit",
			"parser",
			"protobuf",
			"pthread",
		},
	}
}

local function addincludes(lib)
	if (type(lib) == "table") then
		for _, item in pairs(lib) do
			addincludes(item)
		end
		return
	end

	lib = libs[lib]
	if (not lib) then
		return
	end

	if (lib.includes) then
		if (lib.includes.dirs) then
			includedirs(lib.includes.dirs)
		end
		if (lib.includes.files) then
			files(lib.includes.files)
		end
	end
end

local function addlinks(lib, noincludes)
	if (type(lib) == "table") then
		local added = {}
		for _, item in ipairs(lib) do
			addlinks(item)
			added[item] = true
		end
		for filters, item in pairs(lib) do
			if (not added[item]) then
				filter(filters)
				addlinks(item)
				added[item] = true
				filter {}
			end
		end
		return
	end
	local libt = libs[lib]

	if (libt and libt.linkoptions) then
		for _filter, options in pairs(libt.linkoptions) do
			filter(_filter)
				linkoptions(options)
		end
		filter {}
	end

	links(lib)
	if (not noincludes) then
		addincludes(lib)
	end

	if (libt and libt.links) then
		return addlinks(libt.links, true)
	end
end


workspace "lorelai"
	configurations { "debug", "release" }
	platforms { "x86", "x86-64" }

	cppdialect "C++17"

	targetdir "bin/%{cfg.buildcfg}/%{cfg.platform}"
	location "proj"
	characterset "MBCS"
	warnings "Default"
	symbols "On"

	filter "configurations:debug"
		defines "DEBUG"
		optimize "Off"

	filter {"configurations:debug", "action:gmake" }
		buildoptions { "-fsanitize=address" }
		linkoptions { "-fsanitize=address", "-static-libasan" }

	filter {"configurations:release", "action:gmake" }
		buildoptions { "-fno-omit-frame-pointer" }
		linkoptions { "-fno-omit-frame-pointer" }

	filter "action:vs*"
		defines { "LORELAI_INLINE=__forceinline" }
	filter "action:gmake"
		defines {"LORELAI_INLINE=inline __attribute__((always_inline))"}

	filter "configurations:release"
		defines "NDEBUG"
		optimize "Speed"
		flags { "MultiProcessorCompile", "LinkTimeOptimization" }
		floatingpoint "Fast"
		intrinsics "On"
		runtime "Release"
		vectorextensions "SSE2"


	filter "platforms:x86"
		architecture "x86"
	filter "platforms:x86-64"
		architecture "x86_64"

	project "lexer"
		kind "StaticLib"
		addincludes "lexer"
		files "src/lexer/**.cpp"

	project "parser"
		kind "StaticLib"
		addincludes "parser"
		addlinks "lexer"

		files "src/parser/**.cpp"

	project "asmjit"
		kind "StaticLib"
		addincludes "asmjit"

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
		kind "StaticLib"
		targetprefix "" -- remove lib from name (linux)
		addincludes "liblorelai"
		prebuildcommands {
			"protoc --proto_path=../src/vm/proto/src --cpp_out=../src/vm/proto ../src/vm/proto/src/bytecode.proto"
		}

		pchheader "../src/vm/stdafx.h"
		pchsource "../src/vm/stdafx.cpp"

		links {
			"protobuf",
		}

		addlinks {
			"parser",
			"asmjit"
		}

		files {
			"src/vm/**.cc",
			"src/vm/*.cpp",
			"src/vm/software/**.cpp",
			"src/vm/software/*.cpp",
			"src/vm/bytecode/*.cpp"
		}

		filter { "platforms:x86 or x86-64" }
			configuration "with-jit"
				defines { "LORELAI_X86_FASTEST" }
				files {
					"src/vm/x86/**.cpp"
				}

		filter {"platforms:not x86 and not x86-64"}
			configuration "with-jit"
				defines { "LORELAI_SOFTWARE_FASTEST" }

		filter {}
			configuration "not with-jit"
				defines { "LORELAI_SOFTWARE_FASTEST" }

	project "lorelai"
		kind "ConsoleApp"
		addlinks {
			"liblorelai",
		}
		files "tests/runtime/main.cpp"
		includedirs {
			"tclap/include"
		}
		files {
			"tclap/include/**.h"
		}

	project "test-parser"
		kind "ConsoleApp"
		addlinks "parser"

		includedirs {
			"tests/parser",
		}
		files {
			"tests/parser/main.cpp",
		}

	project "test-visitor"
		kind "ConsoleApp"
		addlinks "parser"

		includedirs {
			"tests/visitor",
		}
		files {
			"tests/visitor/main.cpp",
		}

	filter "platforms:x86 or x86-64"
		project "test-asmjit-x86"
			kind "ConsoleApp"
			addlinks "asmjit"
			files "tests/asmjit/main.cpp"
