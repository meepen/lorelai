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
		class OLDscope {
		public:
			OLDscope *findvariablescope(string name, std::shared_ptr<OLDscope> highest = nullptr) {
				auto var = variables.find(name);
				if (var != variables.end()) {
					return this;
				}

				if (parent && parent != highest) {
					return parent->findvariablescope(name);
				}

				return nullptr;
			}

			std::uint32_t addvariable(string name, std::uint32_t stackpos) {
				variables[name] = stackpos;
				return stackpos;
			}

			std::uint32_t getvariableindex(string name) {
				auto var = variables.find(name);
				if (var != variables.end()) {
					return var->second;
				}

				return -1;
			}

			std::uint32_t hasvariable(string name) {
				return variables.find(name) != variables.end();
			}

		public:
			std::unordered_map<string, std::uint32_t> variables;
			std::shared_ptr<OLDscope> parent;
		};

		class variable {
			friend class scope;
			friend class variablevisitor;
			friend struct std::hash<variable>;
		public:
			variable() { }

			variable(std::uint32_t _scopeid, string _name, std::uint32_t _version) :
				version(_version), scopeid(_scopeid), name(_name)
			{

			}

			bool operator==(const variable &other) const {
				return other.scopeid == scopeid && version == other.version && name == other.name;
			}

		private:
			std::uint32_t scopeid, version;
		public:
			string name = "";

			std::uint32_t accesses = 0;
			std::uint32_t writes = 0;
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
		public:
			using variablecontainer = std::shared_ptr<variable>;
			scope(std::uint32_t _id, std::shared_ptr<scope> _parent = nullptr) : id(_id), parent(_parent) { }

			variablecontainer newvariable(string name) {
				variablecontainer v = std::make_shared<variable>(id, name, versionof(name) + 1);
				shadowmap[v->name] = v->version;
				variables.push_back(v);
				return v;
			}

			std::uint32_t versionof(string name) {
				if (auto v = find(name, false)) {
					return v->version;
				}

				return 0;
			}

			variablecontainer find(string name, bool recursive = true) {
				auto it = variables.rbegin();
				while (it != variables.rend()) {
					if ((*it)->name == name) {
						return *it;
					}
					it++;
				}

				if (recursive && parent) {
					return parent->find(name, true);
				}

				return nullptr;
			}

		public:
			std::vector<variablecontainer> variables;
			std::shared_ptr<scope> parent;
			std::unordered_map<string, std::uint32_t> shadowmap;
			std::uint32_t id;
		};

