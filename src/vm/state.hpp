#ifndef STATE_HPP_
#define STATE_HPP_

namespace lorelai {
	namespace vm {
		class state {
		public:
			virtual const char *backend() = 0;

			static state *create();

		};
	}
}

#endif // STATE_HPP_