#ifndef X86STATE_HPP_
#define X86STATE_HPP_

#include "state.hpp"
#include "asmjit/asmjit.h"
#include <memory>
#include <cstdint>
#include <exception>
#include <string>
#include <vector>

#define LORELAI_OFFSETOF(T, key) reinterpret_cast<std::uintptr_t>(&((T *)0)->key)

namespace lorelai {
	namespace vm {
		struct object;

		class exception : public std::exception {
		public:
			exception(asmjit::Error _err) : err(std::string("asmjit error: ") + asmjit::DebugUtils::errorAsString(_err)) { }

			const char *what() {
				return err.c_str();
			}
		private:
			std::string err;
		};

		class x86state : public state {
			struct asmdata {
				x86state *state;
				object **stack;
				std::uint32_t stacksize;
			};

			static void check_error(asmjit::Error err) {
				if (err) {
					throw new exception(err);
				}
			}
		public:
			const char *backend() const override { return "x86 JIT"; }
			void loadstring(const std::string &code) override;

			void *compile() {
				using namespace asmjit;

				CodeHolder code;
				check_error(code.init(runtime.environment()));
				x86::Compiler compiler(&code);
				FuncSignatureBuilder signature(CallConv::kIdHost);
				signature.setRetT<int>();
				signature.addArgT<void *>();

				compiler.addFunc(signature);
				compiler.func()->frame().setPreservedFP();

				auto state_ptr = compiler.is64Bit() ? compiler.newGpq() : compiler.newGpd();
				compiler.setArg(0, state_ptr);

				compiler.mov(x86::dword_ptr(state_ptr, LORELAI_OFFSETOF(asmdata, stacksize)), 2);

				// code here
				auto retn = compiler.newGpd();
				compiler.mov(retn, 23);


				compiler.ret(retn);


				// end of function
				check_error(compiler.endFunc());
				check_error(compiler.finalize());

				void *out;
				check_error(runtime.add(&out, &code));

				return out;
			}

			operator asmdata *() {
				return &data;
			}

		public:
			asmjit::JitRuntime runtime;

			asmdata data = {this};
		};

	}
}

#endif // X86STATE_HPP_