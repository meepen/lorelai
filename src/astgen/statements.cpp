#include "statements.hpp"
#include "expressions.hpp"
#include "errors.hpp"
#include "visitor.hpp"
#include <algorithm>

using namespace lorelai;
using namespace lorelai::astgen;

statements::returnstatement::returnstatement(lexer &lex) {
	// consume return
	lex.read();

	// try to read explist
	while (std::shared_ptr<expression> exp = expression::read(lex)) {
		children.push_back(exp);
		if (lex.lookahead().value_or("") != ",") {
			break;
		}
	}
}

std::shared_ptr<node> statement::read(lexer &lex) {
	/*
	stat ::=  varlist `=´ explist | 
		 functioncall | 
		 do block end | 
		 while exp do block end | 
		 repeat block until exp | 
		 if exp then block {elseif exp then block} [else block] end | 
		 for Name `=´ exp `,´ exp [`,´ exp] do block end | 
		 for namelist in explist do block end | 
		 function funcname funcbody | 
		 local function Name funcbody | 
		 local namelist [`=´ explist] 

	+ optional ;
	*/
	auto word_or_none = lex.lookahead();
	if (!word_or_none) {
		return nullptr;
	}
	auto word = word_or_none.value();

	std::shared_ptr<node> stmt;

	if (word == "return") {
		stmt = std::make_shared<statements::returnstatement>(lex);
	}

	if (stmt && lex.lookahead().value_or("") == ";") {
		lex.read();
	}

	return stmt;
}


// visitor acceptors

bool statements::returnstatement::accept(visitor &visit, std::shared_ptr<node> &container) {
	bool ret = visit.visit(*this, container);
	if (ret) { // if we delete who cares, return early
		return true;
	}

	std::vector<std::shared_ptr<node>> deleted;

	for (auto &child : children) {
		if (child->accept(visit, child)) {
			deleted.push_back(child);
		}
	}

	for (auto &del : deleted) {
		children.erase(std::find(children.cbegin(), children.cend(), del));
	}

	return false;
}