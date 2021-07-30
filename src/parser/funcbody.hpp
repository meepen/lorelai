#ifndef FUNCBODY_HPP_
#define FUNCBODY_HPP_

#include "node.hpp"
#include "types.hpp"
#include <string>
#include <vector>
#include <memory>

namespace lorelai {
	class lexer;
	namespace parser {
		class visitor;
		class funcbody : public branch {
		public:
			funcbody(lexer &lex);
			funcbody(lexer &lex, optional<string> _method_name) : funcbody(lex) { method_name = _method_name; }
			virtual ~funcbody() { destroy(); }

			void accept(visitor &visit, node *&container) override;
			string tostring() override;

			std::vector<node **> getchildren() override {
				return { &block };
			}

		public:
			std::vector<string> params;
			node *block;
			optional<string> method_name { };
		};
	}
}

#endif // FUNCBODY_HPP_
