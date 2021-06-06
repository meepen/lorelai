#include "expressions.hpp"
#include "visitor.hpp"
#include "errors.hpp"

using namespace lorelai::astgen;
using namespace lorelai::astgen::expressions;

#define acceptmacro(name) bool name ::accept(visitor &visit, std::shared_ptr<node> &container) { visit.visit(*this, container); }
LORELAI_EXPRESSION_LITERAL_CLASS_MACRO(acceptmacro);

numberexpression::numberexpression(lexer &lex) {
	size_t size;
	auto word = lex.read();
	if (word.size() >= 2 && word[0] == '0') {
		char typ = word[1];
		if (typ == 'x') {
			char *endptr;
			data = std::strtod(word.c_str(), &endptr);
			if ((endptr - word.c_str()) != word.size()) {
				throw error::expected_for("<number>", "number", word.substr(endptr - word.c_str()));
			}
			size = word.size();
		}
		else if (typ == 'b') {
			data = static_cast<number>(std::stol(word.substr(2), &size, 2));
			size += 2;
		}
		else {
			throw error::expected_for("<number>", "number", word);
		}
	}
	else {
		data = std::stod(word, &size);
	}

	if (size != word.size()) {
		throw error::expected_for("<number>", "number", word);
	}
}

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

	auto word_or_null = lex.lookahead();
	if (!word_or_null) {
		throw error::expected_for("value", "expression", "no value");
	}

	auto word = word_or_null.value();

	std::shared_ptr<node> expr;

	if (word == "false") {
		expr = std::make_shared<falseexpression>(lex);
	}
	else if (word == "true") {
		expr = std::make_shared<trueexpression>(lex);
	}
	else if (word == "nil") {
		expr = std::make_shared<nilexpression>(lex);
	}
	else if (word == "...") {
		expr = std::make_shared<varargexpression>(lex);
	}
	else if (word == "'" || word == "\"" || word == "[") {
		expr = std::make_shared<stringexpression>(lex);
	}
	else if (lexer::isnumberstart(word[0]) && word != ".") {
		expr = std::make_shared<numberexpression>(lex);
	}

	return expr;
}