#include "expressions.hpp"
#include "visitor.hpp"
#include "lexer.hpp"

#include <unordered_map>

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

std::shared_ptr<node> expression::read(lexer &lex) {
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

	if (expr) {
		// binop
	}

	return expr;
}