#include "statements.hpp"
#include "expressions.hpp"
#include "visitor.hpp"
#include "lexer.hpp"
#include <algorithm>
#include <unordered_map>

using namespace lorelai;
using namespace lorelai::parser;
using namespace lorelai::parser::statements;


const static std::unordered_map<string, node *(*)(lexer &lex)> lookupmap = {
	{ "return", [](lexer &lex) -> node * { return new returnstatement(lex); } },
	{ "while", [](lexer &lex) -> node * { return new whilestatement(lex); } },
	{ "do", [](lexer &lex) -> node * { return new dostatement(lex); } },
	{ "repeat", [](lexer &lex) -> node * { return new repeatstatement(lex); } },
	{ "local", [](lexer &lex) -> node * {
		lex.expect("local", "local deducer");
		auto ahead = lex.lookahead();
		if (ahead && ahead.value() == "function") {
			return new localfunctionstatement(lex);
		}

		return new localassignmentstatement(lex);
	} },
	{ "for", [](lexer &lex) -> node * {
		lex.expect("for", "for deducer");

		auto name = expressions::nameexpression(lex).name;
		if (!lex.lookahead()) {
			lex.wasexpected("<token>", "for deducer");
		}
		else if (lex.lookahead().value() == "=") {
			return new fornumstatement(name, lex);
		}
		return new forinstatement(name, lex);
	} },
	{ "break", [](lexer &lex) -> node * { return new breakstatement(lex); } },
	{ "if", [](lexer &lex) -> node * { return new ifstatement(lex); } },
	{ "function", [](lexer &lex) -> node * { return new functionstatement(lex); } }
	/* { "function", [](lexer &lex) -> node * {
		return std::make_shared<functionstatement>(lex);
	} } */
};

node *statement::read(lexer &lex) {
	/*
	stat ::=
!        varlist `=´ explist | 
!		 functioncall | 
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

	node *stmt = nullptr;

	auto lookup = lookupmap.find(word);
	if (lookup != lookupmap.end()) {
		stmt = lookup->second(lex);
	}

	if (!stmt) {
		// varlist = exprlist | functioncall
		auto exp = expression::read(lex);
		if (exp) {
			if (dynamic_cast<expressions::functioncallexpression *>(exp)) {
				stmt = new functioncallstatement(exp);
			}
			else {
				stmt = new assignmentstatement(exp, lex);
			}
		}
	}

	if (stmt && lex.lookahead().value_or("") == ";") {
		lex.read();
	}

	return stmt;
}

LORELAI_STATEMENT_BRANCH_CLASS_MACRO(LORELAI_ACCEPT_BRANCH)