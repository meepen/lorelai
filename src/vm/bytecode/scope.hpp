#ifndef BYTECODE_SCOPE_HPP_
#define BYTECODE_SCOPE_HPP_

#include "types.hpp"
#include "stack.hpp"
#include "visitor.hpp"
#include "parser/expressions.hpp"
#include "parser/statements.hpp"
#include "parser/node.hpp"
#include <memory>
#include <unordered_map>

namespace lorelai {
	namespace bytecode {
		class variable {
			friend class scope;
			friend class variablevisitor;
			friend struct std::hash<variable>;
		public:
			variable() { }

			variable(std::uint32_t _scopeid, const string &_name, std::uint32_t _version, std::uint32_t _argnum) :
				version(_version), scopeid(_scopeid), name(_name), argnum(_argnum)
			{

			}

			variable(std::uint32_t _scopeid, const string &_name, std::uint32_t _version) :
				version(_version), scopeid(_scopeid), name(_name)
			{

			}

			bool operator==(const variable &other) const {
				return other.scopeid == scopeid && version == other.version && name == other.name;
			}

		public:
			std::uint32_t scopeid, version;
			string name = "";

			std::uint32_t accesses = 0;
			std::uint32_t writes = 0;
			optional<std::uint32_t> argnum { };
		};
	}
}

template<>
struct std::hash<lorelai::bytecode::variable> {
	size_t operator()(const lorelai::bytecode::variable &v) const {
		return std::hash<string>()(v.name) ^
			std::hash<std::uint32_t>()(v.scopeid) ^
			std::hash<std::uint32_t>()(v.version);
	}
};

namespace lorelai {
	namespace bytecode {
		class scope {
			friend class variablevisitor;
			variable &createvariable(const variable &v) {
				shadowmap[v.name] = v.version;
				variables.push_back(v);
				return variables.back();
			}
		public:
			using variablecontainer = std::shared_ptr<variable>;
			scope(std::uint32_t _id) : id(_id) { }
			scope(std::uint32_t _id, size_t _parent) : id(_id), parent(_parent) { }

			variable &newvariable(const string &name, std::uint32_t argnum) {
				return createvariable(variable(id, name, versionof(name) + 1, argnum));
			}

			variable &newvariable(const string &name) {
				return createvariable(variable(id, name, versionof(name) + 1));
			}

			std::uint32_t versionof(string name) {
				if (auto v = find(name, false)) {
					return v->version;
				}

				return 0;
			}

			variable *find(string name, optional<int> version = { }) {
				auto it = variables.rbegin();
				while (it != variables.rend()) {
					if (it->name == name) {
						if (!version || version.value() == it->version) {
							return &*it;
						}
					}
					it++;
				}

				return nullptr;
			}

			variable *find(const variable &other) {
				for (auto &v : variables) {
					if (v == other) {
						return &v;
					}
				}

				return nullptr;
			}

		public:
			std::vector<variable> variables;
			std::unordered_map<string, std::uint32_t> shadowmap;
			std::uint32_t id;
			optional<size_t> parent { };
		};

#define LORELAI_SCOPE_VISIT_MACRO(n) LORELAI_VISIT_FUNCTION(n) { setnodescope(container); return false; }

		class scopevisitor : public parser::visitor {
		public:
			using parser::visitor::visit;
			using parser::visitor::postvisit;

			LORELAI_VISIT_FUNCTION(statements::fornumstatement) {
				newscope();
				return false;
			}
			LORELAI_POSTVISIT_FUNCTION(statements::fornumstatement) {
				freescope();
			}

			LORELAI_VISIT_FUNCTION(statements::forinstatement) {
				newscope();
				return false;
			}
			LORELAI_POSTVISIT_FUNCTION(statements::forinstatement) {
				freescope();
			}

			LORELAI_VISIT_FUNCTION(statements::whilestatement) {
				newscope();
				return false;
			}
			LORELAI_POSTVISIT_FUNCTION(statements::whilestatement) {
				freescope();
			}

			LORELAI_VISIT_FUNCTION(statements::dostatement) {
				newscope();
				return false;
			}
			LORELAI_POSTVISIT_FUNCTION(statements::dostatement) {
				freescope();
			}

			LORELAI_VISIT_FUNCTION(statements::functionstatement) {
				newscope(true);
				return false;
			}
			LORELAI_POSTVISIT_FUNCTION(statements::functionstatement) {
				freescope(true);
			}

			LORELAI_VISIT_FUNCTION(statements::localfunctionstatement) {
				newscope(true);
				return false;
			}
			LORELAI_POSTVISIT_FUNCTION(statements::localfunctionstatement) {
				freescope(true);
			}

			LORELAI_VISIT_FUNCTION(statements::ifstatement) {
				newscope();
				return false;
			}
			LORELAI_VISIT_FUNCTION(statements::elseifstatement) {
				freescope();
				newscope();
				return false;
			}
			LORELAI_VISIT_FUNCTION(statements::elsestatement) {
				freescope();
				newscope();
				return false;
			}
			LORELAI_POSTVISIT_FUNCTION(statements::ifstatement) {
				freescope();
			}

