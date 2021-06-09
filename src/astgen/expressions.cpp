#include "expressions.hpp"
#include "visitor.hpp"
#include "lexer.hpp"

#include <unordered_set>
#include <algorithm>

using namespace lorelai;
using namespace lorelai::astgen;
using namespace lorelai::astgen::expressions;

#define acceptmacro(name) bool name ::accept(visitor &visit, std::shared_ptr<node> &container) { visit.visit(*this, container); }
LORELAI_EXPRESSION_NODES_CLASS_MACRO(acceptmacro);

bool enclosedexpression::accept(visitor &visit, std::shared_ptr<node> &container) {
	if (visit.visit(*this, container)) {
		return true;
	}

	visitchildren(visit);

	return false;
}

const static std::unordered_map<string, std::shared_ptr<node>(*)(lexer &lex)> lookupmap = {
	{ "false", [](lexer &lex) -> std::shared_ptr<node> { return std::make_shared<falseexpression>(lex); } },
	{ "true", [](lexer &lex) -> std::shared_ptr<node> { return std::make_shared<trueexpression>(lex); } },
	{ "nil", [](lexer &lex) -> std::shared_ptr<node> { return std::make_shared<nilexpression>(lex); } },
	{ "...", [](lexer &lex) -> std::shared_ptr<node> { return std::make_shared<varargexpression>(lex); } },
	{ "\"", [](lexer &lex) -> std::shared_ptr<node> { return std::make_shared<stringexpression>(lex); } },
	{ "[", [](lexer &lex) -> std::shared_ptr<node> { return std::make_shared<stringexpression>(lex); } },
	{ "'", [](lexer &lex) -> std::shared_ptr<node> { return std::make_shared<stringexpression>(lex); } },
	{ "{", [](lexer &lex) -> std::shared_ptr<node> { return std::make_shared<tableexpression>(lex); } },
	{ "(", [](lexer &lex) -> std::shared_ptr<node> { return std::make_shared<enclosedexpression>(lex); } }
};


/*
Operator precedence in Lua follows the table below, from lower to higher priority:

     or
     and
     <     >     <=    >=    ~=    ==
     ..
     +     -
     *     /     %
     not   #     - (unary)
     ^

As usual, you can use parentheses to change the precedences of an expression.
The concatenation ('..') and exponentiation ('^') operators are right associative.
All other binary operators are left associative. 
*/

const static std::unordered_set<string> unops = {
	"-",
	"not",
	"#"
};

static std::shared_ptr<node> readone(lexer &lex) {
	std::shared_ptr<node> expr;

	if (!lex.lookahead()) {
		return expr;
	}

	auto word = lex.lookahead().value();

	auto has_initializer = lookupmap.find(word);
	if (has_initializer != lookupmap.end()) {
		expr = has_initializer->second(lex);
	}
	else if (word.size() > 0 && lexer::isnumberstart(word[0])) {
		expr = std::make_shared<numberexpression>(lex);
	}
	else if (lexer::isname(word)) {
		expr = std::make_shared<nameexpression>(lex);
	}

	return expr;
}


struct liststruct {
	std::vector<string> unop = {};
	std::shared_ptr<node> expr = nullptr;
	string binop = "";
};

static bool tryprocess(liststruct &lhs, liststruct &rhs, const std::vector<string> &list) {
	if (list.size() == 0 && lhs.unop.size() > 0) {
		std::shared_ptr<node> expr = lhs.expr;
		for (auto cur = lhs.unop.rbegin(); cur != lhs.unop.rend(); cur++) {
			expr = std::make_shared<unopexpression>(*cur, expr);
		}

		lhs.expr = expr;
		lhs.unop.clear();
	}
	else if (&rhs != &lhs && lhs.binop[0] != 0 && std::find(list.begin(), list.end(), lhs.binop) != list.end()) {
		lhs.expr = std::make_shared<binopexpression>(lhs.expr, lhs.binop, rhs.expr);
		lhs.binop = rhs.binop;

		return true;
	}

	return false;
}

template <class T>
struct linkedlist {
	~linkedlist() {
		if (before) {
			before->after = after;
		}
		if (after) {
			after->before = before;
		}

		if (*begin == this) {
			*begin = after;
		}
		if (*end == this) {
			*end = before;
		}
	}

