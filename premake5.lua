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
			"protobuf"
		}
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

	links(lib)
	if (not noincludes) then
		addincludes(lib)
	end

	lib = libs[lib]
	if (lib and lib.links) then
		return addlinks(lib.links, true)
	end
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
		}

		filter "platforms:x86 or x86-64"
			files {
				"src/vm/x86/**.cpp"
			}
		filter "platforms:not x86 and not x86-64"
			files {
				"src/vm/software/**.cpp"
			}

	project "lorelai"
		kind "ConsoleApp"
		addlinks {
			"liblorelai",
		}
		files "tests/runtime/main.cpp"

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
