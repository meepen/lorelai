#include <iostream>
#include <string>
#include <exception>
#include "asmjit/asmjit.h"

using namespace asmjit;

typedef int (*Func)(int *);

class asmjit_exception : public std::exception {
public:
	asmjit_exception(Error _err) : err(std::string("asmjit error: ") + DebugUtils::errorAsString(_err)) { }

	const char *what() {
		return err.c_str();
	}
private:
	std::string err;
};

void check_error(Error err) throw() {
	if (err) {
		throw asmjit_exception(err);
	}
}

int main(int argc, char* argv[]) {
	JitRuntime rt;
	Func fn;
	try {
		CodeHolder code;
		Error err;
		code.init(rt.environment());
		x86::Compiler compiler(&code);

		FuncSignatureBuilder signature(CallConv::kIdHost);
		signature.setRetT<int>();
		signature.addArgT<int *>();

		compiler.addFunc(signature);
		compiler.func()->frame().setPreservedFP();

		auto reg = compiler.newIntPtr();
		check_error(compiler.setArg(0, reg));
		compiler.mov(reg, ptr(reg));
		compiler.add(reg, reg);
		compiler.ret(reg);
		compiler.endFunc();
		compiler.finalize();

		// `compiler` can be deleted

		check_error(rt.add(&fn, &code));
		
		// `code` can die now
		// can also use code.reset() to reuse
	}
	catch (std::exception &e) {
		std::cerr << e.what() << std::endl;
		return 1;
	}

	int arg = 4;
	int result = fn(&arg);
	std::cout << std::to_string(result) << std::endl;

	// you do not need to call this as long as JitRuntime is destructed properly
	rt.release(fn);

	return 0;
}