#include "object.hpp"

namespace lorelai {
	namespace vm {
		struct library {
			const char *name = nullptr;
			luafunction func = nullptr;
		};
		struct namedlibrary {
			const char *name;
			library *lib;
		};

		extern library global[];

		static namedlibrary libraries[] = {
			{ nullptr, global },
		};
	}
}