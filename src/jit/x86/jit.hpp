#ifndef JIT_HPP_
#define JIT_HPP_

#include "asmjit/asmjit.h"
#include <memory>
#include <cstdint>
#include <exception>
#include <string>

namespace lorelai {
	namespace jit {
		class state {
			struct object;
			using luafunction = object *(*)(object *);

			class exception : public std::exception {
			public:
				exception(asmjit::Error _err) : err(std::string("asmjit error: ") + asmjit::DebugUtils::errorAsString(_err)) { }

				const char *what() {
					return err.c_str();
				}
			private:
				std::string err;
			};

			void check_error(asmjit::Error err) {
				if (err) {
					throw new exception(err);
				}
			}
		public:
#pragma pack(push, 1)
			struct object {
				object *metatable = nullptr;
				object (*__index)(object *) = nullptr;
				std::uint8_t type = 0;
				std::uint8_t flags = 0;
			};
#pragma pack(pop)

			luafunction compile() {
				using namespace asmjit;

				CodeHolder code;
				check_error(code.init(runtime.environment()));
				x86::Compiler compiler(&code);
				FuncSignatureBuilder signature(CallConv::kIdHost);
				signature.setRetT<int>();
				signature.addArgT<std::uint32_t>();
				signature.addArgT<void *>();

				compiler.addFunc(signature);
				compiler.func()->frame().setPreservedFP();
				auto stacksize = compiler.newGpd();
				auto stackptr = compiler.newUIntPtr();
				check_error(compiler.setArg(0, stacksize));
				check_error(compiler.setArg(1, stackptr));
				

				// code here
				compiler.ret(stacksize);


				// end of function
				compiler.endFunc();
				compiler.finalize();

				luafunction out;
				check_error(runtime.add(&out, &code));

				return out;
			}
		

		public:
			asmjit::JitRuntime runtime;
		};

	}
}

#endif // JIT_HPP_