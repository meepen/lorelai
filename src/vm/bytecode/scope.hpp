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

			variable(std::uint32_t _scopeid, string _name, std::uint32_t _version) :
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

			variable &newvariable(string name) {
				variable v(id, name, versionof(name) + 1);
				shadowmap[v.name] = v.version;
				variables.push_back(v);
				return variables.back();
			}

			std::uint32_t versionof(string name) {
				if (auto v = find(name, false)) {
					return v->version;
				}

				return 0;
			}

			variable *find(string name, bool recursive = true, optional<int> version = { }) {
				auto it = variables.rbegin();
				while (it != variables.rend()) {
					if (it->name == name) {
						if (!version || version.value() == it->version) {
							return &*it;
						}
					}
					it++;
				}

				if (recursive && parent) {
					return parent->find(name, true);
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

		public:
			std::shared_ptr<scope> curscope = std::make_shared<scope>(0);
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
					found = true;
					return true;
				}
				LORELAI_VISIT_FUNCTION(expressions::dotexpression) {
					found = true;
					return true;
				}

			public:
				bool found = false;
			};

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

				for (size_t i = 0; i < std::min(obj.left.size(), obj.right.size()); i++) {
					constantconfirmer confirmer;
					obj.right[i]->accept(confirmer, obj.right[i]);
					if (confirmer.found) {
						findandaddwrite(obj.left[i]);
					}
				}
			}

			LORELAI_VISIT_FUNCTION(statements::localfunctionstatement) {
				createvariable(obj.name).writes++;
				scopevisitor::visit(obj, container);

				return false;
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
			virtual void onnewvariable(const variable &) { }
			virtual void onnewvariables(const std::vector<variable> &) { }
			virtual void onfreevariable(const variable &) { }

			void onfreescope() override {
				for (auto &child : curscope->variables) {
					onfreevariable(child);
				}
			}

		private:
			void createvariable(const std::vector<string> &names) {
				std::vector<variable> newvars;
				for (auto &name : names) {
					auto &v = curscope->newvariable(name);

					if (v.version > 1) { // variable is now shadowed
						for (auto &child : curscope->variables) {
							if (child.version == v.version - 1) {
								onfreevariable(child);
								break;
							}
						}
					}

					newvars.push_back(v);
				}

				onnewvariables(newvars);
			}

			variable &createvariable(string name) {
				auto &v = curscope->newvariable(name);

				if (v.version > 1) { // variable is now shadowed
					for (auto &child : curscope->variables) {
						if (child.version == v.version - 1) {
							onfreevariable(child);
							break;
						}
					}
				}
				
				onnewvariable(v);

				return v;
			}

			void findandaddwrite(parser::node *obj) {
				if (auto name = dynamic_cast<parser::expressions::nameexpression *>(obj)) {
					auto var = curscope->find(name->name);
					if (var) {
						var->writes++;
					}
				}
			}
			void findandaddwrite(string &name) {
				auto var = curscope->find(name);
				if (var) {
					var->writes++;
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