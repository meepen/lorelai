#ifndef BCCONSTANTS_HPP_
#define BCCONSTANTS_HPP_

namespace lorelai {
    namespace parser {
        class node;
    }

    namespace bytecode {
        class bytecodegenerator;
        parser::node *collapseconstant(bytecodegenerator &gen, parser::node &expr);
    }
}

#endif // BCCONSTANTS_HPP_