		public:
			virtual void onfreescope(bool isfunction) { }
			virtual void onnewscope(bool isfunction) { }

		private:
			void newscope(bool isfunction = false) {
				auto parent = getscope().id;
				activescopes.emplace_back(scopelist.size());
				scopelist.emplace_back(scopelist.size(), parent);
				onnewscope(isfunction);
			}

			void freescope(bool isfunction = false) {
				onfreescope(isfunction);
				activescopes.pop_back();
			}

		protected:
			scope &getscope() {
				return scopelist[activescopes.back()];
			}

			variable *findvariable(const string &name) {
				variable *v = nullptr;
				for (size_t i = activescopes.size(); i > 0; i--) {
					auto &scope = scopelist[activescopes[i - 1]];
					if ((v = scope.find(name))) {
						break;
					}
				}

				return v;
			}

			variable *findvariable(const parser::node *obj) {
				if (auto name = dynamic_cast<const parser::expressions::nameexpression *>(obj)) {
					return findvariable(name->name);
				}

				return nullptr;
			}

		public:
			std::vector<scope> scopelist { { 0 } };
		private:
			std::vector<size_t> activescopes { 0 };
		};

		class variablevisitor : public scopevisitor {
		protected:
			class constantconfirmer : public visitor {
			public:
				using visitor::visit;
				LORELAI_VISIT_FUNCTION(expressions::tableexpression) {
					found = true;
					return true;
				}
				LORELAI_VISIT_FUNCTION(expressions::functioncallexpression) {
					found = true;
					return true;
				}
				LORELAI_VISIT_FUNCTION(expressions::indexexpression) {
					found = true;
					return true;
				}
				LORELAI_VISIT_FUNCTION(expressions::nameexpression) {
					names.push_back(obj.name);
					return false;
				}
				LORELAI_VISIT_FUNCTION(expressions::dotexpression) {
					found = true;
					return true;
				}

			public:
				bool found = false;
				std::vector<string> names;
			};

		public:
			using scopevisitor::visit;
			using scopevisitor::postvisit;

			// postvisit since left side variables don't exist to right side expressions yet
			LORELAI_POSTVISIT_FUNCTION(statements::localassignmentstatement) {
				for (size_t i = 0; i < obj.left.size(); i++) {
					createvariable(obj.left[i]);
				}
			}

			LORELAI_VISIT_FUNCTION(statements::localfunctionstatement) {
				createvariable(obj.name);
				scopevisitor::visit(obj, container);

				return false;
			}

			LORELAI_VISIT_FUNCTION(funcbody) {
				scopevisitor::visit(obj, container);
				std::uint32_t paramnum = 1;

				for (auto &param : obj.params) {
					createvariable(param, paramnum++);
				}

				return false;
			}

			LORELAI_VISIT_FUNCTION(statements::assignmentstatement) {
				for (auto &child : obj.left) {
					if (auto referenced = findvariable(child)) {
						referenced->writes++;
					}
				}

				return false;
			}

			LORELAI_VISIT_FUNCTION(statements::forinstatement) {
				scopevisitor::visit(obj, container);
				for (auto &child : obj.iternames) {
					// need to hold this for the loop either way
					auto &v = createvariable(child);
					v.accesses++;
					v.writes++;
				}

				return false;
			}

			LORELAI_VISIT_FUNCTION(statements::fornumstatement) {
				scopevisitor::visit(obj, container);
				// need to hold this for the loop either way
				auto &v = createvariable(obj.itername);
				v.accesses++;
				v.writes++;

				return false;
			}

			LORELAI_VISIT_FUNCTION(expressions::indexexpression) {
				scopevisitor::visit(obj, container);
				
				if (auto referenced = findvariable(obj.prefix)) {
					referenced->writes++;
				}
				

				return false;
			}

			LORELAI_VISIT_FUNCTION(expressions::nameexpression) {
				auto var = findvariable(obj.name);
				if (var) {
					var->accesses++;
				}

				return false;
			}

		public:
			virtual void onnewvariable(const variable &) { }
			virtual void onnewvariables(const std::vector<variable> &) { }
			virtual void onfreevariable(const variable &) { }

			void onfreescope(bool isfunction) override {
				for (auto &child : getscope().variables) {
					onfreevariable(child);
				}
			}

		private:
			void variablecreated(const variable &v) {
				if (v.version > 1) { // variable is now shadowed
					for (auto &child : getscope().variables) {
						if (child.version == v.version - 1) {
							onfreevariable(child);
							break;
						}
					}
				}
				onnewvariable(v);
			}

			variable &createvariable(const string &name) {
				auto &v = getscope().newvariable(name);

				variablecreated(v);

				return v;
			}

			variable &createvariable(const string &name, std::uint32_t argnum) {
				auto &v = getscope().newvariable(name, argnum);

				variablecreated(v);

				return v;
			}

		public:
			std::unordered_map<variable, std::vector<variable>> variablereferences;
		};
	}
}

#endif // BYTECODE_SCOPE_HPP_