#define LORELAI_SCOPE_VISIT_MACRO(n) LORELAI_VISIT_FUNCTION(n) { setnodescope(container); return false; }

		class scopevisitorbase : public parser::visitor {
		public:
			scopevisitorbase() {
				scopes.push_back(curscope);
			}
			LORELAI_VISIT_NAME_MACRO(LORELAI_SCOPE_VISIT_MACRO)

		protected:
			void setnodescope(parser::node *node) {
				scopemap[node] = curscope;
			}

		protected:
			std::shared_ptr<scope> curscope = std::make_shared<scope>(0);

		public:
			std::unordered_map<parser::node *, std::shared_ptr<scope>> scopemap;
			std::vector<std::shared_ptr<scope>> scopes;
		};

		class scopevisitor : public scopevisitorbase {
		public:
			using scopevisitorbase::visit;
			using scopevisitorbase::postvisit;

			LORELAI_VISIT_FUNCTION(statements::fornumstatement) {
				setnodescope(container);
				newscope();
				return false;
			}
			LORELAI_POSTVISIT_FUNCTION(statements::fornumstatement) {
				freescope();
			}

			LORELAI_VISIT_FUNCTION(statements::forinstatement) {
				setnodescope(container);
				newscope();
				return false;
			}
			LORELAI_POSTVISIT_FUNCTION(statements::forinstatement) {
				freescope();
			}

			LORELAI_VISIT_FUNCTION(statements::whilestatement) {
				setnodescope(container);
				newscope();
				return false;
			}
			LORELAI_POSTVISIT_FUNCTION(statements::whilestatement) {
				freescope();
			}

			LORELAI_VISIT_FUNCTION(statements::dostatement) {
				setnodescope(container);
				newscope();
				return false;
			}
			LORELAI_POSTVISIT_FUNCTION(statements::dostatement) {
				freescope();
			}

			LORELAI_VISIT_FUNCTION(statements::functionstatement) {
				setnodescope(container);
				newscope();
				return false;
			}
			LORELAI_POSTVISIT_FUNCTION(statements::functionstatement) {
				freescope();
			}

			LORELAI_VISIT_FUNCTION(statements::localfunctionstatement) {
				newscope();
				setnodescope(container);
				return false;
			}
			LORELAI_POSTVISIT_FUNCTION(statements::localfunctionstatement) {
				freescope();
			}

			LORELAI_VISIT_FUNCTION(statements::ifstatement) {
				setnodescope(container);
				newscope();
				return false;
			}
			LORELAI_VISIT_FUNCTION(statements::elseifstatement) {
				freescope();
				setnodescope(container);
				newscope();
				return false;
			}
			LORELAI_VISIT_FUNCTION(statements::elsestatement) {
				freescope();
				setnodescope(container);
				newscope();
				return false;
			}
			LORELAI_POSTVISIT_FUNCTION(statements::ifstatement) {
				freescope();
			}

		public:
			virtual void onfreescope() { }
			virtual void onnewscope() { }

		private:
			std::shared_ptr<scope> &newscope() {
				curscope = std::make_shared<scope>(scopes.size(), curscope);
				scopes.push_back(curscope);
				onnewscope();
				return curscope;
			}

			std::shared_ptr<scope> &freescope() {
				if (!curscope) {
					throw;
				}

				onfreescope();
				curscope = curscope->parent;
				return curscope;
			}
		};

		class variablevisitor : public scopevisitor {
		public:
			using scopevisitor::visit;
			using scopevisitor::postvisit;

			LORELAI_VISIT_FUNCTION(statements::localassignmentstatement) {
				scopevisitor::visit(obj, container);
				return false;
			}

			// postvisit since left side variables don't exist to right side expressions yet
			LORELAI_POSTVISIT_FUNCTION(statements::localassignmentstatement) {
				createvariable(obj.left);
			}

			LORELAI_VISIT_FUNCTION(statements::assignmentstatement) {
				for (auto &child : obj.left) {
					findandaddwrite(child);
				}

				return false;
			}

			LORELAI_VISIT_FUNCTION(statements::forinstatement) {
				scopevisitor::visit(obj, container);
				for (auto &child : obj.iternames) {
					// need to hold this for the loop either way
					auto v = curscope->newvariable(child);
					v->accesses++;
					v->writes++;
				}

				return false;
			}

			LORELAI_VISIT_FUNCTION(statements::fornumstatement) {
				scopevisitor::visit(obj, container);
				// need to hold this for the loop either way
				auto v = curscope->newvariable(obj.itername);
				v->accesses++;
				v->writes++;

				return false;
			}

			LORELAI_VISIT_FUNCTION(expressions::indexexpression) {
				scopevisitor::visit(obj, container);

				findandaddwrite(obj.prefix);

				return false;
			}

			LORELAI_VISIT_FUNCTION(expressions::nameexpression) {
				auto var = curscope->find(obj.name, true);
				if (var) {
					var->accesses++;
				}

				return false;
			}

		public:
			virtual void onnewvariable(scope::variablecontainer) { }
			virtual void onnewvariables(const std::vector<scope::variablecontainer> &) { }
			virtual void onfreevariable(scope::variablecontainer) { }

			void onfreescope() override {
				for (auto &child : curscope->variables) {
					onfreevariable(child);
				}
			}

		private:
			void createvariable(const std::vector<string> &names) {
				std::vector<scope::variablecontainer> newvars;
				for (auto &name : names) {
					auto v = curscope->newvariable(name);
					newvars.push_back(v);

					if (v->version > 1) { // variable is now shadowed
						for (auto child : curscope->variables) {
							if (child->version == v->version - 1) {
								onfreevariable(child);
								break;
							}
						}
					}
				}

				onnewvariables(newvars);
			}
			void createvariable(string name) {
				auto v = curscope->newvariable(name);

				if (v->version > 1) { // variable is now shadowed
					for (auto child : curscope->variables) {
						if (child->version == v->version - 1) {
							onfreevariable(child);
							break;
						}
					}
				}
				
				onnewvariable(v);
			}

			void findandaddwrite(parser::node *obj) {
				if (auto name = dynamic_cast<parser::expressions::nameexpression *>(obj)) {
					auto var = curscope->find(name->name);
					if (var) {
						var->writes++;
					}
				}
			}
		};

		struct _scopemap {
			decltype(variablevisitor::scopemap) nodemap;
			std::vector<std::shared_ptr<scope>> scopes;
		};

		static _scopemap generatescopemap(parser::node *container) {
			variablevisitor visitor;
			container->accept(visitor, container);
			return { visitor.scopemap, visitor.scopes };
		}
	}
}

#endif // BYTECODE_SCOPE_HPP_