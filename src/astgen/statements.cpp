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
	while (std::shared_ptr<node> exp = expression::read(lex)) {
		children.push_back(exp);
		auto next = lex.lookahead().value_or("");
		if (next != ",") {
			break;
		}
		// consume ','
		lex.read();
	}
}

statements::dostatement::dostatement(lexer &lex) {
	// consume do
	lex.read();

	// try to read statement list
	while (lex.lookahead() && lex.lookahead().value() != "end") {
		auto stmt = statement::read(lex);
		if (!stmt) {
			throw error::expected_for("statement", "do ... end", lex.lookahead().value_or("no value"));
		}

		children.push_back(stmt);
	}

	auto ensure_end = lex.read();

	if (ensure_end != "end") {
		throw error::expected_for("end", "do .. end", lex.lookahead().value_or("no value"));
	}
}

std::shared_ptr<node> statement::read(lexer &lex) {
	/*
	stat ::=  varlist `=´ explist | 
		 functioncall | 
!		 do block end | 
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
	else if (word == "do") {
		stmt = std::make_shared<statements::dostatement>(lex);
	}

	if (stmt && lex.lookahead().value_or("") == ";") {
		lex.read();
	}

	return stmt;
}


// visitor acceptors

bool statements::returnstatement::accept(visitor &visit, std::shared_ptr<node> &container) {
	if (visit.visit(*this, container)) { // if we delete who cares, return early
		return true;
	}

	visitchildren(visit);
	return false;
}


bool statements::dostatement::accept(visitor &visit, std::shared_ptr<node> &container) {
	if (visit.visit(*this, container)) { // if we delete who cares, return early
		return true;
	}

	visitchildren(visit);
	return false;
}