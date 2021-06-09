#include "statements.hpp"
#include "expressions.hpp"
#include "visitor.hpp"
#include "lexer.hpp"
#include <algorithm>
#include <unordered_map>

using namespace lorelai;
using namespace lorelai::astgen;
using namespace lorelai::astgen::statements;


const static std::unordered_map<string, std::shared_ptr<node>(*)(lexer &lex)> lookupmap = {
	{ "return", [](lexer &lex) -> std::shared_ptr<node> { return std::make_shared<returnstatement>(lex); } },
	{ "while", [](lexer &lex) -> std::shared_ptr<node> { return std::make_shared<whilestatement>(lex); } },
	{ "do", [](lexer &lex) -> std::shared_ptr<node> { return std::make_shared<dostatement>(lex); } },
	{ "repeat", [](lexer &lex) -> std::shared_ptr<node> { return std::make_shared<repeatstatement>(lex); } },
	{ "local", [](lexer &lex) -> std::shared_ptr<node> {
		lex.expect("local", "local deducer");
		auto ahead = lex.lookahead();
		if (ahead) {
			if (ahead.value() == "function") {
				return std::make_shared<localfunctionstatement>(lex);
			}
			else {
				return std::make_shared<localsstatement>(lex);
			}
		}

		return nullptr;
	} },
	{ "for", [](lexer &lex) -> std::shared_ptr<node> {
		lex.expect("for", "for deducer");

		auto name = std::make_shared<expressions::nameexpression>(lex);
		if (!lex.lookahead()) {
			lex.wasexpected("<token>", "for deducer");
		}
		else if (lex.lookahead().value() == "=") {
			return std::make_shared<fornumstatement>(name, lex);
		}
		else {
			return std::make_shared<forinstatement>(name, lex);
		}
	} },
	{ "break", [](lexer &lex) -> std::shared_ptr<node> { return std::make_shared<breakstatement>(lex); } },
	{ "if", [](lexer &lex) -> std::shared_ptr<node> { return std::make_shared<ifstatement>(lex); } },
	{ "function", [](lexer &lex) -> std::shared_ptr<node> { return std::make_shared<functionstatement>(lex); } }
	/* { "function", [](lexer &lex) -> std::shared_ptr<node> {
		return std::make_shared<functionstatement>(lex);
	} } */
};

returnstatement::returnstatement(lexer &lex) {
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

std::shared_ptr<node> statement::read(lexer &lex) {
	/*
	stat ::=  varlist `=´ explist | 
		 functioncall | 
!		 do block end | 
!		 while exp do block end | 
!		 repeat block until exp | 
!		 if exp then block {elseif exp then block} [else block] end | 
!		 for Name `=´ exp `,´ exp [`,´ exp] do block end | 
!		 for namelist in explist do block end | 
!		 function funcname funcbody | 
!		 local function Name funcbody | 
!		 local namelist [`=´ explist] 

	+ optional ;
	*/
	auto word_or_none = lex.lookahead();
	if (!word_or_none) {
		return nullptr;
	}
	auto word = word_or_none.value();

	std::shared_ptr<node> stmt;

	auto lookup = lookupmap.find(word);
	if (lookup != lookupmap.end()) {
		stmt = lookup->second(lex);
	}

	if (!stmt) {
		// varlist = exprlist | functioncall
		auto exp = expression::read(lex);
		if (exp) {
			if (!lex.lookahead()) {
				lex.wasexpected("<token>", "statement deducer");
			}

			if (dynamic_cast<expressions::functioncallexpression *>(exp.get())) {
				stmt = std::make_shared<functioncallstatement>(exp);
			}
			else {
				stmt = std::make_shared<assignmentstatement>(exp, lex);
			}
		}
	}

	if (stmt && lex.lookahead().value_or("") == ";") {
		lex.read();
	}

	return stmt;
}


#define LORELAI_ACCEPTOR(name) \
bool name ::accept(visitor &visit, std::shared_ptr<node> &container) { \
	if (visit.visit(*this, container)) { \
		return true; \
	} \
	\
	visitchildren(visit); \
	return false; \
}

LORELAI_STATEMENT_BRANCH_CLASS_MACRO(LORELAI_ACCEPTOR)