	linkedlist(linkedlist<T> **_begin, linkedlist<T> **_end) : begin(_begin), end(_end) {
		if (!*begin) {
			*begin = this;
			*end = this;
		}
		else {
			(*end)->insertafter(this);
		}
	}

	void insertafter(linkedlist<T> *_after) {
		if (after) {
			after->before = _after;
		}

		_after->before = this;
		_after->after = after;
		after = _after;

		if (*end == this) {
			*end = _after;
		}
	}

	void insertbefore(linkedlist<T> *_before) {
		if (before) {
			before->after = _before;
		}

		_before->before = before;
		_before->after = this;
		before = _before;

		if (*begin == this) {
			*begin = _before;
		}
	}

	T *operator->() {
		return &current;
	}
	
	T current;

	linkedlist<T> *before = nullptr;
	linkedlist<T> *after = nullptr;


	linkedlist<T> **begin, **end;
};

std::shared_ptr<node> expression::read(lexer &lex, bool postexp) {
	/*
	exp ::=  nil | false | true | Number | String | `...´ | function | 
		 prefixexp | tableconstructor | exp binop exp | unop exp 

	prefixexp ::= var | functioncall | `(´ exp `)´

	functioncall ::=  prefixexp args | prefixexp `:´ Name args 

	args ::=  `(´ [explist] `)´ | tableconstructor | String 

	function ::= function funcbody

	funcbody ::= `(´ [parlist] `)´ block end

	parlist ::= namelist [`,´ `...´] | `...´

	tableconstructor ::= `{´ [fieldlist] `}´

	fieldlist ::= field {fieldsep field} [fieldsep]

	field ::= `[´ exp `]´ `=´ exp | Name `=´ exp | exp

	fieldsep ::= `,´ | `;´

	binop ::= `+´ | `-´ | `*´ | `/´ | `^´ | `%´ | `..´ | 
		 `<´ | `<=´ | `>´ | `>=´ | `==´ | `~=´ | 
		 and | or

	unop ::= `-´ | not | `#´
	*/

	if (!postexp) {
		return readone(lex);
	}

	// i tried using std::list and gave up, fuck stl iterators
	linkedlist<liststruct> *begin = nullptr;
	linkedlist<liststruct> *end = nullptr;

	auto word = lex.lookahead().value_or("");

	while (true) {
		auto next = new linkedlist<liststruct>(&begin, &end);
		auto &current = *next;

		while (unops.count(word) > 0) {
			word = lex.read();

			current->unop.push_back(word);

			word = lex.lookahead().value_or("");
		}

		current->expr = readone(lex);

		if (!current->expr) {
			if (begin != next || current->unop.size() > 0) {
				lex.wasexpected("<expression>", "complex expression parser");
			}
			break;
		}

		if (!lex.lookahead()) {
			break;
		}

		word = lex.lookahead().value();

		if (binopexpression::prioritymap.count(word) == 0) {
			break;
		}

		current->binop = lex.read();

		word = lex.lookahead().value_or("");
	}

	if (!begin) {
		return nullptr;
	}

	for (auto &layer : binopexpression::priorities) {
		if (begin == end && begin->current.unop.size() == 0) {
			break;
		}

		if (layer.first) { // right to left
			// rtl has no unop so we don't CARE
			auto rhs = end;
			auto lhs = end->before;
			while (lhs) {
				while (rhs && tryprocess(lhs->current, rhs->current, layer.second)) {
					delete rhs;
					rhs = lhs->after;
				}
				rhs = lhs;
				lhs = lhs->before;
			}
		}
		else {
			auto lhs = begin;

			do { // left to right
				auto rhs = lhs->after;
				rhs = lhs->after;
				// left to right needs to process unops
				// so we need to pass when rhs is nullptr
				
				while (tryprocess(lhs->current, rhs ? rhs->current : lhs->current, layer.second)) {
					delete rhs;
					rhs = lhs->after;
				}

				lhs = rhs;
			}
			while (lhs);
		}
	}

	if (begin != end) {
		throw;
	}

	auto expr = begin->current.expr;
	delete begin;
	
	return expr;
}