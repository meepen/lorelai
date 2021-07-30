#ifndef BCCONSTANTS_HPP_
#define BCCONSTANTS_HPP_

namespace lorelai {
    namespace parser {
        class node;
    }

    namespace bytecode {
        class bytecodegenerator;
        const parser::node *collapseconstant(bytecodegenerator &gen, const parser::node &expr);
    }
}

#endif // BCCONSTANTS_HPP_